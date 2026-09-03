#ifndef QBOOT_TDX_H
#define QBOOT_TDX_H

#include <stdint.h>

/*
 * TDX helpers required for functional SMP boot.
 *
 * TDVMCALL port I/O is not optional decoration: without enough host IO
 * exits before/between accept and AP release, the BSP #VE's on PENDING
 * pages or races APs and the guest shuts down. Stage POST codes, a short
 * UART warm-up, and bsp_settle() exist to create that traffic.
 */

#define QBOOT_POST_PORT		0x80
#define QBOOT_SERIAL_PORT	0x3f8

#define POST_JMP_MAIN		0x04
#define POST_MAIN		0x10
#define POST_ACCEPT_RAM		0x11
#define POST_ACCEPT_HOLE	0x12
#define POST_MAILBOX		0x20
#define POST_MADT_BEFORE	0x21
#define POST_MADT_AFTER		0x22
#define POST_NVS		0x23
#define POST_MPTABLE		0x24
#define POST_APS_RELEASED	0x25
#define POST_JMP_KERNEL		0x30

/* Fatal error POST codes (0xE0..0xEF) */
#define POST_MADT_FAIL		0xE1
#define POST_NVS_FAIL		0xE2
#define POST_MAILBOX_RANGE_FAIL	0xE3
#define POST_TDACCEPT_FAIL	0xE4

#define TDX_PAGE_ALREADY_ACCEPTED	0x00000B0A00000000ULL
#define TDX_OPERAND_BUSY		0x8000020000000000ULL

uint64_t asm_td_io_outb(uint16_t port, uint8_t value);

static inline void post(uint8_t code)
{
	asm_td_io_outb(QBOOT_POST_PORT, code);
}

static inline void bsp_settle(unsigned count)
{
	unsigned i;

	for (i = 0; i < count; i++)
		asm_td_io_outb(QBOOT_POST_PORT, 0x00);
}

static inline void boot_halt(uint8_t code)
{
	post(code);
	/*
	 * Spin with pause — do not call bsp_settle(), which OUTs 0x00 to
	 * the POST port and would overwrite the fatal code we just emitted.
	 */
	for (;;)
		asm volatile("pause");
}

/* UART init + fingerprint: early TDVMCALL warm-up before TDACCEPT. */
void tdx_io_warmup(void);

#endif /* QBOOT_TDX_H */
