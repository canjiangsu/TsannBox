/* linux/arch/arm/mach-s3c2410/mach-smdk2410.c
 *
 * linux/arch/arm/mach-s3c2410/mach-smdk2410.c
 *
 * Copyright (C) 2004 by FS Forth-Systeme GmbH
 * All rights reserved.
 *
 * @Author: Jonas Dietsche
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * @History:
 * derived from linux/arch/arm/mach-s3c2410/mach-bast.c, written by
 * Ben Dooks <ben@simtec.co.uk>
 *
 ***********************************************************************/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/io.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/mach-types.h>

#include <plat/regs-serial.h>

#include <plat/devs.h>
#include <plat/cpu.h>

#include <plat/common-smdk.h>

#include <mach/fb.h>
#include <mach/regs-lcd.h>
#include <mach/s3c2410_ts.h>
#include <linux/string.h>



static struct map_desc smdk2410_iodesc[] __initdata = {

[0] = {                                              
  .virtual	 = (unsigned long)S3C24XX_VA_DM9000, 
  .pfn		 = __phys_to_pfn(S3C24XX_PA_DM9000), 
  .length	  = SZ_1M, 
  .type	   = MT_DEVICE, 
  }, 
  
[1] = {											
 .virtual   = 0xf0d00000, 
 .pfn	   = 0x55000000, 
 .length 	= S3C24XX_SZ_IIS, 
 .type	 = MT_DEVICE, 
			}, 

[2] = {												
  .virtual   = (unsigned long)S3C24XX_VA_GPIO, 
  .pfn	   = __phys_to_pfn(S3C2410_PA_GPIO), 
  .length 	= S3C24XX_SZ_GPIO, 
  .type	 = MT_DEVICE, 
	  },  

[3] = {											
    .virtual = vSMDK2410_ETH_IO,
	.pfn  = __phys_to_pfn(S3C2410_CS3 + (1<<24)),
	.length  = SZ_1M,
	.type  = MT_DEVICE,
  
	  },  
	  
};

#define UCON S3C2410_UCON_DEFAULT
#define ULCON S3C2410_LCON_CS8 | S3C2410_LCON_PNONE | S3C2410_LCON_STOPB
#define UFCON S3C2410_UFCON_RXTRIG8 | S3C2410_UFCON_FIFOMODE

static struct s3c2410_uartcfg smdk2410_uartcfgs[] __initdata = {
	[0] = {
		.hwport	     = 0,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
	[1] = {
		.hwport	     = 1,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
	[2] = {
		.hwport	     = 2,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	}
};


/* LCD driver info */ 

static struct s3c2410fb_display smdk2410_lcd_cfg[] __initdata = {
	
		 {
			  /* Config for 320x240 LCD */
				.lcdcon5 = S3C2410_LCDCON5_FRM565 |
					   S3C2410_LCDCON5_INVVLINE |
					   S3C2410_LCDCON5_INVVFRAME |
					   S3C2410_LCDCON5_PWREN |
					   S3C2410_LCDCON5_HWSWP,
			
				.type		= S3C2410_LCDCON1_TFT,
				.width		= 320,
				.height 	= 240,
				.pixclock	= 270000,
				.xres		= 320,
				.yres		= 240,
				.bpp		= 16,
				.left_margin	=8, 
				.right_margin	= 5,  
				.hsync_len	=  63, 
				.upper_margin	= 15,
				.lower_margin	= 3,
				.vsync_len	=  5,
          },
          {
			/* Configuration for 640x480 SHARP LQ080V3DG01 */
			.lcdcon5 = S3C2410_LCDCON5_FRM565 |
				   S3C2410_LCDCON5_INVVLINE |
				   S3C2410_LCDCON5_INVVFRAME |
				   S3C2410_LCDCON5_PWREN |
				   S3C2410_LCDCON5_HWSWP,

			.type		= S3C2410_LCDCON1_TFT,
			.width		= 640,
			.height		= 480,

			.pixclock	= 40000, /* HCLK/4 */
			.xres		= 640,
			.yres		= 480,
			.bpp		= 16,
			.left_margin	= 4,
			.right_margin	= 116,
			.hsync_len	= 96,
			.upper_margin	= 19,
			.lower_margin	= 11,
			.vsync_len	= 15,
	     },
	     {
			/* Configuration for 800x600 SHARP LQ080V3DG01 */
			.lcdcon5 = S3C2410_LCDCON5_FRM565 |
			       S3C2410_LCDCON5_INVVCLK  |
				   S3C2410_LCDCON5_INVVLINE |
				   S3C2410_LCDCON5_INVVFRAME |
				   S3C2410_LCDCON5_PWREN |
				   S3C2410_LCDCON5_HWSWP,

			.type		= S3C2410_LCDCON1_TFT,
			.width		= 800,
			.height		= 600,

			.pixclock	= 40000, /* HCLK/4 */
			.xres		= 800,
			.yres		= 600,
			.bpp		= 16,
			.left_margin	= 55,
			.right_margin	= 190, 
			.hsync_len	=  119,
			.upper_margin	= 36,
			.lower_margin	= 22,
			.vsync_len	= 5,
	     },
         {

		 /* Configuration for 800x480 SHARP LQ080V3DG01 */
			.lcdcon5 = S3C2410_LCDCON5_FRM565 |
				   S3C2410_LCDCON5_INVVLINE |
				   S3C2410_LCDCON5_INVVFRAME |
				   S3C2410_LCDCON5_PWREN |
				   S3C2410_LCDCON5_HWSWP,

			.type		= S3C2410_LCDCON1_TFT,
			.width		= 800,
			.height		= 480,

			.pixclock	= 40000,//40000, /* HCLK/4 */
			.xres		= 800,
			.yres		= 480,
			.bpp		= 16,
			.left_margin	= 41,  
			.right_margin	= 68,  
			.hsync_len	= 32,  
			.upper_margin	= 26,  
			.lower_margin	=6,  
			.vsync_len	= 2,  
	  	},
		 {

		 /* Configuration for 480x272 SHARP LQ080V3DG01 */
			.lcdcon5 = S3C2410_LCDCON5_FRM565 |
				   S3C2410_LCDCON5_INVVLINE |
				   S3C2410_LCDCON5_INVVFRAME |
				   S3C2410_LCDCON5_PWREN |
				   S3C2410_LCDCON5_HWSWP,

			.type		= S3C2410_LCDCON1_TFT,
			.width		= 480,
			.height		= 272,

			.pixclock	= 40000,//40000, /* HCLK/4 */
			.xres		= 480,
			.yres		= 272,
			.bpp		= 16,
			.left_margin	= 2,  
			.right_margin	= 2,  
			.hsync_len	= 41,  
			.upper_margin	= 2,  
			.lower_margin	=2,  
			.vsync_len	= 10,  
	  	},
	 
};



static struct s3c2410fb_mach_info smdk2410_fb_info __initdata = {
	.displays	= smdk2410_lcd_cfg,
	.num_displays	= ARRAY_SIZE(smdk2410_lcd_cfg), 
	.default_display = 0,
    .lpcsel		= 0,  
};


static int TE2440_fb_setup(char *options)
{
  if(!strncmp("vga800", options, 6))
		smdk2410_fb_info.default_display = 2;
	else if(!strncmp("vga640", options, 6))
		smdk2410_fb_info.default_display = 1;
	else if(!strncmp("tv640", options, 6))
		smdk2410_fb_info.default_display = 1;
	else if(!strncmp("shp640", options, 6))
		smdk2410_fb_info.default_display = 1;
	else if(!strncmp("shp240", options, 6))
		smdk2410_fb_info.default_display = 0;
	else if(!strncmp("sam320", options, 6))
		smdk2410_fb_info.default_display = 0;
	else if(!strncmp("qch800", options, 6))
		smdk2410_fb_info.default_display = 3;
	else if(!strncmp("lcd480", options, 6))
		smdk2410_fb_info.default_display = 4;
	else 
		smdk2410_fb_info.default_display = 0;
	return 1;
}
__setup("display=", TE2440_fb_setup);


/* CS8900 */

static struct resource te2410_cs89x0_resources[] = {
	[0] = {
		.start	= 0x19000000,
		.end	= 0x19000000 + 16,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_EINT9,
		.end	= IRQ_EINT9,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device te2410_cs89x0 = {
	.name		= "cirrus-cs89x0",
	.num_resources	= ARRAY_SIZE(te2410_cs89x0_resources),
	.resource	= te2410_cs89x0_resources,
};

/*Config for TouchScreen*/ 
static struct s3c2410_ts_mach_info smdk2410_ts_cfg __initdata = { 
		.delay = 20000, 
		.presc = 49, 
		.oversampling_shift = 2, 
};



static struct platform_device *smdk2410_devices[] __initdata = {
	&s3c_device_usb,
	&s3c_device_lcd,
	&s3c_device_wdt,
	&s3c_device_i2c,
	&s3c_device_iis,
	&s3c_device_sdi,
	&s3c_device_dm9000,
	&te2410_cs89x0,
	&s3c_device_ts,
};

static void __init smdk2410_map_io(void)
{
	s3c24xx_init_io(smdk2410_iodesc, ARRAY_SIZE(smdk2410_iodesc));
	s3c24xx_init_clocks(0);
	s3c24xx_init_uarts(smdk2410_uartcfgs, ARRAY_SIZE(smdk2410_uartcfgs));
}

static void __init smdk2410_init(void)
{
	s3c24xx_fb_set_platdata(&smdk2410_fb_info); 
	set_s3c2410ts_info(&smdk2410_ts_cfg);
	platform_add_devices(smdk2410_devices, ARRAY_SIZE(smdk2410_devices));
	smdk_machine_init();
}

MACHINE_START(SMDK2410, "SMDK2410") /* @TODO: request a new identifier and switch
				    * to SMDK2410 */
	/* Maintainer: Jonas Dietsche */
	.phys_io	= S3C2410_PA_UART,
	.io_pg_offst	= (((u32)S3C24XX_VA_UART) >> 18) & 0xfffc,
	.boot_params	= S3C2410_SDRAM_PA + 0x100,
	.map_io		= smdk2410_map_io,
	.init_irq	= s3c24xx_init_irq,
	.init_machine	= smdk2410_init,
	.timer		= &s3c24xx_timer,
MACHINE_END


