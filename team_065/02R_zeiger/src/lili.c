#include <stdlib.h>
#include <stdio.h>
#include "lili.h"

element_t *head = NULL;
element_t *letzt = NULL;//global point to last element
//implement necessary functions here
unsigned int* insert_element(unsigned int value)
{
    element_t *now=NULL;//defined as a point of inserted new Element at the Moment
    if(head==NULL)
    {
        head=(element_t*)malloc(sizeof(element_t));//insert 1.Element
        if(head!=NULL)
        {
            head->data=value;
            head->next=NULL;
            letzt=head;//first point also as address of a last empty element relative to 1.Element
            return (unsigned int*)&head->data;
        }
        else
        {
            perror("Error: ");
            head=NULL;
            return 0;
        }
    }
    else
    {
        now=(element_t*)malloc(sizeof(element_t));//insert from 2.Elements
                if(now!=NULL)
                {
                    now->data=value;//give value to data of new element
                    now->next=NULL;//from the Tail of List a new element insert,so the last next point to Null

                    letzt->next=now;//connect the address of now element and last next
                    letzt=now;//renew the global "letzt" in order to call letzt->next at next time
                    return (unsigned int*)&now->data;
                }
                else
                {
                    perror("Error: ");
                    now=NULL;
                    return 0;
                }

    }
}

void print_lili(void)
{
    element_t *temp=NULL;//insert a temporary point in oder to print,temp must repeatedly change
    temp=head;
    printf("printf_lili:");
    while(temp!=NULL)
    {
      printf("%d,",temp->data);
      temp=temp->next;
    }
    printf("\n");
}

unsigned int remove_element(void)
{
    element_t *alt=NULL;
    unsigned int wert=0;
    alt=head; //insert a temporary point in oder to remove element
    if(head==NULL)
    {
        return 0;
    }
    else
    {
        wert=alt->data;//return data of first elemnt
        head=head->next;//renew head point
        alt->next=NULL;
        free(alt);//release memory of first element
        return wert;
    }
}
