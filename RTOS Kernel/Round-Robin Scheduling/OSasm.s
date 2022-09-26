
;/*****************************************************************************/
;/* OSasm.s: low-level OS commands, written in assembly                       */
;// Real Time Operating System 

; This example accompanies the book
;  "Embedded Systems: Real Time Interfacing to the Arm Cortex M3",
;  ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2011
;
;  Programs 6.4 through 6.12, section 6.2
;
;Copyright 2011 by Jonathan W. Valvano, valvano@mail.utexas.edu
;    You may use, edit, run or distribute this file
;    as long as the above copyright notice remains
; THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
; OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
; MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
; VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
; OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
; For more information about my classes, my research, and my books, see
; http://users.ece.utexas.edu/~valvano/
; */

NVIC_INT_CTRL   EQU     0xE000ED04                              ; Interrupt control state register.
NVIC_SYSPRI14   EQU     0xE000ED22                              ; System priority register (priority 14).
NVIC_PENDSV_PRI EQU           0xFF                              ; PendSV priority value (lowest).
NVIC_PENDSVSET  EQU     0x10000000                              ; Value to trigger PendSV exception.

        AREA |.text|, CODE, READONLY, ALIGN=2
        THUMB
        REQUIRE8
        PRESERVE8

        EXTERN  RunPt            ; currently running thread
		EXTERN  NextPt           ; next thread to run

        EXPORT  OS_DisableInterrupts
        EXPORT  OS_EnableInterrupts
        EXPORT  StartOS
	    EXPORT  OS_bSignal
		EXPORT  OS_bWait
	    EXPORT  OS_Signal
		EXPORT  OS_Wait
		EXPORT  OS_TASK_SW
		EXPORT  PendSV_Handler 


;********************************************************************************************************
;                               PERFORM A CONTEXT SWITCH (From task level)
;                                           void OSCtxSw(void)
;
; Note(s) : 1) OS_TASK_SW is called when OS wants to perform a task context switch.  This function
;              triggers the PendSV exception which is where the real work is done.
; refer to uC/OS-II
; 
;********************************************************************************************************

OS_TASK_SW
    LDR     R0, =NVIC_INT_CTRL                                  ; Trigger the PendSV exception (causes context switch)
    LDR     R1, =NVIC_PENDSVSET
    STR     R1, [R0]
    BX      LR
; 

;*********** OS_DisableInterrupts ***************
; disable interrupts
; inputs:  none
; outputs: none
OS_DisableInterrupts
        CPSID   I
        BX      LR

;*********** OS_EnableInterrupts ***************
; disable interrupts
; inputs:  none
; outputs: none
OS_EnableInterrupts
        CPSIE   I
        BX      LR


StartOS
    LDR     R0, =RunPt         ; currently running thread
    LDR     R2, [R0]           ; R2 = value of RunPt
    LDR     SP, [R2]           ; new thread SP; SP = RunPt->stackPointer;
    POP     {R4-R11}           ; restore regs r4-11
    POP     {R0-R3}            ; restore regs r0-3
    POP     {R12}
    POP     {LR}               ; discard LR from initial stack
    POP     {LR}               ; start location
    POP     {R1}               ; discard PSR
    CPSIE   I                  ; Enable interrupts at processor level
    BX      LR                 ; start first thread


; ******** OS_Wait ************
; decrement semaphore and spin/block if less than zero
; input:  pointer to a counting semaphore
; output: none
OS_Wait
    LDREX R1, [R0]
    SUBS R1, #1
    ITT PL
    STREXPL R2, R1, [R0]
    CMPPL R2, #0
    BNE OS_Wait
	BX LR

; ******** OS_Signal ************
; increment semaphore, wakeup blocked thread if appropriate 
; input:  pointer to a counting semaphore
; output: none   
OS_Signal
    LDREX R1, [R0]
    ADD R1, #1
    STREX R2, R1, [R0]
    CMP R2, #0
    BNE OS_Signal
    BX LR

; ******** OS_bWait ************
; if the semaphore is 0 then spin/block
; if the semaphore is 1, then clear semaphore to 0
; input:  pointer to a binary semaphore
; output: none
OS_bWait
    MOV R1, #0
    LDREX R2, [R0]
    CMP R2, #1
    ITT EQ
    STREXEQ R2, R1, [R0]
    CMPEQ R2, #0
    BNE OS_bWait
	BX LR

; ******** OS_bSignal ************
; set semaphore to 1, wakeup blocked thread if appropriate 
; input:  pointer to a binary semaphore
; output: none  
OS_bSignal
    LDREX R1, [R0]
    MOV R1, #1
    STREX R2, R1, [R0]
    CMP R2, #0
    BNE OS_bSignal
    BX LR

; only register R4-R11 need to be saved, r0, r1,r2,r3 temporary not preserved;  r12 frame register not used; r13 sp r14 lr, r15 pc

PendSV_Handler                 ; 1) Saves R4-R11,R12,LR,PC,PSR
    CPSID   I                  ; 2) Prevent interrupt during switch
    PUSH    {R4-R11}           ; 3) Save remaining regs r4-11
    LDR     R0, =RunPt         ; 4) R0=pointer to RunPt, old thread
    LDR     R1, [R0]           ;    R1 = RunPt
    STR     SP, [R1]           ; 5) Save SP into TCB
	
	LDR 	R1, =NextPt        ; 6) R1=pointer to NextPt, next thread
	LDR		R1, [R1]		   ;    R1 = NextPt
	STR		R1, [R0] 		   ; 7) Update Runpt to NextPt
    LDR     SP, [R1]           ; 8) new thread SP; SP = RunPt->sp;

    POP     {R4-R11}           ; 9) restore regs r4-11
    CPSIE   I                  ; 10) tasks run with interrupts enabled
    BX      LR                 ; 11) restore R0-R3,R12,LR,PC,PSR

    
	ALIGN
	END
  