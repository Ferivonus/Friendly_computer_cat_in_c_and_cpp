#include "LinkedList.h"

LinkedList* createLinkedList() {
    LinkedList* newList = (LinkedList*)malloc(sizeof(LinkedList));
    if (newList != NULL) {
        newList->head = NULL;
        newList->tail = NULL;
        newList->length = 0;
    }
    return newList;
}

void addLast(LinkedList* list, C_WorkScheduleItem val) {
    if (list == NULL) return;

    struct node* newNode = (struct node*)malloc(sizeof(struct node));
    if (newNode == NULL) {
        printf("Kritik Hata: Bellek (RAM) ayrilamadi!\n");
        return;
    }

    newNode->data = val;
    newNode->next = NULL;

    if (list->head == NULL) {
        list->head = newNode;
        list->tail = newNode;
    }
    else {
        list->tail->next = newNode;
        list->tail = newNode;
    }

    list->length++;
}

void printList(LinkedList* list) {
    if (list == NULL) return;

    struct node* temp = list->head;

    printf("--- Work Schedule List (Toplam Eleman: %d) ---\n", list->length);
    while (temp != NULL) {
        printf("[ID: %d | Title: %s] -> ", temp->data.id, temp->data.work_title);
        temp = temp->next;
    }
    printf("NULL\n--------------------------\n");
}

void freeLinkedList(LinkedList* list) {
    if (list == NULL) return;

    struct node* current = list->head;
    struct node* nextNode;

    while (current != NULL) {
        nextNode = current->next;
        free(current);
        current = nextNode;
    }


    free(list);
}