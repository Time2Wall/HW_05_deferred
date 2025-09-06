// ex_tasklets.c
// Demonstrates Linux tasklets using the modern API (Linux 6.1+).
// High-priority tasklets were removed; we now only use normal tasklets.

#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/param.h>
#include <linux/smp.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Example: Tasklets (Linux 6.1+ API)");
MODULE_VERSION("1.2");

// Module parameter: how many times to schedule the tasklet
static int times = 3;
module_param(times, int, 0444);
MODULE_PARM_DESC(times, "How many times to schedule the tasklet on module load");

// Tasklet callback: runs in softirq context, must not sleep.
static void demo_tasklet_fn(struct tasklet_struct *t)
{
    pr_info("ex_tasklets: tasklet executed on CPU %d\n",
        raw_smp_processor_id());
}

// Declare a single normal-priority tasklet
static DECLARE_TASKLET(demo_tasklet, demo_tasklet_fn);

static int __init ex_tasklets_init(void)
{
    int i;

    pr_info("ex_tasklets: init, scheduling tasklet %d time(s)\n", times);

    for (i = 0; i < times; i++) {
        tasklet_schedule(&demo_tasklet);
        pr_info("ex_tasklets: scheduled tasklet (%d/%d)\n", i + 1, times);
        msleep(50);
    }

    return 0;
}

static void __exit ex_tasklets_exit(void)
{
    // Kill tasklet if it's scheduled or running
    tasklet_kill(&demo_tasklet);
    pr_info("ex_tasklets: exit (tasklet killed)\n");
}

module_init(ex_tasklets_init);
module_exit(ex_tasklets_exit);
