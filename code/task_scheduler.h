/*
 * task_scheduler.h
 *
 * Created: 2/20/2018 6:38:33 PM
 *  Author: BB
 */ 

// Permission to copy is granted provided that this header remains intact.
// This software is provided with no warranties.

#ifndef TASK_SCHEDULER_H_
#define TASK_SCHEDULER_H_

unsigned long int gcd(unsigned long int a, unsigned long int b)
{
	unsigned long int c;
	while(1)
	{
		c = a%b;
		if(c==0){return b;}
		a = b;
		b = c;
	}
	return 0;
}

unsigned long int findGCD(int arr[], int n)
{
	int result = arr[0];
	for (int i=1; i<n; i++)
	result = gcd(arr[i], result);
	
	return result;
}

typedef struct _task 
{
	/*Tasks should have members that include: state, period,
		a measurement of elapsed time, and a function pointer.*/
	signed char state; //Task's current state
	unsigned long int period; //Task period
	unsigned long int elapsedTime; //Time elapsed since last task tick
	int (*TickFct)(int); //Task tick function
} task;

#endif /* TASK_SCHEDULER_H_ */