#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include <common.h>

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char expr[64];
  word_t old_val;

  /* TODO: Add more members if necessary */

} WP;

extern WP wp_pool[]; 
extern WP *head;
extern WP *free_;

void init_wp_pool();

WP* new_wp();

void free_wp_no(int NO);

void free_wp(WP *wp);

#endif