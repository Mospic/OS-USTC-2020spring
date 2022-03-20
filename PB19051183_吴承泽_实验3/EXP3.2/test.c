#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/err.h>

#include <linux/types.h>
#include <linux/freezer.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pid.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/string.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <linux/signal.h>
#include <linux/moduleparam.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/rmap.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("OS2021");
MODULE_DESCRIPTION("SYSFS_TEST!");
MODULE_VERSION("1.0");

//sysfs
#define SYSFS_TEST_RUN_STOP 0
#define SYSFS_TEST_RUN_START 1

#define PAGE_SIZE_4K 4096

// /sys/kerbel/mm/sysfs_test/cycle
static unsigned int cycle = 0;
// /sys/kerbel/mm/sysfs_test/pid
static unsigned int pid = 0;		
// /sys/kerbel/mm/sysfs_test/func
static unsigned int sysfs_test_func = 0;
//  /sys/kernel/mm/sysfs_test/sysfs_test_run
static unsigned int sysfs_test_run = SYSFS_TEST_RUN_STOP;
//  /sys/kernel/mm/sysfs_test/sleep_millisecs
static unsigned int sysfs_test_thread_sleep_millisecs = 5000;

static struct task_struct* sysfs_test_thread;

static DECLARE_WAIT_QUEUE_HEAD(sysfs_test_thread_wait);

static DEFINE_MUTEX(sysfs_test_thread_mutex);


static int sysfs_testd_should_run(void)
{
    return (sysfs_test_run & SYSFS_TEST_RUN_START);
}

int vma_count = 0;
int file = 0;
int active_file = 0;
int anon = 0;
int active_anon = 0;

char string_pid[10];

static struct proc_dir_entry* proc_pid;
static struct proc_dir_entry* proc_kmscan;

static int test_show(struct seq_file* m, void* v)
{
    int* vma_count_n = (int*)m->private;
    if (vma_count_n != NULL)
    {
        seq_printf(m, "%d\n", *vma_count_n);
    }
    return 0;
}

static int test_open(struct inode* inode, struct file* file)
{
    return single_open(file, test_show, PDE_DATA(inode));
}

struct proc_ops test_ops = {
    .proc_open = test_open,
    .proc_read = seq_read,
    .proc_release = single_release,
};

typedef typeof(follow_page)* my_follow_page;



typedef typeof(page_referenced)* my_page_referenced;



static void get_vma(void)
{
    struct task_struct* task_temp;
    struct task_struct* task = NULL;
    struct vm_area_struct *vma = NULL;
    
/*
    sprintf(string_pid, "%u", pid);
    proc_pid = proc_mkdir(string_pid, proc_kmscan);
    if (proc_pid == NULL) {
        printk("%s proc create %s failed\n", __func__, "pid");
        return ;
    }
*/
    
    vma_count = 0;

    for_each_process(task_temp)
    {
	if(task_temp->pid == pid)
	    task = task_temp;
    }
    if(task == NULL)
    {
	printk("This PID dosen't exist!\n");
	return ;
    }
    if(task->mm && task->mm->mmap)
    {
        for(vma = task->mm->mmap; vma ; vma = vma->vm_next)
	{
	    vma_count++;
	}
    }

    proc_remove(proc_pid);
    sprintf(string_pid, "%u", pid);
    proc_pid = proc_mkdir(string_pid, proc_kmscan);
    if (proc_pid == NULL) {
        printk("%s proc create %s failed\n", __func__, "pid");
        return ;
    }
    proc_create_data("vma_count", 0664, proc_pid, &test_ops, &vma_count);

    return ;
}


static void get_page_status(void)
{
    struct task_struct* task_temp;
    struct task_struct* task = NULL;
    struct vm_area_struct *vma = NULL;
    struct page *Page = NULL;
    unsigned long address = 0;

    my_follow_page mfollow_page;
    my_page_referenced mpage_referenced;
    mfollow_page = (my_follow_page)0xffffffff97c73af0;
    mpage_referenced = (my_page_referenced)0xffffffff97c8e030;

    file = 0;
    active_file = 0;
    anon = 0;
    active_anon = 0;


    for_each_process(task_temp)
    {
	if(task_temp->pid == pid)
	    task = task_temp;
    }
    if(task == NULL)
    {
	printk("This PID dosen't exist!\n");
	return ;
    }

    vma = task->mm->mmap;
    while(vma)
    {
	address = vma->vm_start;
	while(address != vma->vm_end)
	{
	    Page = mfollow_page(vma, address, FOLL_GET);
	    if(Page)
	    {
		file ++;
	        if(PageAnon(Page))
		{
		    anon ++;
	            if(mpage_referenced(Page, 0, Page->mem_cgroup, &vma->vm_flags))
                        active_anon++;
	        }
	        if(mpage_referenced(Page, 0, Page->mem_cgroup, &vma->vm_flags))
                    active_file++;
	    }
	address = address + PAGE_SIZE;
	}
    vma = vma->vm_next;
    }
    proc_remove(proc_pid);
    sprintf(string_pid, "%u", pid);
    proc_pid = proc_mkdir(string_pid, proc_kmscan);
    if (proc_pid == NULL) {
        printk("%s proc create %s failed\n", __func__, "pid");
        return ;
    }

    proc_create_data("file", 0664, proc_pid, &test_ops, &file);
    proc_create_data("active_file", 0664, proc_pid, &test_ops, &active_file);
    proc_create_data("anon", 0664, proc_pid, &test_ops, &anon);
    proc_create_data("active_anon", 0664, proc_pid, &test_ops, &active_anon);
	printk(KREN_ALERT"123123123123\n");
    
    return ;
}


static void sysfs_test_to_do(void)
{
    if (sysfs_test_func == 1)
        get_vma();
    else if (sysfs_test_func == 2)
        get_page_status();
}

static int sysfs_testd_thread(void* nothing)
{
    set_freezable();
    set_user_nice(current, 5);
    while (!kthread_should_stop())
    {
        mutex_lock(&sysfs_test_thread_mutex);
        if (sysfs_testd_should_run())
            sysfs_test_to_do();
        mutex_unlock(&sysfs_test_thread_mutex);
        try_to_freeze();
        if (sysfs_testd_should_run())
        {
            schedule_timeout_interruptible(
                msecs_to_jiffies(sysfs_test_thread_sleep_millisecs));
        }
        else
        {
            wait_event_freezable(sysfs_test_thread_wait,
                sysfs_testd_should_run() || kthread_should_stop());
        }
    }
    return 0;
}


#ifdef CONFIG_SYSFS

/*
 * This all compiles without CONFIG_SYSFS, but is a waste of space.
 */

#define SYSFS_TEST_ATTR_RO(_name) \
        static struct kobj_attribute _name##_attr = __ATTR_RO(_name)

#define SYSFS_TEST_ATTR(_name)                         \
        static struct kobj_attribute _name##_attr = \
                __ATTR(_name, 0644, _name##_show, _name##_store)

static ssize_t sleep_millisecs_show(struct kobject* kobj,
    struct kobj_attribute* attr, char* buf)
{
    return sprintf(buf, "%u\n", sysfs_test_thread_sleep_millisecs);
}

static ssize_t sleep_millisecs_store(struct kobject* kobj,
    struct kobj_attribute* attr,
    const char* buf, size_t count)
{
    unsigned long msecs;
    int err;

    err = kstrtoul(buf, 10, &msecs);
    if (err || msecs > UINT_MAX)
        return -EINVAL;

    sysfs_test_thread_sleep_millisecs = msecs;

    return count;
}
SYSFS_TEST_ATTR(sleep_millisecs);

static ssize_t cycle_show(struct kobject* kobj,
    struct kobj_attribute* attr, char* buf)
{
    return sprintf(buf, "%u\n", cycle);
}

static ssize_t cycle_store(struct kobject* kobj,
    struct kobj_attribute* attr,
    const char* buf, size_t count)
{
    unsigned long tmp;
    int err;

    err = kstrtoul(buf, 10, &tmp);
    if (err || tmp > UINT_MAX)
        return -EINVAL;

    cycle = tmp;

    return count;
}
SYSFS_TEST_ATTR(cycle);

static ssize_t pid_show(struct kobject* kobj,		//pid
    struct kobj_attribute* attr, char* buf)
{
    return sprintf(buf, "%u\n", pid);
}

static ssize_t pid_store(struct kobject* kobj,
    struct kobj_attribute* attr,
    const char* buf, size_t count)
{
    unsigned long tmp;
    int err;

    err = kstrtoul(buf, 10, &tmp);
    if (err || tmp > UINT_MAX)
        return -EINVAL;

    pid = tmp;

    return count;
}
SYSFS_TEST_ATTR(pid);


static ssize_t func_show(struct kobject* kobj,
    struct kobj_attribute* attr, char* buf)
{
    return sprintf(buf, "%u\n", sysfs_test_func);
}

static ssize_t func_store(struct kobject* kobj,
    struct kobj_attribute* attr,
    const char* buf, size_t count)
{
    unsigned long tmp;
    int err;

    err = kstrtoul(buf, 10, &tmp);
    if (err || tmp > UINT_MAX)
        return -EINVAL;

    sysfs_test_func = tmp;

    return count;
}
SYSFS_TEST_ATTR(func);

static ssize_t run_show(struct kobject* kobj, struct kobj_attribute* attr,
    char* buf)
{
    return sprintf(buf, "%u\n", sysfs_test_run);
}

static ssize_t run_store(struct kobject* kobj, struct kobj_attribute* attr,
    const char* buf, size_t count)
{
    int err;
    unsigned long flags;
    err = kstrtoul(buf, 10, &flags);
    if (err || flags > UINT_MAX)
        return -EINVAL;
    if (flags > SYSFS_TEST_RUN_START)
        return -EINVAL;
    mutex_lock(&sysfs_test_thread_mutex);
    if (sysfs_test_run != flags)
    {
        sysfs_test_run = flags;
    }
    mutex_unlock(&sysfs_test_thread_mutex);

    if (flags & SYSFS_TEST_RUN_START)
        wake_up_interruptible(&sysfs_test_thread_wait);
    return count;
}
SYSFS_TEST_ATTR(run);



static struct attribute* sysfs_test_attrs[] = {
    // 扫描进程的扫描间隔 默认为20秒 
    &sleep_millisecs_attr.attr,
    &cycle_attr.attr,
    &pid_attr.attr,
    &func_attr.attr,
    &run_attr.attr,
    NULL,
};


static struct attribute_group sysfs_test_attr_group = {
    .attrs = sysfs_test_attrs,
    .name = "kmscan",
};
#endif /* CONFIG_SYSFS */

/*--------------------------------------------------------*/




// linux5.9.0
// 低版本需要查看proc_fs.h文件定义的结构体




static int sysfs_test_init(void)
{
    int err;
    sysfs_test_thread = kthread_run(sysfs_testd_thread, NULL, "kmscan");
    if (IS_ERR(sysfs_test_thread))
    {
        pr_err("sysfs_test: creating kthread failed\n");
        err = PTR_ERR(sysfs_test_thread);
        goto out;
    }

#ifdef CONFIG_SYSFS
    err = sysfs_create_group(mm_kobj, &sysfs_test_attr_group);
    if (err)
    {
        pr_err("sysfs_test: register sysfs failed\n");
        kthread_stop(sysfs_test_thread);
        goto out;
    }
#else
    sysfs_test_run = KSCAN_RUN_STOP;
#endif  /* CONFIG_SYSFS */

   // 在 proc 根目录创建 kmscan 文件夹
    
    
    proc_kmscan = proc_mkdir("kmscan", NULL);
    if (proc_kmscan == NULL) {
        printk("%s proc create %s failed\n", __func__, "kmscan");
        return -EINVAL;
    }

    sprintf(string_pid, "%u", pid);
    proc_pid = proc_mkdir("pid", proc_kmscan);
    if (proc_pid == NULL) {
        printk("%s proc create %s failed\n", __func__, "pid");
        return -EINVAL;
    }

    return 0;

out:
    return err;
 
}

static void sysfs_test_exit(void)
{
    if (sysfs_test_thread)
    {
        kthread_stop(sysfs_test_thread);
        sysfs_test_thread = NULL;
    }


#ifdef CONFIG_SYSFS

    sysfs_remove_group(mm_kobj, &sysfs_test_attr_group);

#endif

    //printk("sysfs_test exit success!\n");

    proc_remove(proc_pid);
    proc_remove(proc_kmscan);
    printk(KERN_ALERT "test exit!>>>>>>>>>>>>>>>>>>>>>\n");
}



//module_param(vma_count, int, 0644);




/* --- 随内核启动  ---  */
// subsys_initcall(kscan_init);
module_init(sysfs_test_init);
module_exit(sysfs_test_exit);
