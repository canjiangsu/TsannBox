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

#include "key_int.h"

int key_major = 0;
int key_minor = 0;

module_param(key_major, int, S_IRUGO);

#define DEVICE_NAME	"key_int"

#define MAX_KEY_BUF	16
#define KEY_NUM 4
#define KEY_TIMER_DELAY (20*HZ/1000)

#define KEYSTATUS_INT	1
#define KEYSTATUS_DOWN	2
#define KEYSTATUS_UP	3

#define ISKEY_DOWN(key)	(!s3c2410_gpio_getpin(key_info_tab[key].gpio_port))

MODULE_AUTHOR("Tsann");
MODULE_LICENSE("Dual BSD/GPL");

typedef unsigned char KEY_RET;

typedef struct _KEY_DEV_{
	unsigned int keyStatus[KEY_NUM];
	KEY_RET buf[MAX_KEY_BUF];
	unsigned int head;
	unsigned int tail;
	wait_queue_head_t wq;
	struct cdev cdev;
	struct timer_list timer[KEY_NUM];
}KEY_DEV;

static KEY_DEV keydev;

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
	keydev.timer[key].expires = jiffies+KEY_TIMER_DELAY;
	add_timer(&(keydev.timer[key]));
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
	keydev.buf[keydev.tail] = key_no;
	keydev.tail ++;
	if (keydev.tail >= MAX_KEY_BUF)
		keydev.tail = 0;
	wake_up_interruptible(&(keydev.wq));
	return;
}
static void key_timer_handler(unsigned long data)
{
	int key = (int)data;
	if (ISKEY_DOWN(key)) {
		if (keydev.keyStatus[key] == KEYSTATUS_INT) { //从中断进入
			keydev.keyStatus[key] = KEYSTATUS_DOWN;
			keyEvent(key_info_tab[key].key_no);	//记录键值，唤醒等待队列
		}
		keydev.timer[key].expires = jiffies+KEY_TIMER_DELAY;
		add_timer(&(keydev.timer[key]));
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

	keydev.head = keydev.tail = 0;
	for (i=0; i<KEY_NUM; i++)
		keydev.keyStatus[i] = KEYSTATUS_UP;
	init_waitqueue_head(&(keydev.wq));

	for (i=0; i<KEY_NUM; i++) {
		init_timer(&(keydev.timer[i]));
		keydev.timer[i].function = key_timer_handler;
		keydev.timer[i].data = (unsigned long)i;
	}
	request_irqs(); //注册中断函数
	return 0;
}
static int s3c2410_key_release(struct inode *inode, struct file *filp)
{
//	keyEvent = keyEvent_dummy;
	int i;
	
	for (i = 0; i < KEY_NUM; i++) {
		del_timer(&(keydev.timer[i]));
		disable_irq(key_info_tab[i].irq_no);
		free_irq(key_info_tab[i].irq_no, (void *)i);
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
	key_ret = keydev.buf[keydev.head];
	keydev.head ++;
	if (keydev.head >= MAX_KEY_BUF)
		keydev.head = 0;
	
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
		wait_event_interruptible(keydev.wq, (keydev.head != keydev.tail));
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

int KeyInt_init(void)
{
	int iRet;
	dev_t devno;

	printk(KERN_ALERT "KeyInt_init: haha0!\n");

	if (key_major) {
		devno = MKDEV(key_major, key_minor);
		iRet = register_chrdev_region(devno, 1, DEVICE_NAME);
	} else {
		iRet = alloc_chrdev_region(&devno, key_minor, 1, DEVICE_NAME);
		key_major = MAJOR(devno);
	}

	if (iRet < 0) {
		printk(KERN_WARNING "key_int: can't get major %d\n", key_major);
		return iRet;
	}
	cdev_init(&keydev.cdev, &s3c2410_key_fops);
	keydev.cdev.owner = THIS_MODULE;
	iRet = cdev_add(&keydev.cdev, devno, 1);

	if (iRet) {
		printk(KERN_NOTICE "Error %d adding key_int", iRet);
		return iRet;
	}

	return 0;
}
void KeyInt_exit(void)
{
	dev_t devno = MKDEV(key_major, key_minor);

	printk(KERN_ALERT "KeyInt exit!\n");
	cdev_del(&keydev.cdev);
	unregister_chrdev_region(devno, 1);

}

module_init(KeyInt_init);
module_exit(KeyInt_exit);

