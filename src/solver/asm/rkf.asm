default rel

extern printf, pow
global solver_rkf_asm, solver_rkf_initialize, solver_rkf_ready, solver_rfk_update_restrictions
global print_rkf_Ytemp				; Used for debug purposes.

section .rodata
	%include "globals.asm"

	two dq 2.0
	quarter dq 0.25
	eighth dq 0.125
	three_eights dq 0.375
	half dq 0.5
	zero dq 0.0
	minusone dq -1.0
	
	Ytemp_str db "    K%d:  Ytemp[0] = %4.8f, Ytemp[1] = %4.8f, Ytemp[2] = %4.8f, Ytemp[3] = %4.8f",LF,0		; Used for debug purposes.
	
	false equ 0
	true equ 1
	
section .data
	functor dq nullptr				; Pointer to the functor provided to ode_solver::rkfsolver<N> object.
	f dq nullptr					; Will hold the address of the functor callback member function, which will require [functor] as its first parameter.
	this dq nullptr					; Placeholder for the ode_solver::rkfsolver<N> this pointer.
	Ytemp dq nullptr				; Internal arrays of doubles, each of length N.
	K1_p dq nullptr
	K2_p dq nullptr
	K3_p dq nullptr
	K4_p dq nullptr
	K5_p dq nullptr
	K6_p dq nullptr
	R_p dq nullptr
	Y dq nullptr					; External ode_solver::state<N> == double[N] state pointer, provided by the caller of solver_rkf_asm().
	
	δt dq 0.0						; This is the time step δt.
	t dq 0.0						; This is the current time.
	N dd 4							; N is the dimension of the external state vector X.
	Initialized db false			; Returend by solver_ready(), and set by solver_initialize(), which must be called, less we want segfaulting.
	
	; Coefficients needed for the 5th order local truncation error estimate.
	K2_1 dq 0
	K3_1 dq 0
	K3_2 dq 0
	K4_1 dq 0
	K4_2 dq 0
	K4_3 dq 0
	K5_1 dq 0
	K5_2 dq 0
	K5_3 dq 0
	K5_4 dq 0
	K6_1 dq 0
	K6_2 dq 0
	K6_3 dq 0
	K6_4 dq 0
	K6_5 dq 0
	
	; Coefficients needed for R, the local truncation error.
	R_1 dq 0
	R_2 dq 0
	R_3 dq 0
	R_4 dq 0
	R_5 dq 0
	
	; Coefficients for the Runge-Kutta-Fehlberg RK4 variant.
	RK4_1 dq 0
	RK4_2 dq 0
	RK4_3 dq 0
	RK4_4 dq 0
	
	; Coefficient needed to calculate the dt scaling factor.
 	q_coeff dq 0
 	
 	twelve_thirteenths dq 0
 	
 	δt_min dq 0.1
 	δt_max dq 1.0
 	tolerance dq 1e-5
 	
 	acceptable db 1
	
section .text
solver_rkf_asm:
	; void rkf_asm(parent* this, double* Y, double t, double& dt) = void solver::rkf_asm(parent* this, ode_solver::state<N> Y, double t, double& dt)
	; rdi = implicit this pointer to parent.  Here the parent is either ode_solver::rkfsolver<N> if it derives from class assembly_functions, or class
	;		assembly_functions, if that class is a member of class ode_solver::rkfsolver<N>.  Either way, the parent pointer is not used.
	; rsi = pointer to Y.
	; xmm0 = the time/independent variable t.  This is not changed by this routine.
	; rdx = reference to the time step δt.  The time step is changed and thus we must ensure that the change propagates to the client.
	
	; The RKF algorithm has two parts.
	; Part I:
	; Obtain the 5th order local truncation error R[i], and update the time step δt.  If a R[i] happens to exceed the tolerance, then we loop until we
	; find a different δt that produces local truncation errors that are within tolerance, or until the scaling of the time step caused δt to fall outside
	; the permitted [δt_min, δt_max] range, in which case we set δt to the lower upper bound of the range.
	; Part II:
	; With a satisfactory δt, we proceed to calculate the new state using the Runge-Kutta-Fehlberg RK4 variant.
	
	; prologue
	push rbp
	mov rbp, rsp
	push rbx
	push rdx						; As δt is provided as a reference, we need to save the address in order to write the change back to the client.
									; Note that an even number of push operations since 'push rbp' ensure that rsp % 16 == 0.
	
	movsd [t], xmm0					; Save the current time.  It is not changed by this function.
	movsd xmm1, [rdx]				; The address of δt.  We push it along with rbx and need to write the new value back to it after we're done here.
	movsd [δt], xmm1				; Assign the time step variable, as well as its backup copy as this will be modified.
	mov [acceptable], byte true
	mov [Y], rsi					; Save the location of the state vector Y.
	
.PartI:
.LocalTruncationError_DoWhileLoop:
	
	mov [acceptable], byte true
	
	; 	(*_f)(Y, _K1, t);			// Calculate K1/δt.
	mov rdi, [functor]				; The functor "this" pointer is the first parameter to the class's member function.
	mov rsi, [Y]					; Y is the second parameter.
	mov rdx, [K1_p]					; K1 is the third parameter.
	
	movsd xmm0, [t]					; fourth param = t.
	
	call [f]						; Calling (*_f)(Y, K1, t) to obtain K1/δt;

	;	{ _K1[i] *= δt; _Ytemp[i] = Y[i] + K2_1*_K1[i]; }
	mov rdi, [K1_p]					; Set up our pointers.
	mov rsi, [Ytemp]
	mov rbx, [Y]
	
	xor rax, rax					; Initialize loop counter.
.for_K1:
	
	; 	K1[i] *= δt;
	movsd xmm0, [rdi]				; Retrieve K1[i],
	mulsd xmm0, [δt]				; multiply it by δt,
	movsd [rdi], xmm0				; and store the result at K1[i].
	; 	Ytemp[i] = Y[i] + K2_1*K1[i];
	mulsd xmm0, [K2_1]				; Multiply K1[i] by K2_1,
	addsd xmm0,	[rbx]				; add Y[i] to it:  xmm0 = Y[i] + K2_1*K1[i],
	movsd [rsi], xmm0				; and store it at Ytemp[i].
	
	inc eax
	cmp eax, dword [N]
	je .K2_prep

	; Increment the pointers to next double within the K1_p, Ytemp and Y arrays.
	lea rdi, [rdi + 8]
	lea rsi, [rsi + 8]
	lea rbx, [rbx + 8]
	
	jmp .for_K1
	
.K2_prep:

	; 	(*_f)(_Ytemp, _K2, t + 0.25*δt);	// calculate K2/δt.
	mov rdi, [functor]				; The functor "this" pointer is the first parameter to the class's member function.
	mov rsi, [Ytemp]				; Ytemp is the second parameter.
	mov rdx, [K2_p]					; K2_p is the third parameter.
	movsd xmm0, [δt]
	mulsd xmm0, [quarter]
	addsd xmm0, [t]					; param4 = t + 0.25*δt.
	
	call [f]						; Calling (*_f)(Ytemp, K2, t + 0.25*δt) to obtain K2/δt;
	
	; 	{_K2[i] *= dt; _Ytemp[i] = Y[i] + K3_1*_K1[i] + K3_2*_K2[i]; }					
	mov rdi, [K2_p]					; Set up our pointers.
	mov rsi, [Ytemp]
	mov rbx, [Y]
	mov rcx, [K1_p]
	xor rax, rax					; Initialize loop counter.
	
.for_K2:
	; 	K2[i] *= δt;
	movsd xmm0, [rdi]				; Retrieve K2[i],
	mulsd xmm0, [δt]				; multiply it by δt,
	movsd [rdi], xmm0				; and store the result at K2[i].
	;	_Ytemp[i] = Y[i] + K3_1*_K1[i] + K3_2*_K2[i];
	mulsd xmm0, [K3_2]				; Multiply K2[i] by K3_2.
	
	movsd xmm1, [rcx]				; Retrieve K1[i],
	mulsd xmm1, [K3_1]				; and multiply it by K3_1,
	addsd xmm0, xmm1				; and add it to K3_2*K2[i].
	
	movsd xmm1, [rbx]				; Retrieve Y[i],
	addsd xmm0, xmm1				; and add it to K3_1*K1[i] + K3_2*K2[i].
	movsd [rsi], xmm0				; Store the result in Ytemp[i].
	
	inc eax
	cmp eax, dword [N]
	je .K3_prep
	
	; Increment the pointers to next double within the K2_p, Ytemp, Y and K1_p arrays.
	lea rdi, [rdi + 8]
	lea rsi, [rsi + 8]
	lea rbx, [rbx + 8]
	lea rcx, [rcx + 8]
	
	jmp .for_K2

.K3_prep:
	
	;	(*_f)(_Ytemp, _K3, t + 3.0/8.0*dt);		// calculate K3/dt.
	mov rdi, [functor]				; The functor "this" pointer is the first parameter to the class's member function.
	mov rsi, [Ytemp]				; Ytemp is the second parameter.
	mov rdx, [K3_p]					; K3_p is the third parameter.
	movsd xmm0, [δt]
	mulsd xmm0, [three_eights]
	addsd xmm0, [t]					; param4 = t + 3/8*δt.
	
	call [f]						; Calling (*_f)(Ytemp, K3, t + 3/8*δt) to obtain K3/δt;
	
	; 	{ _K3[i] *= dt; _Ytemp[i] = Y[i] + K4_1*_K1[i] + K4_2*_K2[i] + K4_3*_K3[i]; }
	mov rdi, [K3_p]					; Set up our pointers.
	mov rsi, [Ytemp]
	mov rbx, [Y]
	mov rcx, [K1_p]
	mov rdx, [K2_p]
	xor rax, rax					; Initialize loop counter.
	
.for_K3:
	
	;	_K3[i] *= dt;
	movsd xmm0, [rdi]				; Retrieve K3[i],
	mulsd xmm0, [δt]				; multiply it by δt,
	movsd [rdi], xmm0				; and store the result at K3[i].
	;	_Ytemp[i] = Y[i] + K4_1*_K1[i] + K4_2*_K2[i] + K4_3*_K3[i];
	mulsd xmm0, [K4_3]				; Multiply K3[i] by K4_3.
	
	movsd xmm1, [rdx]				; Retrieve K2[i],
	mulsd xmm1, [K4_2]				; and multiply it by K4_2.
	addsd xmm0, xmm1				; Add K4_2*K2[i] to K4_3*K3[i].
	
	movsd xmm1, [rcx]				; Retrieve K1[i],
	mulsd xmm1, [K4_1]				; and multiply it by K4_1.
	addsd xmm0, xmm1				; Add K4_1*K1[i] to K4_2*K2[i] + K4_3*K3[i].
	
	movsd xmm1, [rbx]				; Retrieve Y[i],
	addsd xmm0, xmm1				; and add it to K4_1*K1[i] + K4_2*K2[i] + K4_3*K3[i].
	movsd [rsi], xmm0				; Store the result in Ytemp[i].
	
	inc eax
	cmp eax, dword [N]
	je .K4_prep
	
	; Increment the pointers to next double within the K3_p, Ytemp, Y, K1_p and K2_p arrays.
	lea rdi, [rdi + 8]
	lea rsi, [rsi + 8]
	lea rbx, [rbx + 8]
	lea rcx, [rcx + 8]
	lea rdx, [rdx + 8]
	
	jmp .for_K3
	
.K4_prep:

	; 	(*_f)(_Ytemp, _K4, t + twelve_thirteenths*dt);			// calculate K4/dt.
	mov rdi, [functor]				; The functor "this" pointer is the first parameter to the class's member function.
	mov rsi, [Ytemp]				; Ytemp is the second parameter.
	mov rdx, [K4_p]					; K4_p is the third parameter.
	movsd xmm0, [δt]
	mulsd xmm0, [twelve_thirteenths]
	addsd xmm0, [t]					; param4 = t + 12/13*δt.
	
	call [f]						; Calling (*_f)(Ytemp, K4, t + 12/13*δt) to obtain K4/δt;

	; 	{ _K4[i] *= dt; _Ytemp[i] = Y[i] + K5_1*_K1[i] + K5_2*_K2[i] + K5_3*_K3[i] + K5_4*_K4[i]; }
	mov rdi, [K4_p]					; Set up our pointers.
	mov rsi, [Ytemp]
	mov rbx, [Y]
	mov rcx, [K1_p]
	mov rdx, [K2_p]
	mov r8, [K3_p]
	xor rax, rax					; Initialize loop counter.
	
.for_K4:
	;	_K4[i] *= dt;
	movsd xmm0, [rdi]				; Retrieve K4[i],
	mulsd xmm0, [δt]				; multiply it by δt,
	movsd [rdi], xmm0				; and store the result at K4[i].
	;	_Ytemp[i] = Y[i] + K5_1*_K1[i] + K5_2*_K2[i] + K5_3*_K3[i] + K5_4*_K4[i];
	mulsd xmm0, [K5_4]				; Multiply K4[i] by K5_4.
	
	movsd xmm1, [r8]				; Retrieve K3[i],
	mulsd xmm1, [K5_3]				; and multiply it by K5_3.
	addsd xmm0, xmm1				; Add K5_3*K3[i] to K5_4*K4[i].
	
	movsd xmm1, [rdx]				; Retrieve K2[i],
	mulsd xmm1, [K5_2]				; and multiply it by K5_2.
	addsd xmm0, xmm1				; Add K5_2*K2[i] to K5_3*K3[i] + K5_4*K4[i].
	
	movsd xmm1, [rcx]				; Retrieve K1[i],
	mulsd xmm1, [K5_1]				; and multiply it by K5_1.
	addsd xmm0, xmm1				; Add K5_1*K1[i] to K5_2*K2[i] + K5_3*K3[i] + K5_4*K4[i].
	
	movsd xmm1, [rbx]				; Retrieve Y[i],
	addsd xmm0, xmm1				; and add it to K5_1*K1[i] + K5_2*K2[i] + K5_3*K3[i] + K5_4*K4[i].
	movsd [rsi], xmm0				; Store the result in Ytemp[i].

	inc eax
	cmp eax, dword [N]
	je .K5_prep
	
	; Increment the pointers to next double within the K4_p, Ytemp, Y, K1_p, K2_p and K3_p arrays.
	lea rdi, [rdi + 8]
	lea rsi, [rsi + 8]
	lea rbx, [rbx + 8]
	lea rcx, [rcx + 8]
	lea rdx, [rdx + 8]
	lea r8, [r8 + 8]
	
	jmp .for_K4
	
.K5_prep:
	
	; 	(*_f)(_Ytemp, _K5, t + dt);								// calculate K5/dt.
	mov rdi, [functor]				; The functor "this" pointer is the first parameter to the class's member function.
	mov rsi, [Ytemp]				; Ytemp is the second parameter.
	mov rdx, [K5_p]					; K5_p is the third parameter.
	movsd xmm0, [δt]
	addsd xmm0, [t]					; param4 = t + δt.
	
	call [f]						; Calling (*_f)(Ytemp, K5, t + δt) to obtain K5/δt;
	
	; 	{ _K5[i] *= dt; _Ytemp[i] = Y[i] + K6_1*_K1[i] + K6_2*_K2[i] + K6_3*_K3[i] + K6_4*_K4[i] + K6_5*_K5[i]; }
	mov rdi, [K5_p]					; Set up our pointers.
	mov rsi, [Ytemp]
	mov rbx, [Y]
	mov rcx, [K1_p]
	mov rdx, [K2_p]
	mov r8, [K3_p]
	mov r9, [K4_p]
	xor rax, rax					; Initialize loop counter.
	
.for_K5:
	; 	_K5[i] *= dt;
	movsd xmm0, [rdi]				; Retrieve K5[i],
	mulsd xmm0, [δt]				; multiply it by δt,
	movsd [rdi], xmm0				; and store the result at K5[i].
	; 	_Ytemp[i] = Y[i] + K6_1*_K1[i] + K6_2*_K2[i] + K6_3*_K3[i] + K6_4*_K4[i] + K6_5*_K5[i];
	mulsd xmm0, [K6_5]				; Multiply K4[i] by K6_5.
	
	movsd xmm1, [r9]				; Retrieve K4[i],
	mulsd xmm1, [K6_4]				; and multiply it by K6_4.
	addsd xmm0, xmm1				; Add K6_4*K4[i] to K6_5*K5[i].
	
	movsd xmm1, [r8]				; Retrieve K3[i],
	mulsd xmm1, [K6_3]				; and multiply it by K6_3.
	addsd xmm0, xmm1				; Add K6_3*K3[i] to K6_4*K4[i] + K6_5*K5[i].
	
	movsd xmm1, [rdx]				; Retrieve K2[i],
	mulsd xmm1, [K6_2]				; and multiply it by K6_2.
	addsd xmm0, xmm1				; Add K6_2*K2[i] to K6_3*K3[i] + K6_4*K4[i] + K6_5*K5[i].
	
	movsd xmm1, [rcx]				; Retrieve K1[i],
	mulsd xmm1, [K6_1]				; and multiply it by K6_1.
	addsd xmm0, xmm1				; Add K6_1*K1[i] to K6_2*K2[i] + K6_3*K3[i] + K6_4*K4[i] + K6_5*K5[i].
	
	movsd xmm1, [rbx]				; Retrieve Y[i],
	addsd xmm0, xmm1				; and add it to K6_1*K1[i] + K6_2*K2[i] + K6_3*K3[i] + K6_4*K4[i] + K6_5*K5[i].
	movsd [rsi], xmm0				; Store the result in Ytemp[i].
	
	inc eax
	cmp eax, dword [N]
	je .K6_prep
	
	; Increment the pointers to next double within the K5_p, Ytemp, Y, K1_p, K2_p, K3_p and K4_p arrays.
	lea rdi, [rdi + 8]
	lea rsi, [rsi + 8]
	lea rbx, [rbx + 8]
	lea rcx, [rcx + 8]
	lea rdx, [rdx + 8]
	lea r8, [r8 + 8]
	lea r9, [r9 + 8]
	
	jmp .for_K5
	
.K6_prep:
	
	; 	(*_f)(_Ytemp, _K6, t + 0.5*dt);							// calculate K6/dt.
	mov rdi, [functor]				; The functor "this" pointer is the first parameter to the class's member function.
	mov rsi, [Ytemp]				; Ytemp is the second parameter.
	mov rdx, [K6_p]					; K6_p is the third parameter.
	movsd xmm0, [δt]
	mulsd xmm0, [half]
	addsd xmm0, [t]					; param4 = t + 0.5*δt.
	
	call [f]						; Calling (*_f)(Ytemp, K6, t + 0.5*δt) to obtain K6/δt;
	
	; Note that K6 = K6/δt, so we need to multiply it by δt later.
	
.R_prep:
	
	;	_R[i] = abs(R_1*_K1[i] + R_2*_K3[i] + R_3*_K4[i] + R_4*_K5[i] + R_5*_K6[i])/δt;
	movsd xmm3, [zero]				; This will hold the cummulative sum RR = R[i]*R[i], where R[i] is the local trunctation error in the 5th order RKF.
	mov rdi, [K1_p]					; Set up our pointers.
	mov rsi, [K3_p]
	mov rbx, [K4_p]
	mov rcx, [K5_p]
	mov rdx, [K6_p]
	mov r8, [R_p]
	xor rax, rax					; Initialize loop counter.
	
.for_R:
	; 	_K6[i] *= dt;				; K6[i] up to now was acutally K6[i]/δt.
	movsd xmm4, [rdx]				; Retrieve K6[i],
	mulsd xmm4, [δt]				; multiply it by δt,
	movsd [rdx], xmm4				; and store the result at K6[i].
	
	; 	_R[i] = abs(R_1*_K1[i] + R_2*_K3[i] + R_3*_K4[i] + R_4*_K5[i] + R_5*_K6[i])/δt;
	mulsd xmm4, [R_5]				; Mutilipy K6[i] by R_5.
	
	movsd xmm1, [rcx]				; Retrieve K5[i],
	mulsd xmm1, [R_4]				; and multiply it by R_4.
	addsd xmm4, xmm1				; Add R_4*K5[i] to R_5*K6[i].
	
	movsd xmm1, [rbx]				; Retrieve K4[i],
	mulsd xmm1, [R_3]				; and multiply it by R_3.
	addsd xmm4, xmm1				; Add R_3*K4[i] to R_4*K5[i] + R_5*K6[i].
	
	movsd xmm1, [rsi]				; Retrieve K3[i],
	mulsd xmm1, [R_2]				; and multiply it by R_2.
	addsd xmm4, xmm1				; Add R_2*K3[i] to R_3*K4[i] + R_4*K5[i] + R_5*K6[i].
	
	movsd xmm1, [rdi]				; Retrieve K1[i],
	mulsd xmm1, [R_1]				; and multiply it by R_1.
	addsd xmm4, xmm1				; Add R_1*K1[i] to R_2*K3[i] + R_3*K4[i] + R_4*K5[i] + R_5*K6[i].
	
	ucomisd xmm4, [zero]			; Absolute value test.  If xmm4 >= 0.0 leave it alone, otherwise multiply by -1.
	jae .abs_done
	mulsd xmm4, [minusone]
	
.abs_done:
	
	divsd xmm4, [δt]				; Divide abs(R_1*K1[i] to R_2*K3[i] + R_3*K4[i] + R_4*K5[i] + R_5*δt*K6[i]) by δt.
	movsd [r8], xmm4				; Store R[i] in R_p.

	ucomisd xmm4, [tolerance]		; Test if R[i] > tolerance.  Go to Acceptability_Done if R[i] <= tolerance, else set flag.
	jbe .Acceptability_Done
	mov [acceptable], byte false

.Acceptability_Done:
	
	mulsd xmm4, xmm4				; Continuing, square the R[i].
	addsd xmm3, xmm4				; Add it to xmm3, which holds the cummulative sum of the R^2[i] values.	
	
	inc eax
	cmp eax, dword [N]
	je .R_loop_done
	
	; Increment the pointers to next double within the K5_p, Ytemp, Y, K1_p, K2_p, K3_p and K4_p arrays.
	lea rdi, [rdi + 8]
	lea rsi, [rsi + 8]
	lea rbx, [rbx + 8]
	lea rcx, [rcx + 8]
	lea rdx, [rdx + 8]
	lea r8, [r8 + 8]
	
	jmp .for_R
	
.R_loop_done:
	
	; Calculate the scaling factor q
	movsd xmm0, [tolerance]			; Retrieve tolerance.
	mulsd xmm0, xmm0				; Tolerance squared.
	divsd xmm0, xmm3				; param1 = (tolerance/R[i])^2
	movsd xmm1, [eighth]			; param2 = 1/8
	
	; TODO -- Figure out how to do the pow() function, without a call.  If we can't, then check error state in rax!
	call pow wrt ..plt				; xmm0 = pow(tolerance^2/R[i]^2)^0.125, with tolerance^2/R^2[i] = xmm0, and 0.125 in xmm1.
	
	mulsd xmm0, [q_coeff]			; Multiply pow(tolerance^2/R[i]^2)^0.125 by q_coeff to obtain the scaling factor q.
	movsd xmm1, [δt]				; Retrieve the time step, δt.
	mulsd xmm0, xmm1				; δt = q*δt.
	movsd [δt], xmm0

	; Test whether the time step is within the allowed [δt_min, δt_max] range.
	ucomisd xmm0, [δt_min]			; Is δt < δt_min?  If so, set δt to δt_min, and set flag to stop looping.
	ja .UpperLimitTest				; δt is above the lower limit, so test it against the upper limit.
	movsd xmm1, [δt_min]			; Set δt to δt_min and procced to PartII.
	movsd [δt], xmm1
	jmp .PartII_UpdateTheStateVector

.UpperLimitTest:
	ucomisd xmm0, [δt_max]			; Is δt > δt_max?  If so, set δt to δt_max, and set flag to stop looping.
	jb .AcceptabilityTest			; δt is within the allowed range.  Proceed to acceptability test.
	movsd xmm1, [δt_max]			; Set δt to δt_max and proceed to PartII.
	movsd [δt], xmm1
	jmp .PartII_UpdateTheStateVector

.AcceptabilityTest:					; This is necessary as despite δt being within acceptable range at this point, we may have had one of the R[i] > tolerance.
	xor rax, rax					; Iterate the main Loop until we obtain an acceptable error estimate R[i] for all i.
	cmp al, [acceptable]			; Test acceptable == false.  We set acceptable to true at the beginning of the main loop.
	je .LocalTruncationError_DoWhileLoop

.PartII_UpdateTheStateVector:		; Update the state vector.
	
	;	Y[i] = Y[i] + RK4_1*_K1[i] + RK4_2*_K3[i] + RK4_3*_K4[i] + RK4_4*_K5[i];
	mov rdi, [K1_p]					; Set up our pointers.
	mov rsi, [K3_p]
	mov rbx, [K4_p]
	mov rcx, [K5_p]
	mov rdx, [Y]
	xor rax, rax					; Initialize loop counter.
	
.rk4_loop:
	movsd xmm0, [rcx]				; Retrieve K5[i],
	mulsd xmm0, [RK4_4]				; and multiply it by RK4_4.
	
	movsd xmm1, [rbx]				; Retrieve K4[i],
	mulsd xmm1, [RK4_3]				; and multiply it by RK4_3.
	addsd xmm0, xmm1				; Add RK4_3*K4[i] to RK4_4*K5[i].
	
	movsd xmm1, [rsi]				; Retrieve K3[i],
	mulsd xmm1, [RK4_2]				; and multiply it by RK4_2.
	addsd xmm0, xmm1				; Add RK4_2*K3[i] to RK4_3*K4[i] + RK4_4*K5[i].
	
	movsd xmm1, [rdi]				; Retrieve K1[i],
	mulsd xmm1, [RK4_1]				; and multiply it by RK4_1.
	addsd xmm0, xmm1				; Add RK4_1*K1[i] to RK4_2*K3[i] + RK4_3*K4[i] + RK4_4*K5[i].
	
	movsd xmm1, [rdx]				; Retrieve Y[i]
	addsd xmm0, xmm1				; and add it to RK4_1*K1[i] + RK4_2*K3[i] + RK4_3*K4[i] + RK4_4*K5[i].
	movsd [rdx], xmm0				; Store result in Y[i].

	inc eax
	cmp eax, dword [N]
	je .rkf_done

	; Increment the pointers to next double within the K5_p, Ytemp, Y, K1_p, K2_p, K3_p and K4_p arrays.
	lea rdi, [rdi + 8]
	lea rsi, [rsi + 8]
	lea rbx, [rbx + 8]
	lea rcx, [rcx + 8]
	lea rdx, [rdx + 8]
	
	jmp .rk4_loop
	
.rkf_done:

	; epilogue
	
; 	push rbp
; 	mov rbp, rsp
; 	push rbx
; 	push rdx						; As δt is provided as a reference, we need to save the address in order to write the change back to the client.
	
	; As δt was a reference, we need to update the reference.
	movsd xmm0, [δt]
	pop rdx
	movsd [rdx], xmm0
	
	pop rbx
	pop rbp
	ret

solver_rkf_initialize:
	; static void initialize(void* ode_solver::rkfsolver<N> parent, void* functor, const unsigned int* offsets, const double* constants)
	; rdi = this pointer to object of class ode_solver::rkfsolver<N>.
	; rsi = pointer to functor object provided to ode_solver::rkfsolver<N> object.
	; rdx = pointer to the static ode_solver::rkfsolver<N>::offsets array.
	; rcx = pointer to the array of constant doubles used in the calculation
	
	; prologue
	push rbp
	mov rbp, rsp
	push rbx
	push r14
	
	mov [this], rdi
	mov [functor], rsi				; Set the pointer to the functor provided to ode_solver::rkfsolver<N> object.
	
	; K1_p = this + offsets[0]
	mov eax, dword [rdx]			; Read unsigned int from offsets[0].
	mov rbx, [this]
	add rbx, rax					; K1_p = this + offsets[0].
	mov [K1_p], rbx
	
	; K2_p = this + offsets[1]
	mov eax, dword [rdx+4]
	mov rbx, [this]
	add rbx, rax					; Add the offset to this,
	mov [K2_p], rbx					; and store the address in K2_p.
	
	; K3_p = this + offsets[2]
	mov eax, dword [rdx+8]
	mov rbx, [this]
	add rbx, rax					; Add the offset to this,
	mov [K3_p], rbx					; and store the address in K3_p.
	
	; K4_p = this + offsets[3]
	mov eax, dword [rdx+12]
	mov rbx, [this]
	add rbx, rax					; Add the offset to this,
	mov [K4_p], rbx					; and store the address in K4_p.
	
	; K5_p = this + offsets[4]
	mov eax, dword [rdx+16]
	mov rbx, [this]
	add rbx, rax					; Add the offset to this,
	mov [K5_p], rbx					; and store the address in K4_p.
	
	; K6_p = this + offsets[5]
	mov eax, dword [rdx+20]
	mov rbx, [this]
	add rbx, rax					; Add the offset to this,
	mov [K6_p], rbx					; and store the address in K4_p.
	
	; Ytemp = this + offsets[6]
	mov eax, dword [rdx+24]
	mov rbx, [this]
	add rbx, rax					; Add the offset to this,
	mov [Ytemp], rbx				; and store the address in Ytemp.
	
	; R_p = this + offsets[7]
	mov eax, dword [rdx+28]
	mov rbx, [this]
	add rbx, rax					; Add the offset to this,
	mov [R_p], rbx					; and store the address in K4_p.
	
	; _f = this + offset[8]
	mov eax, dword [rdx+32]
	mov rbx, [this]					
	add rbx, rax					; Add the offset to the functor's pointer to the this pointer,
	mov rbx, [rbx]					; to obtain the pointer to ode_solver::base_f<N> object.
	mov rbx, [rbx]					; As the address of base_f<N> and its member base_f<N>::_f coincide, due to it being the only member of the class,
									; dereferencing base_f<N> yields the address of its callback, stored in base_f<N>::_f.
	mov [f], rbx					; Store the address in f.
	
	; _N = this + offset[9]
	xor rax, rax
	mov eax, dword [rdx+36]
	mov rbx, [this]					
	add rbx, rax					; Add the offset to this,
	mov rbx, [rbx]					; N is a value not a pointer, so dereference it before assignment.
	mov [N], rbx
	
	; _δt_min = this + offset[10]
	mov eax, dword [rdx+40]			; Obtain the offset to _δt_min.
	mov rbx, [this]					; Obtain the this pointer.
	add rbx, rax					; Add the offset to the this pointer.
	mov rbx, [rbx]					; De-reference the location, and
	mov [δt_min], rbx				; assign the value to δt_min.
	
	; _δt_max = this + offset[11]
	mov eax, dword [rdx+44]			; Obtain the offset to _δt_max.
	mov rbx, [this]					; Obtain the this pointer.
	add rbx, rax					; Add the offset to the this pointer.
	mov rbx, [rbx]					; De-reference the location, and
	mov [δt_max], rbx				; assign the value to δt_max.
	
	; _tolerance = this + offset[12]
	mov eax, dword [rdx+48]			; Obtain the offset to _tolerance.
	mov rbx, [this]					; Obtain the this pointer.
	add rbx, rax					; Add the offset to the this pointer.
	movsd xmm0, [rbx]				; De-reference the location, and
	movsd [tolerance], xmm0			; assign the value to tolerance.

	; Load the constants needed for RKF
	
	; K2_1 = [rcx] + 0
	lea rax, [K2_1]
	movsd xmm0, [rcx]
	movsd [rax], xmm0
	
	; K3_1 = [rcx] + 8
	lea rax, [K3_1]
	movsd xmm0, [rcx + 8]
	movsd [rax], xmm0
	
	; K3_2 = [rcx] + 16
	lea rax, [K3_2]
	movsd xmm0, [rcx + 16]
	movsd [rax], xmm0
	
	; K4_1 = [rcx] + 24
	lea rax, [K4_1]
	movsd xmm0, [rcx + 24]
	movsd [rax], xmm0
	
	; K4_2 = [rcx] + 32
	lea rax, [K4_2]
	movsd xmm0, [rcx + 32]
	movsd [rax], xmm0
	
	; K4_3 = [rcx] + 40
	lea rax, [K4_3]
	movsd xmm0, [rcx + 40]
	movsd [rax], xmm0
	
	; K5_1 = [rcx] + 48
	lea rax, [K5_1]
	movsd xmm0, [rcx + 48]
	movsd [rax], xmm0
	
	; K5_2 = [rcx] + 56
	lea rax, [K5_2]
	movsd xmm0, [rcx + 56]
	movsd [rax], xmm0
	
	; K5_3 = [rcx] + 64
	lea rax, [K5_3]
	movsd xmm0, [rcx + 64]
	movsd [rax], xmm0
	
	; K5_4 = [rcx] + 72
	lea rax, [K5_4]
	movsd xmm0, [rcx + 72]
	movsd [rax], xmm0
	
	; K6_1 = [rcx] + 80
	lea rax, [K6_1]
	movsd xmm0, [rcx + 80]
	movsd [rax], xmm0
	
	; K6_2 = [rcx] + 88
	lea rax, [K6_2]
	movsd xmm0, [rcx + 88]
	movsd [rax], xmm0
	
	; K6_3 = [rcx] + 96
	lea rax, [K6_3]
	movsd xmm0, [rcx + 96]
	movsd [rax], xmm0
	
	; K6_4 = [rcx] + 104
	lea rax, [K6_4]
	movsd xmm0, [rcx + 104]
	movsd [rax], xmm0
	
	; K6_5 = [rcx] + 112
	lea rax, [K6_5]
	movsd xmm0, [rcx + 112]
	movsd [rax], xmm0
	
	; R_1 = [rcx] + 120
	lea rax, [R_1]
	movsd xmm0, [rcx + 120]
	movsd [rax], xmm0
	
	; R_2 = [rcx] + 128
	lea rax, [R_2]
	movsd xmm0, [rcx + 128]
	movsd [rax], xmm0
	
	; R_3 = [rcx] + 136
	lea rax, [R_3]
	movsd xmm0, [rcx + 136]
	movsd [rax], xmm0
	
	; R_4 = [rcx] + 144
	lea rax, [R_4]
	movsd xmm0, [rcx + 144]
	movsd [rax], xmm0
	
	; R_5 = [rcx] + 152
	lea rax, [R_5]
	movsd xmm0, [rcx + 152]
	movsd [rax], xmm0
	
	; RK4_1 = [rcx] + 160
	lea rax, [RK4_1]
	movsd xmm0, [rcx + 160]
	movsd [rax], xmm0
	
	; RK4_2 = [rcx] + 168
	lea rax, [RK4_2]
	movsd xmm0, [rcx + 168]
	movsd [rax], xmm0
	
	; RK4_3 = [rcx] + 176
	lea rax, [RK4_3]
	movsd xmm0, [rcx + 176]
	movsd [rax], xmm0
	
	; RK4_4 = [rcx] + 184
	lea rax, [RK4_4]
	movsd xmm0, [rcx + 184]
	movsd [rax], xmm0
	
	; q_coeff = [rcx] + 192
	lea rax, [q_coeff]
	movsd xmm0, [rcx + 192]
	movsd [rax], xmm0
	
	; twelve_thirteenths = [rcx] + 200
	lea rax, [twelve_thirteenths]
	movsd xmm0, [rcx + 200]
	movsd [rax], xmm0
	
	lea rbx, Initialized			; Set the Initialized flag to TRUE.
	mov [rbx], byte TRUE
	
	; epilogue
	
; 	push rbp
; 	mov rbp, rsp
; 	push rbx
; 	push r14
	
	pop r14
	pop rbx
	pop rbp
	ret
	
solver_rkf_ready:
	; bool asmReady() const
	push rbp,
	mov rbp, rsp
	
	lea rax, Initialized
	;mov al, byte [rax]				; This will suffice, but bits [8, 63] will not be cleared to zero.
	mov rax, [rax]					; Instead, we use rax to clear all higher order bits.

	pop rbp
	ret
	
solver_rfk_update_restrictions:
	; static void updateRestrictions(void* parent, const unsigned int* offsets)
	; rdi = this pointer of parent.
	; rsi = pointer to the array of offsets.
	push rbp
	mov rbp, rsp
	
	mov rcx, [this]
	
	; _δt_min = this + offset[10]
	mov eax, dword [rdx+40]			; Obtain the offset to _δt_min.
	mov rcx, [this]					; Obtain the this pointer.
	add rcx, rax					; Add the offset to the this pointer.
	mov rcx, [rcx]					; De-reference the location, and
	mov [δt_min], rcx				; assign the value to δt_min.
	
	; _δt_max = this + offset[11]
	mov eax, dword [rdx+44]			; Obtain the offset to _δt_max.
	mov rcx, [this]					; Obtain the this pointer.
	add rcx, rax					; Add the offset to the this pointer.
	mov rcx, [rcx]					; De-reference the location, and
	mov [δt_max], rcx				; assign the value to δt_max.
	
	; _tolerance = this + offset[12]
	mov eax, dword [rdx+48]			; Obtain the offset to _tolerance.
	mov rcx, [this]					; Obtain the this pointer.
	add rcx, rax					; Add the offset to the this pointer.
	movsd xmm0, [rcx]				; De-reference the location, and
	movsd [tolerance], xmm0			; assign the value to tolerance.
	
	; epilogue
	pop rbp
	ret
	
print_rkf_Ytemp:
	; private void solver::print_Ytemp(int n)
	; Used internally for debug purposes.
	; calls printf(Ytemp_str, int n), additionally using 4 doubles in xmm0-xmm3, per the Ytemp_str format string.
	; Note that printf is variadic and is extremely sensitive to 16-byte alignment of the rsp.
	
	; prologue:
	push rbp						; Restore 16-byte alignment and establish the stack frame.
	mov rbp, rsp
	push rax
	push rdi

	; TODO -- Remove this from the github version!
	; https://stackoverflow.com/questions/10324333/does-printf-require-additional-stack-space-on-the-x86-64
	; OK, rsp has be to be 16-byte aligned, and not 8!
	; Originally here, we had 3 push operations, rbp, rax, and rdi.  For that case, we needed to subtract
	; 16 bytes from the rsp.  With the mess have here (K4 variant), we need to use 8 to restore 16-byte
	; alignment.  It has to involve parity of push operations.  Note that this is required before the call.

	push rbx
	push rsi
	push r10
	push r11
	push r12
	push r13
	push r14
	sub rsp, 8						; We pushed odd number of registers after rbp, so restore the 16-byte alignment.

	mov rsi, rdi					; param 1 = n, and will be param 2 of printf().
	mov rdi, [Ytemp]
	movsd xmm0, [rdi + 0]
	movsd xmm1, [rdi + 8]
	movsd xmm2, [rdi + 16]
	movsd xmm3, [rdi + 24]
	mov eax, 4						; As we make use of 4 SSE == vector registers, indicate them in al.
	lea rdi, Ytemp_str				; param 1 of printf() = format string.
	call printf wrt ..plt

	; epilogue
	
; 	push rbp						; Restore 16-byte alignment and establish the stack frame.
; 	mov rbp, rsp
; 	push rax
; 	push rdi
; 	push rbx
; 	push rsi
; 	push r10
; 	push r11
; 	push r12
; 	push r13
; 	push r14
; 	sub rsp, 8		
	
	add rsp, 8
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop rsi
	pop rbx
	pop rdi
	pop rax
	pop rbp
	ret



	
