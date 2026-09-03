#include "tdx.h"

static void tdx_putc(char c)
{
	if (c == '\n')
		tdx_putc('\r');

	/*
	 * Do not poll LSR via TDVMCALL IN — if the host never completes
	 * reads, that loop hangs the TD. Fire-and-forget OUT is enough.
	 */
	asm_td_io_outb(QBOOT_SERIAL_PORT, (uint8_t)c);
}

static void tdx_puts(const char *s)
{
	if (!s)
		return;
	while (*s)
		tdx_putc(*s++);
}

void tdx_io_warmup(void)
{
	/* 115200 8N1 on COM1 — each OUT is a host exit / timing wedge. */
	asm_td_io_outb(QBOOT_SERIAL_PORT + 1, 0x00); /* IER */
	asm_td_io_outb(QBOOT_SERIAL_PORT + 3, 0x80); /* LCR: DLAB on */
	asm_td_io_outb(QBOOT_SERIAL_PORT + 0, 0x01); /* DLL: 115200 */
	asm_td_io_outb(QBOOT_SERIAL_PORT + 1, 0x00); /* DLM */
	asm_td_io_outb(QBOOT_SERIAL_PORT + 3, 0x03); /* LCR: 8N1 */
	asm_td_io_outb(QBOOT_SERIAL_PORT + 2, 0xc7); /* FCR */
	asm_td_io_outb(QBOOT_SERIAL_PORT + 4, 0x0b); /* MCR */

	/*
	 * Short line so the host path sees a burst of OUTs before accept.
	 * Also a cheap signal that this firmware image is actually running.
	 */
	tdx_puts("QBOOT-FW madt-mailbox post-always\n");
}
