#ifndef INTERRUPTS_H
#define INTERRUPTS_H

void interrupts_initialize(void);
void interrupts_enable(void);
void interrupts_disable(void);

void interrupts_send_eoi(unsigned char irq);

#endif