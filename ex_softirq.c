// ex_softirq.c
// Demonstrates deferred execution in SoftIRQ context using irq_work.
// Rationale: Modules cannot register arbitrary softirq vectors, but irq_work
// runs in softirq context (on the same CPU), which is perfect for this demo.

#include <linux/init.h>
#include <linux/module.h>
#include <linux/irq_work.h>
#include <linux/delay.h>
#include <linux/param.h>
#include <linux/smp.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Example: SoftIRQ via irq_work");
MODULE_VERSION("1.0");

// Module parameter: how many times to queue irq_work on load.
static int times = 3;
module_param(times, int, 0444);
MODULE_PARM_DESC(times, "How many irq_work items to queue on module load");

static struct irq_work demo_work;

// irq_work callback runs in softirq context (non-process context).
static void demo_irq_workfn(struct irq_work *work)
{
    // We are in softirq context here.
    pr_info("ex_softirq: irq_work callback on CPU %d (in softirq context)\n",
        raw_smp_processor_id());
    // Do no heavy/slow operations here; keep it atomic-friendly.
}

// Schedule the irq_work 'times' times.
static int __init ex_softirq_init(void)
{
    int i;

    init_irq_work(&demo_work, demo_irq_workfn);

    pr_info("ex_softirq: init, queuing irq_work %d time(s)\n", times);
    for (i = 0; i < times; i++) {
        // Queue work to run in softirq context soon.
        irq_work_queue(&demo_work);
        pr_info("ex_softirq: queued irq_work (%d/%d)\n", i + 1, times);
        // Small sleep to make dmesg ordering clearer (process context here).
        msleep(50);
    }

    // Note: irq_work will be drained automatically by the kernel's softirq handling.
    return 0;
}

static void __exit ex_softirq_exit(void)
{
    // Ensure any pending irq_work is processed before we unload.
    irq_work_sync(&demo_work);
    pr_info("ex_softirq: exit (irq_work synced)\n");
}

module_init(ex_softirq_init);
module_exit(ex_softirq_exit);
