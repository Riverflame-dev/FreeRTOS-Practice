// ece 485 lab 1 os.c

#include <stdint.h> 
#include "tm4c123gh6pm.h"
#include "pll.h"
#include "os.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(uint32_t sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
void OS_TASK_SW(void); 
void StartOS(void); 

#define MAXTHREADS 8    // maximum number of threads
#define STACKSIZE  512  // number of 32-bit words in stack
#define FIFOSIZE 256
#define INVALID 0
#define VALID 1

#define FAILURE 0
#define SUCCESS 1
// TCB structure, assembly code depends on the order of variables
// in this structure. Specifically sp, next and sleepState.
struct tcb{
   uint32_t *sp;          // pointer to stack (valid for threads not running)
   struct tcb *next, *prev;  // linked-list pointer
   uint32_t sleepState, blockedState;
   uint32_t id, priority;
   uint8_t valid;
   uint32_t stack[STACKSIZE]; 
};

// Global variables for TCBs
typedef struct tcb tcbType;
tcbType tcbs[MAXTHREADS];
tcbType *RunPt = '\0';
tcbType *NextPt = '\0';
tcbType *Head = '\0';
tcbType *Tail = '\0';
uint32_t NumThreads = 0; // global index to point to place to put new tcb in array

uint32_t gTimeSlice;

void OS_Schedule(void)
{
	 NextPt = (*RunPt).next;

//	 RunPt = (*RunPt).next;  
//   NextPt = (*NextPt).next;
	 
	
}
// ********* Scheduler *************
// Calculates next thread to be run and sets NextPt to it
void OS_Suspend(void) {
   OS_Schedule(); 
   OS_TASK_SW(); 
}

// ************ OS_Init ******************
// initialize operating system, disable interrupts until OS_Launch
// initialize OS controlled I/O: serial, ADC, 
// input: none
// output: non
void OS_Init(void) {

	int i; // Used for indexing

	// Disable interrupts
	DisableInterrupts();

	PLL_Init(Bus80MHz);       // set system clock to 80 Mhz
	
	// Initializing TCBs
	for(i = 0; i < MAXTHREADS; i++) {
	  tcbs[i].valid = INVALID;
	}

	RunPt = &tcbs[0];       // Thread 0 will run first
}

// *********** setInitialStack **************
// Initializes the stack for easy debugging
// Inputs: Index of tcbs array to initialize 
// Outputs: None
void setInitialStack(int i) {
  tcbs[i].sp = &tcbs[i].stack[STACKSIZE-16]; // thread stack pointer
  tcbs[i].stack[STACKSIZE-1] = 0x01000000;   // thumb bit
  tcbs[i].stack[STACKSIZE-3] = 0x14141414;   // R14
  tcbs[i].stack[STACKSIZE-4] = 0x12121212;   // R12
  tcbs[i].stack[STACKSIZE-5] = 0x03030303;   // R3
  tcbs[i].stack[STACKSIZE-6] = 0x02020202;   // R2
  tcbs[i].stack[STACKSIZE-7] = 0x01010101;   // R1
  tcbs[i].stack[STACKSIZE-8] = 0x00000000;   // R0
  tcbs[i].stack[STACKSIZE-9] = 0x11111111;   // R11
  tcbs[i].stack[STACKSIZE-10] = 0x10101010;  // R10
  tcbs[i].stack[STACKSIZE-11] = 0x09090909;  // R9
  tcbs[i].stack[STACKSIZE-12] = 0x08080808;  // R8
  tcbs[i].stack[STACKSIZE-13] = 0x07070707;  // R7
  tcbs[i].stack[STACKSIZE-14] = 0x06060606;  // R6
  tcbs[i].stack[STACKSIZE-15] = 0x05050505;  // R5
  tcbs[i].stack[STACKSIZE-16] = 0x04040404;  // R4
}

//******** OS_AddThread *************** 
// add a foregound thread to the scheduler
// Inputs: pointer to a void/void foreground task
//         number of bytes allocated for its stack
// Outputs: 1 if successful, 0 if this thread can not be added
// stack size must be divisable by 8 (aligned to double word boundary)
// In Lab 2, you can ignore both the stackSize and priority fields
// In Lab 3, you can ignore the stackSize fields

int OS_AddThread(void(*task)(void), uint32_t stackSize) {
   
  long status;
  int i;
  int index;

  status = StartCritical();

  if(NumThreads == 0) {
    // First thread no TCBs yet
    tcbs[0].next = &tcbs[0];
	  tcbs[0].prev = &tcbs[0];
	  Head = &tcbs[0];
	  Tail = &tcbs[0];
	  index = 0;
  } else {
    
	// Look for open spot in tcbs array
	for(i = 0; i < MAXTHREADS; i++) {
	  if(tcbs[i].valid == INVALID) {
	    index = i;
		  i = MAXTHREADS; // Exit loop
	  } else {
	    index = -1;  // Sentinel to detect no invalid spots
	  }
	}

	if(index == -1) {
	  EndCritical(status);
	  return FAILURE; // No space in tcbs
	}

	tcbs[index].next = Head; // New tcb points to head
	tcbs[index].prev = Tail; // Point back to current tail
	(*Tail).next = &tcbs[index]; // Tail now points to new tcb
	Tail = &tcbs[index]; // New tcb becomes the tail
	(*Head).prev = &tcbs[index]; // Head now points backwards to new tcb
  }


  // Initilizing the stack for debugging
  setInitialStack(index);

  // Set PC for stack to point to function to run
  tcbs[index].stack[STACKSIZE-2] = (uint32_t )(task);

  // Set inital values for sleep status and id
  tcbs[index].sleepState = 0;
  tcbs[index].id = index;
  tcbs[index].valid = VALID;
  NumThreads++;
  
  EndCritical(status);

  return SUCCESS;
}

// ******* OS_Launch *********************
// start the scheduler, enable interrupts
// Inputs: number of 20ns clock cycles for each time slice
//        ( Maximum of 24 bits)
// Outputs: none (does not return)
void OS_Launch( ){

  StartOS(); // Assembly language function that initilizes stack for running
 
  while(1) { }
}

