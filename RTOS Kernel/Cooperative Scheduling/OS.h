

						   
#ifndef __OS_H
#define __OS_H  1

#include <stdint.h>
// fill these depending on your clock (Currently for 80Mhz clock)
#define TIME_1MS  80000  
#define TIME_2MS  2*TIME_1MS


// ******** OS_Init ************
// initialize operating system, disable interrupts until OS_Launch
// initialize OS controlled I/O: serial, ADC, systick, select switch and timer2
// input:  none
// output: none
void OS_Init(void);



// ******* OS_Launch *********************
// start the scheduler, enable interrupts
// Inputs: number of ms for each time slice
// Outputs: none (does not return)
void OS_Launch(void);
// OS scheduler to find next running task for cooperative scheduling

void OS_Schedule(void);
int OS_AddThread(void(*task)(void), uint32_t stackSize);

void OS_Suspend(void); 
#endif
