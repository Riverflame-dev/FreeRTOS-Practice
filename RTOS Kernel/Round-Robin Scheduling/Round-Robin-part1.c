// UARTIntsTestMain.c
// Runs on LM4F120/TM4C123
// Tests the UART0 to implement bidirectional data transfer to and from a
// computer running HyperTerminal.  This time, interrupts and FIFOs
// are used.
// Daniel Valvano
// September 12, 2013

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Program 5.11 Section 5.6, Program 3.10

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// U0Rx (VCP receive) connected to PA0
// U0Tx (VCP transmit) connected to PA1

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "PLL.h"
#include "os.h"

// Functions from startup.s
void EnableInterrupts(void);
void DisableInterrupts(void);
long StartCritical(void);
void EndCritical(long st);

#define PF1   (*((volatile uint32_t *)0x40025008))  // red LED
#define PF2   (*((volatile uint32_t *)0x40025010))  // blue LED
#define PF3   (*((volatile uint32_t *)0x40025020))  // green LED
	
// initialize 
void PortF_Init(void) {
  SYSCTL_RCGCGPIO_R |= 0x20;                       // 1) activate clock for Port F
  while((SYSCTL_PRGPIO_R&0x20)==0){};              // allow time for clock to start
                                                   // 2) no need to unlock PF2
  GPIO_PORTF_PCTL_R &= ~0x0000FFF0;                // 3) regular GPIO PF1, PF2, PF3
  GPIO_PORTF_AMSEL_R &= ~(1u<<1|1u<<2|1u<<3);      // 4) disable analog function on PF1, PF2, PF3
  GPIO_PORTF_DIR_R |= (1u<<1|1u<<2|1u<<3);         // 5) set direction to output PF2
  GPIO_PORTF_AFSEL_R &= ~(1u<<1|1u<<2|1u<<3);      // 6) regular port function
  GPIO_PORTF_DEN_R |= (1u<<1|1u<<2|1u<<3);         // 7) enable digital port		
}

unsigned long Count1; // number of times thread1 loops 
unsigned long Count2; // number of times thread2 loops 
unsigned long Count3; // number of times thread3 loops 

void Thread1(void){
		Count1 = 0; 
		for(;;){
				PF1 ^= 0x02;	// heartbeat 
			  Count1++;
		}
}

void Thread2(void){ 
		Count2 = 0; 
		for(;;){
				PF2 ^= 0x04;	// heartbeat 
			  Count2++;
		}
}

void Thread3(void){ 
		Count3 = 0; 
		for(;;){
				PF3 ^= 0x08;	// heartbeat 
			  Count3++;
		}
}

int main(void){	      // Testmain1
		OS_Init();	      // initialize, disable interrupts 
		PortF_Init();	    // profile user threads 
	  uint32_t NumCreated = 0 ;
		NumCreated += OS_AddThread(&Thread1,128); 
		NumCreated += OS_AddThread(&Thread2,128); 
		NumCreated += OS_AddThread(&Thread3,128);
		// Count1 Count2 Count3 should be equal or off by one at all times 
		OS_Launch();     // doesn't return, interrupts enabled in here 
		return 0;	       // this never executes
}
