#ifndef TIMER_H
#define TIMER_H

void timer_initialize(unsigned int frequency);

void timer_interrupt_handler(void);

unsigned int timer_get_ticks(void);

#endif