#include <config.h>

#define NADN_BOOT_DEBUG	0
/***************·ÉÁè*****************/
#define rGPACON		(*(volatile unsigned *)0x56000000)

//Nand Flash
#define rNFCONF		(*(volatile unsigned *)0x4E000000)		//NAND Flash configuration
#define rNFCONT		(*(volatile unsigned *)0x4E000004)      //NAND Flash control
#define rNFCMD		(*(volatile unsigned *)0x4E000008)      //NAND Flash command
#define rNFADDR		(*(volatile unsigned *)0x4E00000C)      //NAND Flash address
#define rNFDATA		(*(volatile unsigned *)0x4E000010)      //NAND Flash data
#define rNFDATA8	(*(volatile unsigned char *)0x4E000010)     //NAND Flash data
#define rNFMECCD0	(*(volatile unsigned *)0x4E000014)      //NAND Flash ECC for Main Area
#define rNFMECCD1	(*(volatile unsigned *)0x4E000018)
#define rNFSECCD	(*(volatile unsigned *)0x4E00001C)		//NAND Flash ECC for Spare Area
#define rNFSTAT		(*(volatile unsigned *)0x4E000020)		//NAND Flash operation status
#define rNFESTAT0	(*(volatile unsigned *)0x4E000024)
#define rNFESTAT1	(*(volatile unsigned *)0x4E000028)
#define rNFMECC0	(*(volatile unsigned *)0x4E00002C)
#define rNFMECC1	(*(volatile unsigned *)0x4E000030)
#define rNFSECC		(*(volatile unsigned *)0x4E000034)
#define rNFSBLK		(*(volatile unsigned *)0x4E000038)		//NAND Flash Start block address
#define rNFEBLK		(*(volatile unsigned *)0x4E00003C)		//NAND Flash End block address

// HCLK=100Mhz
#define TACLS		1//7	// 1-clk(0ns) 
#define TWRPH0		4//7	// 3-clk(25ns)
#define TWRPH1		1//7	// 1-clk(10ns)  //TACLS+TWRPH0+TWRPH1>=50ns

#define	EnNandFlash()	(rNFCONT |= 1)
#define	DsNandFlash()	(rNFCONT &= ~1)
#define	NFChipEn()		(rNFCONT &= ~(1<<1))
#define	NFChipDs()		(rNFCONT |= (1<<1))
#define	InitEcc()		(rNFCONT |= (1<<4))
#define	MEccUnlock()	(rNFCONT &= ~(1<<5))
#define	MEccLock()		(rNFCONT |= (1<<5))
#define	SEccUnlock()	(rNFCONT &= ~(1<<6))
#define	SEccLock()		(rNFCONT |= (1<<6))

#define	WrNFDat8(dat)	(rNFDATA8 = (dat))
#define	WrNFDat32(dat)	(rNFDATA = (dat))
#define	RdNFDat8()		(rNFDATA8)	//byte access
#define	RdNFDat32()		(rNFDATA)	//word access

#define	WrNFCmd(cmd)	(rNFCMD = (cmd))
#define	WrNFAddr(addr)	(rNFADDR = (addr))
#define	WrNFDat(dat)	WrNFDat8(dat)
#define	RdNFDat()		RdNFDat8()	//for 8 bit nand flash, use byte access

#define	RdNFMEcc()		(rNFMECC0)	//for 8 bit nand flash, only use NFMECC0
#define	RdNFSEcc()		(rNFSECC)	//for 8 bit nand flash, only use low 16 bits

#define	RdNFStat()		(rNFSTAT)
#define	NFIsBusy()		(!(rNFSTAT&1))
#define	NFIsReady()		(rNFSTAT&1)

#define	READCMD0	0
#define	READCMD1	0x30
#define	ERASECMD0	0x60
#define	ERASECMD1	0xd0
#define	PROGCMD0	0x80
#define	PROGCMD1	0x10
#define	QUERYCMD	0x70
#define	RdIDCMD		0x90

#define NAND_5_ADDR_CYCLE	1
#define NAND_PAGE_SIZE  	2048
#define BAD_BLOCK_OFFSET 	NAND_PAGE_SIZE
#define NAND_BLOCK_MASK  	(NAND_PAGE_SIZE - 1)
#define NAND_BLOCK_SIZE  	(NAND_PAGE_SIZE * 64)

int NandAddr = 0;
#ifdef	WIAT_BUSY_HARD
#define	WaitNFBusy()	while(NFIsBusy())
#else
static unsigned int WaitNFBusy(void)	// R/B Î´½ÓºÃ?
{
	unsigned int stat;
	
	WrNFCmd(QUERYCMD);
	do {
		stat = RdNFDat();
		//printf("%x\n", stat);
	}while(!(stat&0x40));
	WrNFCmd(READCMD0);
	return stat&1;
}
#endif

int CheckBadBlk(unsigned int addr)
{
	unsigned int dat;
	
	addr &= ~0x3f;


	NFChipEn();
	WrNFCmd(READCMD0);
	WrNFAddr(0);		//2048&0xff
	WrNFAddr(8);		//(2048>>8)&0xf
	WrNFAddr(addr);
	WrNFAddr(addr>>8);
	if(NandAddr)
		WrNFAddr(addr>>16);
	WrNFCmd(READCMD1);
	WaitNFBusy();
	dat = RdNFDat();
	
	
	NFChipDs();
	return (dat!=0xff);
}

void ReadPage(unsigned int addr, unsigned char *buf)
{
	unsigned short i;
	Uart_SendByte('R');
	NFChipEn();
	WrNFCmd(READCMD0);
	WrNFAddr(0);
	WrNFAddr(0);
	WrNFAddr(addr);
	WrNFAddr(addr>>8);
	if(NandAddr)
		WrNFAddr(addr>>16);	
	WrNFCmd(READCMD1);
	InitEcc();
	WaitNFBusy();
	for(i=0; i<2048; i++)
		buf[i] = RdNFDat();
	NFChipDs();
}

void InitNandCfg(void)
{
	rGPACON = (rGPACON &~(0x3f<<17)) | (0x3f<<17);
	
	rNFCONF = (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4)|(0<<0);
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
	
	rNFCONT = (0<<13)|(0<<12)|(0<<10)|(0<<9)|(0<<8)|(1<<6)|(1<<5)|(1<<4)|(1<<1)|(1<<0);
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
}


int ReadChipId(void)
{
	int id;

	NFChipEn();	
	WrNFCmd(RdIDCMD);
	WrNFAddr(0);
	while(NFIsBusy());	
	id  = RdNFDat()<<8;
	id |= RdNFDat();		
	NFChipDs();		
	
	return id;
}

void nand_boot(void)
{	
	unsigned int i;
#if NADN_BOOT_DEBUG
	unsigned char *ram_addr = 0x30000000;
#else
	unsigned char *ram_addr = TEXT_BASE;
#endif
	int iSize = 0x60000;
	int iStartPage = 0;

	Uart_SendByte('N');
	Uart_SendByte('B');
	
	InitNandCfg();
	i = ReadChipId();
	if(i == 0xecda){
		NandAddr = 1;
		Uart_SendByte('I');
		Uart_SendByte('D');
	}
	for(i=0; iSize>0; ) {
		if(!(i&0x3f)) {
			if(CheckBadBlk(i+iStartPage)) {
				i += 64;
				//iSize -= 64<<11;
				Uart_SendByte('C');
				Uart_SendByte('B');
				continue;
			}
		}
		ReadPage((i+iStartPage), ram_addr);
		i++;
		iSize -= 2048;
		ram_addr += 2048;
	}
	
	DsNandFlash();
}

/*******************************************/

