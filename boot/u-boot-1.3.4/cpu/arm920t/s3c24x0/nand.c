/*
 * (C) Copyright 2006 OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
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
 */

#include <common.h>

#if 0
#define DEBUGN	printf
#else
#define DEBUGN(x, args ...) {}
#endif

#if defined(CONFIG_CMD_NAND)
#if !defined(CFG_NAND_LEGACY)

#include <nand.h>
#include <s3c2440.h>
#include <asm/io.h>

#define S3C2440_NFCONT_EN          (1<<0)
#define S3C2440_NFCONT_INITECC     (1<<4)
#define S3C2440_NFCONT_nFCE        (1<<1)
#define S3C2440_NFCONT_MAINECCLOCK (1<<5)
#define S3C2440_NFCONF_TACLS(x)    ((x)<<12)
#define S3C2440_NFCONF_TWRPH0(x)   ((x)<<8)
#define S3C2440_NFCONF_TWRPH1(x)   ((x)<<4)

#define S3C2440_ADDR_NALE 0x08
#define S3C2440_ADDR_NCLE 0x0c

static void s3c2440_hwcontrol(struct mtd_info *mtd, int cmd)
{
	struct nand_chip *chip = mtd->priv;
	S3C2440_NAND * nand_reg = S3C2440_GetBase_NAND();
	
	DEBUGN("hwcontrol(): 0x%02x\r\n", cmd);

	switch (cmd) {
		case NAND_CTL_SETNCE:
			writel(readl(&nand_reg->NFCONT) & ~S3C2440_NFCONT_nFCE, 
				&nand_reg->NFCONT);
			DEBUGN("NFCONT=0x%08x\n", readl(&nand_reg->NFCONT));
			break;
		case NAND_CTL_CLRNCE:
			writel(readl(&nand_reg->NFCONT) | S3C2440_NFCONT_nFCE,
				   &nand_reg->NFCONT);
			DEBUGN("NFCONT=0x%08x\n", readl(&nand_reg->NFCONT));
			break;
		case NAND_CTL_SETALE:
			chip->IO_ADDR_W = &nand_reg->NFADDR;
			DEBUGN("SETALE=%08X\r\n", chip->IO_ADDR_W);
			break;
		case NAND_CTL_SETCLE:
			chip->IO_ADDR_W = &nand_reg->NFCMD;
			DEBUGN("SETCLE=%08X\r\n", chip->IO_ADDR_W);
			break;
		default:
			chip->IO_ADDR_W = &nand_reg->NFDATA;
			DEBUGN("default=%08X\r\n", chip->IO_ADDR_W);
			break;
	}
	return;
}


static int s3c2440_dev_ready(struct mtd_info *mtd)
{
	S3C2440_NAND *nand_reg = S3C2440_GetBase_NAND();
	DEBUGN("s3c2440_dev_ready\r\n");
	return readl(&nand_reg->NFSTAT) & 0x01;
}


#ifdef CONFIG_S3C2410_NAND_HWECC
void s3c2410_nand_enable_hwecc(struct mtd_info *mtd, int mode)
{
	DEBUGN("s3c2410_nand_enable_hwecc(%p, %d)\n", mtd ,mode);
	NFCONF |= S3C2410_NFCONF_INITECC;
}

static int s3c2410_nand_calculate_ecc(struct mtd_info *mtd, const u_char *dat,
				      u_char *ecc_code)
{
	ecc_code[0] = NFECC0;
	ecc_code[1] = NFECC1;
	ecc_code[2] = NFECC2;
	DEBUGN("s3c2410_nand_calculate_hwecc(%p,): 0x%02x 0x%02x 0x%02x\n",
		mtd , ecc_code[0], ecc_code[1], ecc_code[2]);

	return 0;
}

static int s3c2410_nand_correct_data(struct mtd_info *mtd, u_char *dat,
				     u_char *read_ecc, u_char *calc_ecc)
{
	if (read_ecc[0] == calc_ecc[0] &&
	    read_ecc[1] == calc_ecc[1] &&
	    read_ecc[2] == calc_ecc[2])
		return 0;

	printf("s3c2410_nand_correct_data: not implemented\n");
	return -1;
}
#endif

int board_nand_init(struct nand_chip *nand)
{
	u_int32_t cfg;
	u_int8_t tacls, twrph0, twrph1;
	S3C24X0_CLOCK_POWER * const clk_power = S3C24X0_GetBase_CLOCK_POWER();
	S3C2440_NAND * const nand_reg = S3C2440_GetBase_NAND();
		
	printf("board_nand_init()\n");

	writel(readl(&clk_power->CLKCON) | (1 << 4), &clk_power->CLKCON);
	
#if defined(CONFIG_S3C2440)
	twrph0 = 4;
	twrph1 = 1;
	tacls = 1;
	// TACLS		[14:12]	CLE&ALE duration = HCLK*TACLS.
	// TWRPH0		[10:8]	TWRPH0 duration = HCLK*(TWRPH0+1)
	// TWRPH1		[6:4]	TWRPH1 duration = HCLK*(TWRPH1+1)
	// AdvFlash(R)	[3]		Advanced NAND, 0:256/512, 1:1024/2048
	// PageSize(R)	[2]		NAND memory page size
	//						when [3]==0, 0:256, 1:512 bytes/page.
	//						when [3]==1, 0:1024, 1:2048 bytes/page.
	// AddrCycle(R)	[1]		NAND flash addr size
	//						when [3]==0, 0:3-addr, 1:4-addr.
	//						when [3]==1, 0:4-addr, 1:5-addr.
	// BusWidth(R/W) [0]	NAND bus width. 0:8-bit, 1:16-bit.
	
	cfg = 0;
	cfg |= S3C2440_NFCONF_TACLS(tacls);
	cfg |= S3C2440_NFCONF_TWRPH0(twrph0);
	cfg |= S3C2440_NFCONF_TWRPH1(twrph1);
	writel(cfg, &nand_reg->NFCONF);

	cfg = (0<<13)|(0<<12)|(0<<10)|(0<<9)|(0<<8)|(1<<6)|(1<<5)|(1<<4)|(1<<1)|(1<<0);
	// Lock-tight	[13]	0:Disable lock, 1:Enable lock.
	// Soft Lock	[12]	0:Disable lock, 1:Enable lock.
	// EnablillegalAcINT[10]	Illegal access interupt control. 0:Disable, 1:Enable
	// EnbRnBINT	[9]		RnB interrupt. 0:Disable, 1:Enable
	// RnB_TrandMode[8]		RnB transition detection config. 0:Low to High, 1:High to Low
	// SpareECCLock	[6]		0:Unlock, 1:Lock
	// MainECCLock	[5]		0:Unlock, 1:Lock
	// InitECC(W)	[4]		1:Init ECC decoder/encoder.
	// Reg_nCE		[1]		0:nFCE=0, 1:nFCE=1.
	// NANDC Enable	[0]		operating mode. 0:Disable, 1:Enable.

	writel(cfg, &nand_reg->NFCONT);
	printf("NFCONFT:%08X\r\n", readl(&nand_reg->NFCONT));
	/* initialize nand_chip data structure */
	nand->IO_ADDR_R = nand->IO_ADDR_W = (void *)&nand_reg->NFDATA;
#endif

	/* read_buf and write_buf are default */
	/* read_byte and write_byte are default */

	/* hwcontrol always must be implemented */
	nand->hwcontrol = s3c2440_hwcontrol;

	nand->dev_ready = s3c2440_dev_ready;

#ifdef CONFIG_S3C2410_NAND_HWECC
	nand->enable_hwecc = s3c2410_nand_enable_hwecc;
	nand->calculate_ecc = s3c2410_nand_calculate_ecc;
	nand->correct_data = s3c2410_nand_correct_data;
	nand->eccmode = NAND_ECC_HW3_512;
#else
	nand->eccmode = NAND_ECC_SOFT;
#endif

#ifdef CONFIG_S3C2410_NAND_BBT
	nand->options = NAND_USE_FLASH_BBT;
#else
	nand->options = 0;
#endif

	printf("end of nand_init:IO_ADDR_R=%08X\r\n", nand->IO_ADDR_R);

	return 0;
}

#else
 #error "U-Boot legacy NAND support not available for S3C2410"
#endif
#endif
