
#define IDT_MAX_DESC 256


typedef struct {
    unsigned short isr_low;
    unsigned short selector;
    unsigned char ist;
    unsigned char attributes;
    unsigned short isr_mid;
    unsigned int isr_high;
    unsigned int zero;
} __attribute__((packed)) idt_entry;



typedef struct {
  unsigned long s[10];
  unsigned long code;
} __attribute__((packed))regs_t;


__attribute__((noreturn))
void trap(regs_t*);


extern void *isr_table[];



void init_idt(void);
void load_idt(void);
void new_irq(unsigned char, void(*)(void));
void set_idt_entry(unsigned char, void(*)(void), unsigned char);


