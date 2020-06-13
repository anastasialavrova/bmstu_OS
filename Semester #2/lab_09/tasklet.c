#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/time.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lavrova");
MODULE_DESCRIPTION("lab_9");

#define SHARED_IRQ 1

static int my_dev_id;
char tasklet_data[] = "tasklet was called";

void tasklet_function(unsigned long data);

DECLARE_TASKLET(my_tasklet, tasklet_function, (unsigned long) &tasklet_data);

void tasklet_function(unsigned long data) 
{
    printk(KERN_INFO "Tasklet: state - %ld, count - %d, data - %s\n",
        my_tasklet.state,
        my_tasklet.count,
        my_tasklet.data);
}

// Обработчик прерывания
static irqreturn_t my_interrupt(int irq, void *dev_id) 
{
    // Проверка, что произошло нужное прерывание
    if (irq = SHARED_IRQ)
    {
        printk(KERN_INFO "Tasklet scheduled\n");
        // Постановка тасклета в очередь на исполнение
        tasklet_schedule(&my_tasklet);
    // Прерывание обработано
	return IRQ_HANDLED;
    }
    else
        // Прерывание не обработано
        return IRQ_NONE;
}

// Инициализация модуля
static int __init my_tasklet_init(void) 
{
    // Разделение линии IRQ с другими устройствами
    int ret = request_irq(SHARED_IRQ, my_interrupt, IRQF_SHARED, "my_interrupt", &my_dev_id);
    if (ret)
    {
	printk(KERN_INFO "Error on request_irq\n");
	return -1;
    }
    printk(KERN_INFO "Module loaded\n");
    return 0;
}

// Выход загружаемого модуля
static void __exit my_tasklet_exit(void) 
{
    // Удаление тасклета
    tasklet_kill(&my_tasklet);
    // Освобождение линии прерывания
    free_irq(SHARED_IRQ, &my_dev_id);
    printk(KERN_INFO "Module unloaded\n");
}

module_init(my_tasklet_init);
module_exit(my_tasklet_exit);
