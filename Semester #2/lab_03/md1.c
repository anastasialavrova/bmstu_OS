#include <linux/init.h>
#include <linux/module.h>
#include "md.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lavrova");
MODULE_DESCRIPTION("Lab_3");

char* md1_str_data = "Привет мир";
int md1_int_data = 64;

extern char* md1_get_str(int n)
{
    printk(KERN_INFO "md1: md1_get_str(%d) called\n", n);
    switch (n)
    {
    case 1:
    	return "Первая строка";
    	break;
    case 2:
        return "Вторая строка";
    	break;
    default:
    	return "1 - первая строка, 2 - вторая строка";
    	break;
  }
}

extern int md1_factorial(int n)
{
  printk(KERN_INFO "md1: md1_factorial(%d) called\n", n);

  int i, answer = 1;

  if (n <= 0)
    return 0;

  for (i = 2; i <= n; i++)
    answer *= i;

  return answer;
}

EXPORT_SYMBOL(md1_str_data);
EXPORT_SYMBOL(md1_int_data);
EXPORT_SYMBOL(md1_get_str);
EXPORT_SYMBOL(md1_factorial);

static int __init my_module_init(void)
{
  printk(KERN_INFO "md1: loaded\n");
  return 0;
}

static void __exit my_module_exit(void)
{
  printk(KERN_INFO "md1: unloaded\n");
}

module_init(my_module_init);
module_exit(my_module_exit);
