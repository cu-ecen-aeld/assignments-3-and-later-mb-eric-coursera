/*
  The queue.h based implementation of linked list of threads
  by Mark Bartish
 */

#ifndef _THREAD_SLIST_H_
#define _THREAD_SLIST_H_
#include "queue.h"


typedef int thr_handle_t;

typedef struct node
{
    thr_handle_t      thr_handle;
    SLIST_ENTRY(node) nodes;
} node_t;

// This typedef creates a head_t that makes it easy for us to pass pointers to
// head_t without the compiler complaining.
typedef SLIST_HEAD(head_s, node) head_t;



void thread_slist_init();

void thread_slist_destroy();

void thread_slist_add(thr_handle_t thr_handle);

void thread_slist_remove(thr_handle_t thr_handle);

#endif //_THREAD_SLIST_H_
