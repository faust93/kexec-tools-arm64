#ifndef PURGATORY_H
#define PURGATORY_H

void putchar(int ch);
void sprintf(char *buffer, const char *fmt, ...)
	__attribute__ ((format (printf, 2, 3)));
void printf(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void setup_arch(void);
void post_verification_setup_arch(void);

static inline void model_brk(int line)
{
	asm volatile(
		"mov x0, #0x18;"	/* angel_SWIreason_ReportException */
		"mov x1, #0x20000;"
		"add x1, x1, #0x20;"	/* ADP_Stopped_BreakPoint */
		"hlt #0xf000\n"		/* A64 semihosting */
		:
		: "r" (line)
		: "x0", "x1");
}

#endif /* PURGATORY_H */
