#ifndef __FRAME_H__
#define __FRAME_H__

#define PSR_MODE_USR  0x10
#define PSR_MODE_FIQ  0x11
#define PSR_MODE_IRQ  0x12
#define PSR_MODE_SVC  0x13
#define PSR_MODE_ABT  0x17
#define PSR_MODE_UND  0x1b
#define PSR_MODE_SYS  0x1f
#define PSR_MODE_MASK 0x1f

#define CF_SPSR		0
#define CF_R0		4
#define CF_R1		8
#define CF_R2		12
#define CF_R3		16
#define CF_R4		20
#define CF_R5		24
#define CF_R6		28
#define CF_R7		32
#define CF_R8		36
#define CF_R9		40
#define CF_R10		44
#define CF_R11		48
#define CF_R12		52
#define CF_USR_SP	56
#define CF_USR_LR	60
#define CF_SVC_SP	64
#define CF_SVC_LR	68
#define CF_PC		72

#define FRAME_SIZE	76

#define	IRQ_R0		0
#define	IRQ_SPSR	4
#define	IRQ_LR		8
#define	IRQ_SIZE	12

#define PUSHFRAMEINSVC					\
	sub		lr, lr, #4;					\
	sub		r13, r13, #IRQ_SIZE;		\
	str		lr, [r13, #IRQ_LR];			\
	mrs		lr, spsr;					\
	str		lr, [r13, #IRQ_SPSR];		\
	str		r0, [r13, #IRQ_R0];			\
	mov		r0, r13;					\
	add		r13, r13, #12;				\
	mrs		r14, cpsr;					\
	bic		r14, r14, #PSR_MODE_MASK;	\
	orr		r14, r14, #PSR_MODE_SVC;	\
	msr		cpsr_cxsf, r14;				\
	sub		sp, sp, #FRAME_SIZE;		\
	str		lr, [sp, #CF_SVC_LR];		\
	ldr		lr, [r0, #IRQ_LR];			\
	str		lr, [sp, #CF_PC];			\
	ldr		r14, [r0, #IRQ_SPSR];		\
	str		r14, [sp, #CF_SPSR];		\
	ldr		r0, [r0, #IRQ_R0];			\
	add		r14, sp, #4;				\
	stmia	r14!, {r0-r12};				\
	stmia	r14, {r13, r14}^;			\
	str		sp, [sp, #CF_SVC_SP]

#define PULLFRAMEANDEXIT				\
	ldmia	sp!, {r0};					\
	msr		spsr, r0;					\
	ldmia	sp!, {r0-r12};				\
	ldmia	sp, {r13, r14}^;			\
	add		sp, sp, #(3*4);				\
	ldmia	sp!, {lr, pc}^


#endif
