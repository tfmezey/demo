default rel

extern printf
global solver_rk4_asm, solver_rk4_initialize, solver_rk4_ready
global print_rk4_Ytemp				; Used for debug purposes.

section .rodata
	%include "globals.asm"

	; Count of offsets measured from the ode_solver::rk4solver<N> this pointer, needed to point to the correct double arrays.
	offsetCount EQU 7
	
	half dq 0.5
	two dq 2.0
	six dq 6.0
	
	Ytemp_str db "    K%d:  Ytemp[0] = %4.8f, Ytemp[1] = %4.8f, Ytemp[2] = %4.8f, Ytemp[3] = %4.8f",LF,0		; Used for debug purposes.
	
section .data
	functor dq nullptr				; Pointer to the functor provided to ode_solver::rk4solver<N> object.
	f dq nullptr					; Will hold the address of the functor callback member function, which will require [functor] as its first parameter.
	this dq nullptr					; Placeholder for the ode_solver::rk4solver<N> this pointer.
	Ytemp dq nullptr				; Internal arrays of doubles, each of length N.
	K1_p dq nullptr
	K2_p dq nullptr
	K3_p dq nullptr
	K4_p dq nullptr
	Y dq nullptr					; External ode_solver::state<N> == double[N] state pointer, provided by the caller of solver_rk4_asm().
	
	δt dq 0.0						; This is the time step dt.
	t dq 0.0						; This is the current time.
	N dd 4							; N is the dimension of the external state vector X.
	Initialized db FALSE			; Returend by solver_ready(), and set by solver_initialize(), which must be called, less we want segfaulting.
	
section .text
solver_rk4_asm:
	; void rk4_asm(parent* this, double* Y, double t, double dt) = void solver::rk4_asm(parent* this, ode_solver::state<N> Y, double t, double dt)
	; rdi = implicit this pointer to parent.  Here the parent is either ode_solver::rk4solver<N> if it derives from class assembly_functions, or class
	;		assembly_functions, if that class is a member of class ode_solver::rk4solver<N>.  Either way, the parent pointer is not used.
	; rsi = pointer to Y.
	; xmm0 = t.
	; xmm1 = h.
	
	; prologue
	push rbp
	mov rbp, rsp
	push rbx
	push r12
	push r13
	push r14						; Even number of push operations since 'push rbp' ensure that rsp % 16 == 0.
	
	movsd [t], xmm0
	movsd [δt], xmm1
	mov [Y], rsi
	
	;	(*_f)(Y, K1, t);
	mov rdi, [functor]				; The functor "this" pointer is the first parameter to the class's member function.
	mov rsi, [Y]					; Y is the second parameter.
	mov rdx, [K1_p]					; K1 is the third parameter.
	
	movsd xmm0, [t]					; fourth param = t.
	
	call [f]						; Calling (*_f)(Y, K1, t) to obtain K1/δt.
	
	; 	{ _K1[i] *= δt; _Ytemp[i] = Y[i] + 0.5*_K1[i]; }
	mov rdi, [K1_p]					; Set up our pointers.
	mov rsi, [Ytemp]
	mov rbx, [Y]
	
	xor rax, rax					; Initialize loop counter.
.for_K1:

	; 	K1[i] *= δt;
	movsd xmm0, [rdi]				; Retrieve K1[i],
	mulsd xmm0, [δt]				; multiply it by δt,
	movsd [rdi], xmm0				; and store the result at K1[i].
	; 	Ytemp[i] = Y[i] + 0.5*K1[i];
	mulsd xmm0, [half]				; Multiply K1[i] by 0.5,
	addsd xmm0,	[rbx]				; add Y[i] to it:  xmm0 = Y[i] + 0.5*K1[i],
	movsd [rsi], xmm0				; and store it at Ytemp[i].
	
	inc eax
	cmp eax, dword [N]
	je .K2_prep

	; Increment the pointers to next double within the K1_p, Ytemp amd Y arrays.
	lea rdi, [rdi + 8]
	lea rsi, [rsi + 8]
	lea rbx, [rbx + 8]
	
	jmp .for_K1
	
.K2_prep:

	;	(*_f)(Ytemp, K2, t + 0.5*δt);
	mov rdi, [functor]				; The functor "this" pointer is the first parameter to the class's member function.
	mov rsi, [Ytemp]				; Ytemp is the second parameter.
	mov rdx, [K2_p]					; K2_p is the third parameter.
	movsd xmm0, [δt]
	mulsd xmm0, [half]
	addsd xmm0, [t]					; param4 = t + 0.5*δt.
	
	call [f]						; Calling (*_f)(Ytemp, K2, t + 0.5*δt) to obtain K2/δt.
	
	;	{_K2[i] *= δt; _Ytemp[i] = Y[i] + 0.5*_K2[i]; }					
	mov rdi, [K2_p]					; Set up our pointers.
	mov rsi, [Ytemp]
	mov rbx, [Y]
	xor rax, rax
	
.for_K2:
	; 	K2[i] *= δt;
	movsd xmm0, [rdi]				; Retrieve K2[i],
	mulsd xmm0, [δt]				; multiply it by δt,
	movsd [rdi], xmm0				; and store the result at K2[i].
	; 	Ytemp[i] = Y[i] + 0.5*K2[i];
	mulsd xmm0, [half]				; Multiply K2[i] by 0.5,
	addsd xmm0,	[rbx]				; add Y[i] to it:  xmm0 = Y[i] + 0.5*K2[i],
	movsd [rsi], xmm0				; and store it at Ytemp[i].
	
	inc eax
	cmp eax, dword [N]
	je .K3_prep

	; Increment the pointers to next double within the K2_p, Ytemp, and Y arrays.
	lea rdi, [rdi + 8]
	lea rsi, [rsi + 8]
	lea rbx, [rbx + 8]
	
	jmp .for_K2
	
.K3_prep:

; 	; 	(*_f)(Ytemp, K3, t + 0.5*δt);
	mov rdi, [functor]				; The functor "this" pointer is the first parameter to the class's member function.
	mov rsi, [Ytemp]				; Ytemp is the second parameter.
	mov rdx, [K3_p]					; K3 is the third parameter.
	movsd xmm0, [δt]
	mulsd xmm0, [half]
	addsd xmm0, [t]					; param4 = t + 0.5*δt.
	
	call [f]						; Calling (*_f)(Ytemp, K3, t + 0.5*δt) to obtain K3/δt.
	
	;	{ _K3[i] *= δt; _Ytemp[i] = Y[i] + _K3[i]; }
	mov rdi, [K3_p]					; Set up our pointers.
	mov rsi, [Ytemp]
	mov rbx, [Y]
	xor rax, rax
	
.for_K3:
	; 	K3[i] *= δt;
	movsd xmm0, [rdi]				; Retrieve K3[i],
	mulsd xmm0, [δt]				; multiply it by δt,
	movsd [rdi], xmm0				; and store the result at K3[i].
	; 	Ytemp[i] = Y[i] + K3[i];
	addsd xmm0, [rbx]				; Add Y[i] to K3[i],
	movsd [rsi], xmm0				; and store the result.
	
	inc eax
	cmp eax, dword [N]
	je .K4_prep

	; Increment the pointers to next double within the K3_p, Ytemp, and Y arrays.
	lea rdi, [rdi + 8]
	lea rsi, [rsi + 8]
	lea rbx, [rbx + 8]
	
	jmp .for_K3

.K4_prep:

	;	(*_f)(Ytemp, K4, t + δt);
	mov rdi, [functor]				; The functor "this" pointer is the first parameter to the class's member function.
	mov rsi, [Ytemp]				; Ytemp is the second parameter.
	mov rdx, [K4_p]					; K4 is the third parameter.
	movsd xmm0, [δt]
	addsd xmm0, [t]					; param4 = t + δt.
	
	call [f]						; Calling (*_f)(Ytemp, K4, t + δt) to obtain K4/δt.
	
	;	{ _K4[i] *= δt; Y[i] = Y[i] + (_K1[i] + 2.0*_K2[i] + 2.0*_K3[i] + _K4[i])/6; }
	mov r10, [K1_p]
	mov r11, [K2_p]
	mov r12, [K3_p]
	mov r13, [K4_p]					; Set up our pointers.
	mov r14, [Y]
	movsd xmm5, [two]
	movsd xmm6, [six]
	xor rax, rax
	
.for_F4:
	;	K4[i] *= δt;
	movsd xmm0, [r13]				; Retrieve K4[i],
	mulsd xmm0, [δt]				; multiply it by δt,
	movsd [r13], xmm0				; and store the result at K4[i].
	
	;	Y[i] = Y[i] + (K1[i] + 2.0*K2[i] + 2.0*K3[i] + K4[i])/6;
	movsd xmm1,	[r14]				; Y[i]
	movsd xmm2, [r10]				; K1[i]
	movsd xmm3, [r11]				; K2[i]
	mulsd xmm3, xmm5				; 2*K2[i]
	movsd xmm4, [r12]				; K3[i]
	mulsd xmm4, xmm5				; 2*K3[i]
	
	addsd xmm0, xmm2				; K4[i] + K1[i]
	addsd xmm0, xmm3				; K4[i] + K1[i] + 2.0*K2[i]
	addsd xmm0, xmm4				; K4[i] + K1[i] + 2.0*K2[i] + 2.0*K3[i]
	divsd xmm0, xmm6				; (K4[i] + K1[i] + 2.0*K2[i] + 2.0*K2[i])/6
	addsd xmm0, xmm1				; Y[i] + (K4[i] + K1[i] + 2.0*K2[i] + 2.0*K2[i])/6 
	movsd [r14], xmm0				; Save it in Y[i].
	
	inc eax
	cmp eax, dword [N]
	je .rk4_done

	; Increment the pointers to next double within the K1, ... , K4, Ytemp, and Y arrays.
	lea r10, [r10 + 8]
	lea r11, [r11 + 8]
	lea r12, [r12 + 8]
	lea r13, [r13 + 8]
	lea r14, [r14 + 8]
	
	jmp .for_F4

.rk4_done:
	; epilogue
	
; 	push rbp
; 	mov rbp, rsp
; 	push rbx
; 	push r12
; 	push r13
; 	push r14

	pop r14
	pop r13
	pop r12
	pop rbx
	pop rbp
	ret

solver_rk4_initialize:
	; static void initialize(void* ode_solver::rk4solver<N>, void* functor, const unsigned int*)
	; rdi = this pointer to object of class ode_solver::rk4solver<N>.
	; rsi = pointer to functor object provided to ode_solver::rk4solver<N> object.
	; rdx = pointer to the static ode_solver::rk4solver<N>::offsets array.
	
	; prologue
	push rbp
	mov rbp, rsp
	push rbx
	;  Note that we have an even number of push opreations.
	;  However, as we do not call any functions here, we do not have to worry about the stack pointer being 16-byte aligned.
	
	mov [this], rdi
	mov [functor], rsi				; Set the pointer to the functor provided to ode_solver::rk4solver<N> object.
	
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
	
	; Ytemp = this + offsets[4]
	mov eax, dword [rdx+16]
	mov rbx, [this]
	add rbx, rax					; Add the offset to this,
	mov [Ytemp], rbx				; and store the address in Ytemp.
	
	; _f = this + offset[5]
	mov eax, dword [rdx+20]
	mov rbx, [this]					
	add rbx, rax					; Add the offset to the functor's pointer to the this pointer,
	mov rbx, [rbx]					; to obtain the pointer to ode_solver::base_f<N> object.
	mov rbx, [rbx]					; As the address of base_f<N> and its member base_f<N>::_f coincide, due to it being the only member of the class,
									; dereferencing base_f<N> yields the address of its callback, stored in base_f<N>::_f.
	mov [f], rbx					; Store the address in f.
	
	; _N = this + offset[6]
	xor rax, rax
	mov eax, dword [rdx+24]
	mov rbx, [this]					
	add rbx, rax					; Add the offset to this,
	mov rbx, [rbx]					; N is a value not a pointer, so dereference it before assignment.
	mov [N], rbx
	
	lea rbx, Initialized			; Set the Initialized flag to TRUE.
	mov [rbx], byte TRUE
	
	; epilogue
	
; 	push rbp
; 	mov rbp, rsp
; 	push rbx
	
	pop rbx
	pop rbp
	ret
	
solver_rk4_ready:
	; bool asmReady() const
	lea rax, Initialized
	;mov al, byte [rax]				; This will suffice, but bits [8, 63] will not be cleared to zero.
	mov rax, [rax]					; Instead, we use rax to clear all higher order bits.

	ret
	
print_rk4_Ytemp:
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
; 	sub rsp, 8						; We pushed odd number of registers after rbp, so restore the 16-byte alignment.
	
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


