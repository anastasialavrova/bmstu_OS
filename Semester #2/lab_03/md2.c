#include <linux/init.h>
#include <linux/module.h>
#include "md.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lavrova");
MODULE_DESCRIPTION("Lab_3");

static int __init my_module_init(void)
{
  printk(KERN_INFO "md2: Старт\n");
  printk(KERN_INFO "md2: Число экспортированное из md1: %d\n", md1_int_data);
  printk(KERN_INFO "md2: Строка экспортированная из md1: %s\n", md1_str_data);
  printk(KERN_INFO "md2: Результат работы функции md1_get_str(0): %s\n", md1_get_str(0));
  printk(KERN_INFO "md2: Результат работы функции md1_get_str(1): %s\n", md1_get_str(1));
  printk(KERN_INFO "md2: Результат работы функции md1_get_str(2): %s\n", md1_get_str(2));
  printk(KERN_INFO "md2: Результат работы функции md1_factorial(5): %d\n", md1_factorial(5));
  return 0;
}

static void __exit my_module_exit(void)
{
  printk(KERN_INFO "md2: unloaded\n");
}

module_init(my_module_init);
module_exit(my_module_exit);
