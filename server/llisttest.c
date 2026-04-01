#include "queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct node
{
    char              c;
    SLIST_ENTRY(node) nodes;
} node_t;

// This typedef creates a head_t that makes it easy for us to pass pointers to
// head_t without the compiler complaining.
typedef SLIST_HEAD(head_s, node) head_t;

static void fill_slist (head_t *head, const char *string);
static void free_slist (head_t *head);
static void print_slist(head_t *head);


int main(int argc, char **argv){
    printf("linkted list test\n");

    //declare the head

    head_t head;
    SLIST_INIT( &head);

    fill_slist( &head, "Head aega\n");

    print_slist(&head);
    
    free_slist( &head);
    
    return 0;
}

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

static void free_slist(head_t *head)
{
    struct node * e = NULL;
    while (!SLIST_EMPTY(head))
    {
        e = SLIST_FIRST(head);
        SLIST_REMOVE_HEAD(head, nodes);
        free(e);
        e = NULL;
    }
}


static void print_slist(head_t *head)
{
    struct node *e = NULL;
    SLIST_FOREACH(e, head, nodes)
    {
        printf("%c", e->c);
    }
}
