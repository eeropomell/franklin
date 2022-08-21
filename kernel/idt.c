#include "limine.h"
#include "franklin/defs.h"
#include "franklin/acpi.h"
#include "franklin/idt.h"
#include "franklin/69.h"
#include "franklin/io.h"
#include "franklin/spinlock.h"
#include "franklin/interrupt.h"




__attribute__((aligned(0x10)))
static idt_entry idt[256];

static struct {
    unsigned short size;
    unsigned long base;
} __attribute__((packed)) idtr;


void load_idt() {
    asm("lidt %0" :: "m" (idtr));
    asm("sti");
}

void init_idt() {
    idtr.base = (uintptr_t)&idt[0];
    idtr.size = (uint16_t)sizeof(idt_entry) * IDT_MAX_DESC - 1;

    for (uint8_t vector = 0; vector < 32; vector++) {
        set_idt_entry(vector, isr_table[vector], 0x8E);
    }

    load_idt();
}


void new_irq(unsigned char vector, void(*isr)(void)) {
  set_idt_entry(vector, isr, 0x8E);
}

void set_idt_entry(unsigned char vector, void(*isr)(), unsigned char flags) {

    idt_entry *desc = &idt[vector];
    desc->isr_low = (unsigned long)isr & 0xFFFF;
    desc->selector = (0x5 << 3);
    desc->ist = 0;
    desc->attributes = flags;
    desc->isr_mid = ((unsigned long)isr >> 16) & 0xFFFF;
    desc->isr_high = ((unsigned long)isr >> 32) & 0xFFFFFFFF;
    desc->zero = 0;
}

extern void apic_timer(void);
extern void kbd_press(void);


void trap(regs_t *regs) {
  if (regs->code < 32) {
    char s[20];
    itoa(regs->code, s);
    print("trap error: ");
    print(s);
    asm("cli; hlt");
  }


  switch(regs->code) {
  case 32:
    timerh(10);
    break;
  case 33:
    kbd_press();
    break;
  case 34:
    apic_timer();
    break;
  }


};
