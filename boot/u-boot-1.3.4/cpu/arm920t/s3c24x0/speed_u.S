
speed.o:     file format elf32-littlearm

Disassembly of section .text:

00000000 <get_PLLCLK>:
 * the specified bus in HZ.
 */
/* ------------------------------------------------------------------------- */

static ulong get_PLLCLK(int pllreg)
{
   0:	e92d4800 	push	{fp, lr}
   4:	e28db004 	add	fp, sp, #4	; 0x4
   8:	e24dd018 	sub	sp, sp, #24	; 0x18
   c:	e50b001c 	str	r0, [fp, #-28]
    S3C24X0_CLOCK_POWER * const clk_power = S3C24X0_GetBase_CLOCK_POWER();
  10:	eb000035 	bl	ec <S3C24X0_GetBase_CLOCK_POWER>
  14:	e1a03000 	mov	r3, r0
  18:	e50b3018 	str	r3, [fp, #-24]
    ulong r, m, p, s;

    if (pllreg == MPLL)
  1c:	e51b301c 	ldr	r3, [fp, #-28]
  20:	e3530000 	cmp	r3, #0	; 0x0
  24:	1a000003 	bne	38 <get_PLLCLK+0x38>
	r = clk_power->MPLLCON;
  28:	e51b3018 	ldr	r3, [fp, #-24]
  2c:	e5933004 	ldr	r3, [r3, #4]
  30:	e50b3014 	str	r3, [fp, #-20]
  34:	ea000007 	b	58 <get_PLLCLK+0x58>
    else if (pllreg == UPLL)
  38:	e51b301c 	ldr	r3, [fp, #-28]
  3c:	e3530001 	cmp	r3, #1	; 0x1
  40:	1a000003 	bne	54 <get_PLLCLK+0x54>
	r = clk_power->UPLLCON;
  44:	e51b3018 	ldr	r3, [fp, #-24]
  48:	e5933008 	ldr	r3, [r3, #8]
  4c:	e50b3014 	str	r3, [fp, #-20]
  50:	ea000000 	b	58 <get_PLLCLK+0x58>
    else
	hang();
  54:	ebfffffe 	bl	0 <hang>

    m = ((r & 0xFF000) >> 12) + 8;
  58:	e51b3014 	ldr	r3, [fp, #-20]
  5c:	e2033aff 	and	r3, r3, #1044480	; 0xff000
  60:	e1a03623 	lsr	r3, r3, #12
  64:	e2833008 	add	r3, r3, #8	; 0x8
  68:	e50b3010 	str	r3, [fp, #-16]
    p = ((r & 0x003F0) >> 4) + 2;
  6c:	e51b3014 	ldr	r3, [fp, #-20]
  70:	e2033e3f 	and	r3, r3, #1008	; 0x3f0
  74:	e1a03223 	lsr	r3, r3, #4
  78:	e2833002 	add	r3, r3, #2	; 0x2
  7c:	e50b300c 	str	r3, [fp, #-12]
    s = r & 0x3;
  80:	e51b3014 	ldr	r3, [fp, #-20]
  84:	e2033003 	and	r3, r3, #3	; 0x3
  88:	e50b3008 	str	r3, [fp, #-8]

    return((CONFIG_SYS_CLK_FREQ * m) / (p << s));
  8c:	e51b1010 	ldr	r1, [fp, #-16]
  90:	e1a03001 	mov	r3, r1
  94:	e1a02103 	lsl	r2, r3, #2
  98:	e1a03282 	lsl	r3, r2, #5
  9c:	e0623003 	rsb	r3, r2, r3
  a0:	e0833001 	add	r3, r3, r1
  a4:	e1a02103 	lsl	r2, r3, #2
  a8:	e0833002 	add	r3, r3, r2
  ac:	e1a02103 	lsl	r2, r3, #2
  b0:	e0833002 	add	r3, r3, r2
  b4:	e1a02203 	lsl	r2, r3, #4
  b8:	e0632002 	rsb	r2, r3, r2
  bc:	e1a03402 	lsl	r3, r2, #8
  c0:	e1a01003 	mov	r1, r3
  c4:	e51b2008 	ldr	r2, [fp, #-8]
  c8:	e51b300c 	ldr	r3, [fp, #-12]
  cc:	e1a03213 	lsl	r3, r3, r2
  d0:	e1a00001 	mov	r0, r1
  d4:	e1a01003 	mov	r1, r3
  d8:	ebfffffe 	bl	0 <__udivsi3>
  dc:	e1a03000 	mov	r3, r0
}
  e0:	e1a00003 	mov	r0, r3
  e4:	e24bd004 	sub	sp, fp, #4	; 0x4
  e8:	e8bd8800 	pop	{fp, pc}

000000ec <S3C24X0_GetBase_CLOCK_POWER>:
static inline S3C24X0_DMAS * S3C24X0_GetBase_DMAS(void)
{
	return (S3C24X0_DMAS * const)S3C24X0_DMA_BASE;
}
static inline S3C24X0_CLOCK_POWER * S3C24X0_GetBase_CLOCK_POWER(void)
{
  ec:	e52db004 	push	{fp}		; (str fp, [sp, #-4]!)
  f0:	e28db000 	add	fp, sp, #0	; 0x0
	return (S3C24X0_CLOCK_POWER * const)S3C24X0_CLOCK_POWER_BASE;
  f4:	e3a03313 	mov	r3, #1275068416	; 0x4c000000
}
  f8:	e1a00003 	mov	r0, r3
  fc:	e28bd000 	add	sp, fp, #0	; 0x0
 100:	e8bd0800 	pop	{fp}
 104:	e1a0f00e 	mov	pc, lr

00000108 <get_FCLK>:

/* return FCLK frequency */
ulong get_FCLK(void)
{
 108:	e92d4800 	push	{fp, lr}
 10c:	e28db004 	add	fp, sp, #4	; 0x4
    return(get_PLLCLK(MPLL));
 110:	e3a00000 	mov	r0, #0	; 0x0
 114:	ebffffb9 	bl	0 <get_PLLCLK>
 118:	e1a03000 	mov	r3, r0
}
 11c:	e1a00003 	mov	r0, r3
 120:	e8bd8800 	pop	{fp, pc}

00000124 <get_HCLK>:

/* return HCLK frequency */
ulong get_HCLK(void)
{
 124:	e92d4800 	push	{fp, lr}
 128:	e28db004 	add	fp, sp, #4	; 0x4
 12c:	e24dd008 	sub	sp, sp, #8	; 0x8
    S3C24X0_CLOCK_POWER * const clk_power = S3C24X0_GetBase_CLOCK_POWER();
 130:	ebffffed 	bl	ec <S3C24X0_GetBase_CLOCK_POWER>
 134:	e1a03000 	mov	r3, r0
 138:	e50b3008 	str	r3, [fp, #-8]

    return((clk_power->CLKDIVN & 0x2) ? get_FCLK()/2 : get_FCLK());
 13c:	e51b3008 	ldr	r3, [fp, #-8]
 140:	e5933014 	ldr	r3, [r3, #20]
 144:	e2033002 	and	r3, r3, #2	; 0x2
 148:	e3530000 	cmp	r3, #0	; 0x0
 14c:	0a000004 	beq	164 <get_HCLK+0x40>
 150:	ebfffffe 	bl	108 <get_FCLK>
 154:	e1a03000 	mov	r3, r0
 158:	e1a030a3 	lsr	r3, r3, #1
 15c:	e50b300c 	str	r3, [fp, #-12]
 160:	ea000002 	b	170 <get_HCLK+0x4c>
 164:	ebfffffe 	bl	108 <get_FCLK>
 168:	e1a03000 	mov	r3, r0
 16c:	e50b300c 	str	r3, [fp, #-12]
 170:	e51b300c 	ldr	r3, [fp, #-12]
}
 174:	e1a00003 	mov	r0, r3
 178:	e24bd004 	sub	sp, fp, #4	; 0x4
 17c:	e8bd8800 	pop	{fp, pc}

00000180 <get_PCLK>:

/* return PCLK frequency */
ulong get_PCLK(void)
{
 180:	e92d4800 	push	{fp, lr}
 184:	e28db004 	add	fp, sp, #4	; 0x4
 188:	e24dd008 	sub	sp, sp, #8	; 0x8
    S3C24X0_CLOCK_POWER * const clk_power = S3C24X0_GetBase_CLOCK_POWER();
 18c:	ebffffd6 	bl	ec <S3C24X0_GetBase_CLOCK_POWER>
 190:	e1a03000 	mov	r3, r0
 194:	e50b3008 	str	r3, [fp, #-8]

    return((clk_power->CLKDIVN & 0x1) ? get_HCLK()/2 : get_HCLK());
 198:	e51b3008 	ldr	r3, [fp, #-8]
 19c:	e5933014 	ldr	r3, [r3, #20]
 1a0:	e2033001 	and	r3, r3, #1	; 0x1
 1a4:	e20330ff 	and	r3, r3, #255	; 0xff
 1a8:	e3530000 	cmp	r3, #0	; 0x0
 1ac:	0a000004 	beq	1c4 <get_PCLK+0x44>
 1b0:	ebfffffe 	bl	124 <get_HCLK>
 1b4:	e1a03000 	mov	r3, r0
 1b8:	e1a030a3 	lsr	r3, r3, #1
 1bc:	e50b300c 	str	r3, [fp, #-12]
 1c0:	ea000002 	b	1d0 <get_PCLK+0x50>
 1c4:	ebfffffe 	bl	124 <get_HCLK>
 1c8:	e1a03000 	mov	r3, r0
 1cc:	e50b300c 	str	r3, [fp, #-12]
 1d0:	e51b300c 	ldr	r3, [fp, #-12]
}
 1d4:	e1a00003 	mov	r0, r3
 1d8:	e24bd004 	sub	sp, fp, #4	; 0x4
 1dc:	e8bd8800 	pop	{fp, pc}

000001e0 <get_UCLK>:

/* return UCLK frequency */
ulong get_UCLK(void)
{
 1e0:	e92d4800 	push	{fp, lr}
 1e4:	e28db004 	add	fp, sp, #4	; 0x4
    return(get_PLLCLK(UPLL));
 1e8:	e3a00001 	mov	r0, #1	; 0x1
 1ec:	ebffff83 	bl	124 <get_HCLK>
 1f0:	e1a03000 	mov	r3, r0
}
 1f4:	e1a00003 	mov	r0, r3
 1f8:	e8bd8800 	pop	{fp, pc}
