#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/irq.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>

#define DEVICE_NAME		"buttons"
#define BUTTON_MAJOR	232

struct button_irq_desc {
	int irq;
	unsigned long flags;
	char *name;
};

static struct button_irq_desc button_irqs [] = {
	{IRQ_EINT0, IRQF_TRIGGER_FALLING,	"KEY1"},
	{IRQ_EINT2, IRQF_TRIGGER_FALLING,	"KEY2"},
	{IRQ_EINT3, IRQF_TRIGGER_FALLING,	"KEY3"},
	{IRQ_EINT4, IRQF_TRIGGER_FALLING,	"KEY4"},
};

static volatile int press_cnt [] = {0, 0, 0, 0};
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);
static volatile int ev_press = 0;

static irqreturn_t buttons_interrupt(int irq, void *dev_id)
{
	volatile int *press_cnt = (volatile int *)dev_id;

	*press_cnt = *press_cnt + 1;
	ev_press = 1;
	wake_up_interruptible(&button_waitq);

	return IRQ_RETVAL(IRQ_HANDLED);
}

static int s3c24xx_buttons_open(struct inode *inode, struct file *file)
{
	int i;
	int err;

	for (i = 0; i < sizeof(button_irqs)/sizeof(button_irqs[0]); i++) {
		err = request_irq(button_irqs[i].irq, buttons_interrupt, button_irqs[i].flags,
						  button_irqs[i].name, (void *)&press_cnt[i]);
		if (err)
			break;
	}

	if (err) {
		i --;
		for (; i >= 0; i--)
			free_irq(button_irqs[i].irq, (void *)&press_cnt[i]);
		return -EBUSY;
	}

	return 0;
}

static int s3c24xx_buttons_close(struct inode *inode, struct file *file)
{
	int i;

	for (i = 0; i < sizeof(button_irqs)/sizeof(button_irqs[0]); i++) {
		free_irq(button_irqs[i].irq, (void *)&press_cnt[i]);
	}

	return 0;
}

static int s3c24xx_buttons_read(struct file *filp, char __user *buff,
									size_t count, loff_t *offp)
{
	unsigned long err;

	wait_event_interruptible(button_waitq, ev_press);

	ev_press = 0;

	err = copy_to_user(buff, (const void *)press_cnt, min(sizeof(press_cnt), count));
	memset((void *)press_cnt, 0, sizeof(press_cnt));

	return err ? -EFAULT : 0;
}

static struct file_operations s3c24xx_buttons_fops = {
	.owner		=	THIS_MODULE,
	.open		=	s3c24xx_buttons_open,
	.release	=	s3c24xx_buttons_close,
	.read		=	s3c24xx_buttons_read,
};

static int __init s3c24xx_buttons_init(void)
{
	int ret;

	ret = register_chrdev(BUTTON_MAJOR, DEVICE_NAME, &s3c24xx_buttons_fops);
	if (ret < 0) {
		printk(DEVICE_NAME " can't register major number\n");
		return ret;
	}

	printk(DEVICE_NAME "initialized\n");
	return 0;
}

static void __exit s3c24xx_buttons_exit(void)
{
	unregister_chrdev(BUTTON_MAJOR, DEVICE_NAME);
}

module_init(s3c24xx_buttons_init);
module_exit(s3c24xx_buttons_exit);

MODULE_AUTHOR("Tsann");
MODULE_DESCRIPTION("S3C2410/S3C2440 BUTTON Driver");
MODULE_LICENSE("GPL");

