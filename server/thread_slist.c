#include "thread_slist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>




static void free_slist ();
//static void print_slist(head_t *head);



static  head_t head = { 0 };
static  bool   slist_inited = false;


/*
int main(int argc, char **argv){
    printf("linkted list test\n");

    //declare the head

    SLIST_INIT( &head);

    fill_slist( &head, "Head aega\n");

    print_slist(&head);
    
    free_slist( &head);
    
    return 0;
}
*/

/*
static void fill_slist(head_t *head, const char *string)
{
    size_t slen = strlen(string);
    struct node *last = NULL;
    for (size_t i = 0; i < slen; i++)
    {
        struct node * e = malloc(sizeof(struct node));
        if (e == NULL)
        {
            fprintf(stderr, "malloc failed");
            exit(EXIT_FAILURE);
        }
        e->c = string[i];
        if (last == NULL){
            SLIST_INSERT_HEAD(head, e, nodes);
            last = e;
        }
        else{
            SLIST_INSERT_AFTER(last, e, nodes);
            last = e;
        }
        e = NULL;
    }
}
*/

static void free_slist()
{
    struct node * e = NULL;
    while (!SLIST_EMPTY(&head)){
        e = SLIST_FIRST(&head);
        SLIST_REMOVE_HEAD(&head, nodes);
        free(e);
        e = NULL;
    }
}


void thread_slist_init(){
    if (slist_inited == false){
        SLIST_INIT(&head);
    }
    slist_inited = true;
}

void thread_slist_destroy(){
    free_slist();
    slist_inited = false;
}

void thread_slist_add(thr_handle_t thr_handle){
    
}

void thread_slist_remove(thr_handle_t thr_handle){
    //SLIST_INSERT_AFTER(n1, n2, entries);
    //SLIST_REMOVE(&head, n2, entry, entries);   // Deletion
    //free(n2);
    //n3 = SLIST_FIRST(&head);
    //SLIST_REMOVE_HEAD(&head, entries);         // Deletion from the head
    //free(n3);
}
    
