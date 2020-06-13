#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lavrova");
MODULE_DESCRIPTION("lab_9");

#define SHARED_IRQ 1

static int my_dev_id;
static int irq_call_counter = 0;

static struct workqueue_struct *wq;

void my_workqueue_function(struct work_struct *work);

DECLARE_WORK(workqueue_name, my_workqueue_function);

void my_workqueue_function(struct work_struct *work) 
{
    printk(KERN_INFO "Workqueue: working, counter: %d\n", ++irq_call_counter);
}

static irqreturn_t my_interrupt(int irq, void *dev) 
{
    // Проверка, что произошло нужное прерывание
    if (irq = SHARED_IRQ)
    {
	queue_work(wq, &workqueue_name);
        printk(KERN_INFO "Workqueue: starting\n");
    // Прерывание обработано
	return IRQ_HANDLED;
    }
    else
        // Прерывание не обработано
        return IRQ_NONE;
}

// Инициализация модуля
static int __init my_workqueue_init(void) 
{
    // Разделение линии IRQ с другими устройствами
    int ret = request_irq(SHARED_IRQ, my_interrupt, IRQF_SHARED, "my_interrupt", &my_dev_id);
    if (ret) 
    {
        printk(KERN_ERR "Module error: couldn't register handler\n");
        return ret;
    }

    // Создание очереди работ
    wq = create_workqueue("my_workqueue");
    if (!wq) 
    {
        free_irq(SHARED_IRQ, &my_dev_id);
        printk(KERN_INFO "Module error: couldn't create workqueue\n");
        return -ENOMEM;
    }

    printk(KERN_INFO "Module loaded\n");
    return 0;
}

// Выход загружаемого модуля
static void __exit my_workqueue_exit(void) 
{
    // Удаление очереди работ
    flush_workqueue(wq);
    destroy_workqueue(wq);
    // Освобождение линии прерывания
    free_irq(SHARED_IRQ, &my_dev_id);
    printk(KERN_INFO "Module unloaded\n");
}

module_init(my_workqueue_init)
module_exit(my_workqueue_exit)
