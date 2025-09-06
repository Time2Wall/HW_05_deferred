// ex_workqueue.c
// Demonstrates workqueues: using the system workqueue, a dedicated workqueue,
// and delayed work.

#include <linux/init.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/param.h>
#include <linux/smp.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Example: Workqueues (system, dedicated, delayed)");
MODULE_VERSION("1.0");

// Module parameters
static int burst = 3;          // how many work items to queue immediately
static int delay_ms = 200;     // delay for delayed work
module_param(burst,   int, 0444);
module_param(delay_ms, int, 0444);
MODULE_PARM_DESC(burst, "How many work items to queue on load");
MODULE_PARM_DESC(delay_ms, "Delay (ms) for delayed work");

// Work items
static struct work_struct        sys_wq_work;
static struct delayed_work       sys_wq_delayed;
static struct workqueue_struct  *dedicated_wq;
static struct work_struct        dedicated_work;

// System workqueue worker
static void sys_wq_workfn(struct work_struct *work)
{
	pr_info("ex_workqueue: system wq work on CPU %d\n", raw_smp_processor_id());
}

// System delayed workqueue worker
static void sys_wq_delayed_workfn(struct work_struct *work)
{
	pr_info("ex_workqueue: system DELAYED work on CPU %d after %dms\n",
		raw_smp_processor_id(), delay_ms);
}

// Dedicated workqueue worker
static void dedicated_workfn(struct work_struct *work)
{
	pr_info("ex_workqueue: dedicated wq work on CPU %d\n", raw_smp_processor_id());
}

static int __init ex_workqueue_init(void)
{
	int i;

	INIT_WORK(&sys_wq_work, sys_wq_workfn);
	INIT_DELAYED_WORK(&sys_wq_delayed, sys_wq_delayed_workfn);

	// Create a dedicated workqueue. Flags example: WQ_UNBOUND for flexibility.
	dedicated_wq = alloc_workqueue("ex_wq_demo", WQ_UNBOUND | WQ_HIGHPRI, 0);
	if (!dedicated_wq) {
		pr_err("ex_workqueue: failed to create dedicated workqueue\n");
		return -ENOMEM;
	}
	INIT_WORK(&dedicated_work, dedicated_workfn);

	pr_info("ex_workqueue: init, queuing %d system work(s) and one delayed (%dms), plus dedicated work\n",
		burst, delay_ms);

	// Queue a burst of work items to the system workqueue
	for (i = 0; i < burst; i++) {
		schedule_work(&sys_wq_work);
		pr_info("ex_workqueue: queued system work (%d/%d)\n", i + 1, burst);
		msleep(30);
	}

	// Queue delayed work to the system workqueue
	schedule_delayed_work(&sys_wq_delayed, msecs_to_jiffies(delay_ms));
	pr_info("ex_workqueue: queued delayed work (%dms)\n", delay_ms);

	// Queue one work item to the dedicated workqueue
	if (!queue_work(dedicated_wq, &dedicated_work))
		pr_info("ex_workqueue: dedicated work was already queued\n");
	else
		pr_info("ex_workqueue: queued dedicated work\n");

	return 0;
}

static void __exit ex_workqueue_exit(void)
{
	// Cancel and flush delayed/system work
	cancel_delayed_work_sync(&sys_wq_delayed);
	flush_scheduled_work(); // flush global workqueue users from this module

	// Flush and destroy the dedicated workqueue
	if (dedicated_wq) {
		flush_workqueue(dedicated_wq);
		destroy_workqueue(dedicated_wq);
		dedicated_wq = NULL;
	}

	pr_info("ex_workqueue: exit (all work flushed and destroyed)\n");
}

module_init(ex_workqueue_init);
module_exit(ex_workqueue_exit);
