; globals.asm
;default rel

; 29th Mary 2024 -- 1742 CET
; We still have problems when we had the variables and their initial values.  Converting
; everything to EQU fixed this.  However, the Newline cannot work, as the equ directive
; sets a label to a value, and we cannot have constant arrays.  As a consequence, we
; commented it out.
; Note that this make things easier, as we no longer have to worry about fucking PIE
; complaints from the linker.

; default rel
; global LF, NULLchar, nullptr
; global TRUE, FALSE
; global STDIN, STDOUT, STDERR
; global newline
; global SYS_read, SYS_write, SYS_open, SYS_close, SYS_fork, SYS_exit, SYS_creat, SYS_time
; global EXIT_SUCCESS

; section .rodata

%ifndef GLOBALS_ASM
%define GLOBALS_ASM

; 	section .rodata
	nullptr			EQU 0				; 64-bit nullptr
	NULLchar		EQU 0

 	TRUE			EQU 1
 	FALSE			EQU 0

	STDIN			EQU 0
	STDOUT			EQU 1
	STDERR			EQU 2

	LF				EQU 10				; line feed

	;System Call Codes:
	SYS_read		EQU 0
	SYS_write		EQU 1
	SYS_open		EQU 2
	SYS_close		EQU 3
	SYS_fork		EQU 57
	SYS_exit		EQU 60
	SYS_creat		EQU 85				; file open/create
	SYS_time		EQU 201

; 	newline			db 10, 0			; As this is a compound value, it cannot be defined with dw.
	EXIT_SUCCESS	EQU 0

%endif

; %ifndef GLOBALS_ASM
; %define GLOBALS_ASM
; 
; ; 	section .rodata
; 	nullptr			dq 0				; 64-bit nullptr
; 	NULLchar		db 0
; 
;  	TRUE			dq 1
;  	FALSE			dq 0
; 
; 	STDIN			dd 0
; 	STDOUT			dd 1
; 	STDERR			dd 2
; 
; 	LF				db 10				; line feed
; 
; 	;System Call Codes:
; 	SYS_read		dd 0
; 	SYS_write		dd 1
; 	SYS_open		dd 2
; 	SYS_close		dd 3
; 	SYS_fork		dd 57
; 	SYS_exit		dd 60
; 	SYS_creat		dd 85				; file open/create
; 	SYS_time		dd 201
; 
; 	newline			db 10, 0			; As this is a compound value, it cannot be defined with dw.
; 	EXIT_SUCCESS	dd 0
; 
; %endif
	
