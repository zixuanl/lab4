/**
 * @file device.c
 *
 * @brief Implements simulated devices.
 * @author Kartik Subramanian <ksubrama@andrew.cmu.edu>
 * @date 2008-12-01
 */

#include <types.h>
#include <assert.h>

#include <task.h>
#include <sched.h>
#include <device.h>
#include <systime.h>
#include <arm/reg.h>
#include <arm/psr.h>
#include <arm/exception.h>

/**
 * @brief Fake device maintainence structure.
 * Since our tasks are periodic, we can represent 
 * tasks with logical devices. 
 * These logical devices should be signalled periodically 
 * so that you can instantiate a new job every time period.
 * Devices are signaled by calling dev_update 
 * on every timer interrupt. In dev_update check if it is 
 * time to create a tasks new job. If so, make the task runnable.
 * There is a wait queue for every device which contains the tcbs of
 * all tasks waiting on the device event to occur.
 */

struct dev
{
	tcb_t* sleep_queue;
	unsigned long   next_match;
};
typedef struct dev dev_t;

/* devices will be periodically signaled at the following frequencies */
const unsigned long dev_freq[NUM_DEVICES] = {100, 200, 500, 50};
static dev_t devices[NUM_DEVICES];

/**
 * @brief Initialize the sleep queues and match values for all devices.
 */
void dev_init(void)
{
	int i;
	for (i = 0; i < NUM_DEVICES; i++) {
		devices[i].sleep_queue = (void *)0;
		devices[i].next_match =system_time+dev_freq[i]; 	
	}
}


/**
 * @brief Puts a task to sleep on the sleep queue until the next
 * event is signalled for the device.
 *
 * @param dev  Devisece number.
 */
void dev_wait(unsigned int dev)
{
	tcb_t* tmp = devices[dev].sleep_queue;
	while (1) {
		if(tmp == (void *)0) {
			tmp = runqueue_remove(get_cur_prio());
			break;
		}
		tmp = tmp->sleep_queue;
	}
	dispatch_sleep();	
}


/**
 * @brief Signals the occurrence of an event on all applicable devices. 
 * This function should be called on timer interrupts to determine that 
 * the interrupt corresponds to the event frequency of a device. If the 
 * interrupt corresponded to the interrupt frequency of a device, this 
 * function should ensure that the task is made ready to run 
 */
void dev_update(unsigned long millis)
{	
	int i;
	for (i = 0; i < NUM_DEVICES; i++) {
		if( millis == devices[i].next_match) {
			devices[i].next_match = devices[i].next_match + dev_freq[i];
			runqueue_add(devices[i].sleep_queue, 0);
		}
	}
	dispatch_save();	
}

