#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/sched/signal.h>
#include<linux/moduleparam.h>
#include<linux/timer.h>
#include<linux/delay.h>
#include<linux/slab.h>
#include<linux/jiffies.h>
#include<linux/list.h>


struct timer_list my_timer_list;


MODULE_LICENSE("GPL");

MODULE_AUTHOR("WCZ");

MODULE_DESCRIPTION("Lab3_2 test code/n");

static int func = 0;
module_param(func, int, 0644);
static int pid = -1;
module_param(pid, int, 0644);


void timer_function(struct timer_list *t)
{
	struct task_struct* task;
	int counter = 0;
	for_each_process(task)
	{
		if(task->mm == NULL)
			counter ++;
	}
	printk(KERN_ALERT"my number of kernel process is %d",counter);
	mod_timer(&my_timer_list,jiffies + 5 * HZ);
	return ;
}




static int __init lab3_2_init(void)
{
	
	printk(KERN_ALERT"my_module_init() begin\n");
	
	if(func == 1)
	{
		struct task_struct* task;
		printk(KERN_ALERT"PID\tSTATE\tCOMMAND\n");
		for_each_process(task)
		{
			if(task->mm == NULL)
				printk("%d\t%ld\t%s\n",task->pid, task->state, task->comm);
		}
	}

	if(func == 2)
	{		

		timer_setup(&my_timer_list, timer_function, 0);
		my_timer_list.expires = jiffies + 5 * HZ;
		add_timer(&my_timer_list);
		
	}
	if(func == 3)
	{
		struct task_struct *t;
		struct task_struct *task = NULL;
		//struct list_head *children;
		//struct list_head *sibling;


		printk(KERN_ALERT"pid receive successfully:%d\n", pid);
		for_each_process(t)
		{
			if(t->pid == pid)
			{
				task = t;
			}
		}
		if(task == NULL)
			printk("pid failed\n");
		else
		{
			if(/*task->parent != NULL*/1)
				printk(KERN_ALERT"His parent is : pid=%d,state=%ld,comm=%s\n",task->parent->pid,task->parent->state,task->parent->comm);

			if(1)
			{
				struct list_head *temp;
				struct task_struct *t_comp = list_entry(task->children.prev, struct task_struct, sibling);

				for(temp = task->children.next; ;temp = temp->next)
				{
					struct task_struct *t1 = list_entry(temp, struct task_struct, sibling);
					

					printk(KERN_ALERT"His children is : pid=%d,state=%ld,comm=%s\n",t1->pid,t1->state,t1->comm);
					if(t1->pid == t_comp->pid)
						break;
				}
			}
			if(1)
			{
				struct list_head *temp;
				for(temp = task->parent->children.next; ;temp = temp->next)
				{
					struct task_struct *t1 = list_entry(temp, struct task_struct, sibling);
					struct task_struct *t_comp = list_entry(task->parent->children.prev, struct task_struct, sibling);

					printk(KERN_ALERT"His siblings is : pid=%d,state=%ld,comm=%s\n",t1->pid,t1->state,t1->comm);
					if(t1->pid == t_comp->pid)
						break;
				}
			}
			if(1)
			{
				struct task_struct *temp = task;
				printk(KERN_ALERT"His thread is : pid=%d,state=%ld,comm=%s\n",temp->pid,temp->state,temp->comm);
				while_each_thread(task, temp)
				{
					printk(KERN_ALERT"His thread is : pid=%d,state=%ld,comm=%s\n",temp->pid,temp->state,temp->comm);
				}
			}
		}
		
	}
	
	return 0;
}



static void __exit lab3_2_exit(void)
{
	if(func == 2)
	{
		del_timer(&my_timer_list);
	}	
	printk(KERN_ALERT" module has exited!\n");
}

// 模块初始化宏，用于加载该模块
module_init(lab3_2_init);
// 模块卸载宏，用于卸载该模块
module_exit(lab3_2_exit);

