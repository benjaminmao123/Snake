/*
 * queue.h
 *
 * Created: 3/8/2018 8:49:24 AM
 *  Author: BB
 */ 


#ifndef QUEUE_H_
#define QUEUE_H_

// C program for array implementation of queue
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

struct Node
{
	int x;
	int y;
	struct Node *next;
};

struct Queue
{
	struct Node *front;
	struct Node *back;
};

struct Node * NewNode(int x, int y)
{
	struct Node *temp = (struct Node*)malloc(sizeof(struct Node));
	temp->x = x;
	temp->y = y;
	temp->next = NULL;
	
	return temp;
}

struct Queue * CreateQueue()
{
	struct Queue *q = (struct Queue*)malloc(sizeof(struct Queue));
	q->front = q->back = NULL;
	
	return q;
}

void Push(struct Queue *q, int x, int y)
{
	struct Node *temp = NewNode(x, y);
	
	if (q->back == NULL)
	{
		q->front = q->back = temp;
		return;
	}
	
	q->back->next = temp;
	q->back = temp;
}

struct Node * Pop(struct Queue *q)
{
	if (q->front == NULL)
	{
		return NULL;
	}
	
	struct Node *temp = q->front;
	q->front = q->front->next;
	
	if (q->front == NULL)
	{
		q->back = NULL;
	}
	
	return temp;
}

int Size(struct Queue *q)
{
	struct Node *temp = q->front;
	int count = 0;
	
	while (temp != NULL)
	{
		++count;
		temp = temp->next;	
	}
	
	return count;
}

#endif /* QUEUE_H_ */