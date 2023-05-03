#ifndef __MOUSE
#define __MOUSE

#include <minix/sysutil.h>
#include "i8042.h"
#include <lcom/lcf.h>
#include <stdint.h>
#include <stdio.h>


int (mouse_config)(uint8_t control_word);
int (mouse_subscribe_int)(uint8_t *bit_no);
void (mouse_ih)();
int (mouse_unsubscribe_int)();
int (mouse_sync)();
int (mouse_process_packet)();


#endif
