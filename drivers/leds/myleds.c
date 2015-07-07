#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/irq.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>

#define DEVICE_NAME		"leds"
#define LED_MAJOR		231

#define IOCTL_LED_ON	0
#define IOCTL_LED_OFF	1

static unsigned long led_table[] = {
	S3C2410_GPB5,
	S3C2410_GPB6,
	S3C2410_GPB8,
	S3C2410_GPB10,
};

static unsigned int led_cfg_table[] = {
	S3C2410_GPB5_OUTP,
	S3C2410_GPB6_OUTP,
	S3C2410_GPB8_OUTP,
	S3C2410_GPB10_OUTP,
};

static int s3c24xx_leds_open(struct inode *inode, struct file *file)
{
	int i;

	for (i = 0; i < 4; i++) {
		s3c2410_gpio_cfgpin(led_table[i], led_cfg_table[i]);
	}
	return 0;
}

static int s3c24xx_leds_ioctl(
	struct inode *inode,
	struct file *file,
	unsigned int cmd,
	unsigned long arg)
{
	if (arg > 4) {
		return -EINVAL;
	}

	switch(cmd) {
		case IOCTL_LED_ON:
			s3c2410_gpio_setpin(led_table[arg], 0);
			return 0;

		case IOCTL_LED_OFF:
			s3c2410_gpio_setpin(led_table[arg], 1);
			return 0;

		default:
			return -EINVAL;
	}
}

static struct file_operations s3c24xx_leds_fops = {
	.owner = THIS_MODULE,
	.open = s3c24xx_leds_open,
	.ioctl = s3c24xx_leds_ioctl,
};

static int __init s3c24xx_leds_init(void)
{
	int ret;

	ret = register_chrdev(LED_MAJOR, DEVICE_NAME, &s3c24xx_leds_fops);
	if (ret < 0) {
		printk(DEVICE_NAME " can't register major number\n");
		return ret;
	}

	printk(DEVICE_NAME "initialized\n");
	return 0;
}

static void __exit s3c24xx_leds_exit(void)
{
	unregister_chrdev(LED_MAJOR, DEVICE_NAME);
}

module_init(s3c24xx_leds_init);
module_exit(s3c24xx_leds_exit);

MODULE_AUTHOR("TSANN");
MODULE_DESCRIPTION("S3C2410/S3C2440 LED Driver");
MODULE_LICENSE("GPL");

