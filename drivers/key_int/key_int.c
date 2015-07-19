#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/cdev.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/interrupt.h>

#include <asm/uaccess.h>	/* copy_*_user */
#include <asm/irq.h>
#include <asm/gpio.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>

#define KEY_MAJOR	0
#define DEVICE_NAME	"KEY_INT_DRIVER"

#define MAX_KEY_BUF	16
#define KEY_NUM 4
#define KEY_TIMER_DELAY (20*HZ/1000)

#define KEYSTATUS_INT	1
#define KEYSTATUS_DOWN	2
#define KEYSTATUS_UP	3

#define ISKEY_DOWN(key)	(!s3c2410_gpio_getpin(key_info_tab[key].gpio_port))

typedef unsigned char KEY_RET;

typedef struct _KEY_DEV_{
	unsigned int keyStatus[KEY_NUM];
	KEY_RET buf[MAX_KEY_BUF];
	unsigned int head;
	unsigned int tail;
	wait_queue_head_t wq;
	struct cdev cdev;
}KEY_DEV;

static KEY_DEV keydev;
static struct timer_list key_timer[KEY_NUM];

struct key_info {
	int irq_no;
	unsigned long flags;
	unsigned int gpio_port;
	int key_no;
};
static struct key_info key_info_tab[KEY_NUM] = {
	{IRQ_EINT0, IRQF_TRIGGER_FALLING, S3C2410_GPF0, 1},
	{IRQ_EINT2, IRQF_TRIGGER_FALLING, S3C2410_GPF2, 2},
	{IRQ_EINT3, IRQF_TRIGGER_FALLING, S3C2410_GPF3, 3},
	{IRQ_EINT4, IRQF_TRIGGER_FALLING, S3C2410_GPF4, 4},
};
static irqreturn_t s3c2410_eint_key(int irq, void *dev_id)
{
	int key = (int)dev_id;
	disable_irq(key_info_tab[key].irq_no);

	keydev.keyStatus[key] = KEYSTATUS_INT;
	key_timer[key].expires = jiffies+KEY_TIMER_DELAY;
	add_timer(&key_timer[key]);
	return IRQ_RETVAL(IRQ_HANDLED);
}


static int request_irqs(void)
{
	struct key_info *k;
	int i;
	for (i = 0; i < sizeof(key_info_tab) / sizeof(key_info_tab[0]); i++) {
		k = key_info_tab + i;
		if (request_irq(k->irq_no, s3c2410_eint_key, k->flags, DEVICE_NAME, (void *)i)) {
			return -1;
		}		
	}
	return 0;
}
/*
static void free_irqs(void)
{
	struct key_info *k;
	int i;
	for (i = 0; i < sizeof(key_info_tab) / sizeof(key_info_tab[0]); i++) {
		k = key_info_tab+i;
		free_irq(k->irq_no, s3c2410_eint_key);
	}
}
*/
static void keyEvent(int key_no)
{
	keydev.buf[keydev.tail%MAX_KEY_BUF] = key_no;
	keydev.tail ++;
	keydev.tail = keydev.tail%MAX_KEY_BUF;
	wake_up_interruptible(&(keydev.wq));
	return;
}
static void key_timer_handler(unsigned long data)
{
	int key = data;
	if (ISKEY_DOWN(key)) {
		if (keydev.keyStatus[key] == KEYSTATUS_INT) { //从中断进入
			keydev.keyStatus[key] = KEYSTATUS_DOWN;
			key_timer[key].expires = jiffies+KEY_TIMER_DELAY;
			keyEvent(key_info_tab[key].key_no);	//记录键值，唤醒等待队列
			add_timer(&key_timer[key]);
		} else {
			key_timer[key].expires = jiffies+KEY_TIMER_DELAY;
			add_timer(&key_timer[key]);
		}
	} else {
		keydev.keyStatus[key] = KEYSTATUS_UP;
		enable_irq(key_info_tab[key].irq_no);
	}
}
static int s3c2410_key_open(struct inode *inode, struct file *filp)
{
	int i;
	
	keydev.head = keydev.tail = 0;
//	keyEvent = keyEvent_raw;

	request_irqs(); //注册中断函数
	keydev.head = keydev.tail = 0;
	for (i=0; i<KEY_NUM; i++)
		keydev.keyStatus[i] = KEYSTATUS_UP;
	init_waitqueue_head(&(keydev.wq));

	for (i=0; i<KEY_NUM; i++)
		setup_timer(&key_timer[i], key_timer_handler, i);
	return 0;
}
static int s3c2410_key_release(struct inode *inode, struct file *filp)
{
//	keyEvent = keyEvent_dummy;
	int i;
	
	for (i = 0; i < KEY_NUM; i++) {
		del_timer(&key_timer[i]);
		disable_irq(key_info_tab[i].irq_no);
		free_irq(key_info_tab[i].irq_no, s3c2410_eint_key);
	}
//	free_irqs();
	return 0;
}
static KEY_RET keyRead(void)
{
	KEY_RET key_ret;
	if (keydev.head == keydev.tail) {
		printk(DEVICE_NAME " key buf is empty.\n");
		return -1;
	}
	key_ret = keydev.buf[keydev.head%MAX_KEY_BUF];
	keydev.head ++;
	keydev.head = keydev.head%MAX_KEY_BUF;
	
	return key_ret;
}
static int s3c2410_key_read(struct file *filp, char __user *buf, 
								size_t count, loff_t *ppos)
{
	unsigned long err;
	KEY_RET key_ret;
	
retry:
	if (keydev.head != keydev.tail) {
		key_ret = keyRead();
		err = copy_to_user(buf, &key_ret, sizeof(key_ret));
	} else {
		if (filp->f_flags & O_NONBLOCK) {
			return -EAGAIN;
		}
		interruptible_sleep_on(&(keydev.wq));
		goto retry;
	}
	return err ? -EFAULT : 0;
}

static struct file_operations s3c2410_key_fops = {
	.owner = THIS_MODULE, 
	.open = s3c2410_key_open, 
	.release = s3c2410_key_release,
	.read = s3c2410_key_read,
};

static int __init s3c2410_key_init(void)
{
	int iRet;

	iRet = register_chrdev(KEY_MAJOR, DEVICE_NAME, &s3c2410_key_fops);
	if (iRet < 0) {
		printk(DEVICE_NAME " can't register major number\n");
		return iRet;
	}
	printk(DEVICE_NAME " initialized!\n");
	return 0;
}
static void __exit s3c2410_key_exit(void)
{
	unregister_chrdev(KEY_MAJOR, DEVICE_NAME);
	printk(DEVICE_NAME " exit!\n");
}

module_init(s3c2410_key_init);
module_exit(s3c2410_key_exit);

