/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "sdb.h"
#include <debug.h>
#include <monitor/watchpoint.h>

#define NR_WP 32

/* 结构体定义放头文件了 */

WP wp_pool[NR_WP] = {};
WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp() {
  if (free_ == NULL) {
    printf("No free watchpoint available.\n");
    assert(0);
  }

  WP* wp = free_;
  free_ = free_->next;
  wp->next = head;
  head = wp;
  return wp;
}

void free_wp_no(int NO) {
  WP *cur = head;
  /* 检测是否合法 */
  while (cur != NULL && cur->NO != NO) {
    cur = cur->next;
  }
  if (cur == NULL) {
    printf("Invalid watchpoint number.\n");
    assert(0);
  }
  free_wp(cur);
}

void free_wp(WP *wp) {
  if (wp == NULL) {
    printf("Invalid watchpoint.\n");
    assert(0);
  }

  if (head == wp) {
    head = wp->next;
    wp->next = free_;
    free_ = wp;
  }
  else {
    WP* cur = head;
    while (cur->next != wp) {
      if (cur->next == NULL) {
        printf("Can't find the watchpoint\n");
        assert(0);
      }
      cur = cur->next;
    }
    cur->next = wp->next;
    wp->next = free_;
    free_ = wp;
  }
}
