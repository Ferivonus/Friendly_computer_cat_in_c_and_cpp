#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "WorkSchedule_Wrapper.h"

#include <stdio.h>
#include <stdlib.h>
struct node {
    C_WorkScheduleItem data;
    struct node* next;
};

typedef struct LinkedList {
    struct node* head; 
    struct node* tail;
    int length;
} LinkedList;


#ifdef __cplusplus
extern "C" {
#endif

    LinkedList* createLinkedList(void);
    void addLast(LinkedList* list, C_WorkScheduleItem val);
    void printList(LinkedList* list);
    void freeLinkedList(LinkedList* list);

#ifdef __cplusplus
}
#endif

#endif // LINKEDLIST_H