#include <rt_misc.h>

 __value_in_regs struct __initial_stackheap __user_initial_stackheap( 
 unsigned R0, unsigned SP, unsigned R2, unsigned SL){    
 
 struct __initial_stackheap config;     

 //config.heap_limit  = 0x00724B40;     

 //config.stack_limit = 0x00004000;
 //config.stack_limit = 0x00100000;    

/* works */
config.heap_base   = 0x00260000;     
config.stack_base  = 0x00460000;    


/* factory defaults */
 //config.heap_base   = 0x00060000;     
 //config.stack_base  = 0x00080000;    
 return config;} 