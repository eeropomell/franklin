#include "franklin/69.h"
#include "franklin/switch.h"



void switch_to(thread *prev, thread *next) {

  asm volatile("mov %%rsp, %0" : "=r"(prev->rsp));
  asm("mov %0, %%rsp" :: "r"(next->rsp));
};

void swwitch() {
  
};

