#include <asm/bootparam.h>
#include <asm/e820.h>
#include "string.h"
#include "mptable.h"
#include "ioport.h"

struct tdcall_args {
	uint64_t rax;
	uint64_t rcx;
	uint64_t rdx;
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;
	uint64_t r12;
	uint64_t r13;
};

void asm_td_call(struct tdcall_args *args);

static void accept_page(uint64_t page)
{
	struct tdcall_args args;

	memset(&args, 0, sizeof(struct tdcall_args));

	args.rax = 6; // TDCALL_TDACCEPTPAGE
	args.rcx = page;

	asm_td_call(&args);
}

int __attribute__ ((section (".text.startup"))) main(uint64_t cpuid)
{
	struct boot_params *bp = (struct boot_params *) 0x7000;
	uint64_t entry = 0x1000123;
	int i;

	if (cpuid == 0) {
        for (i = 0; i < bp->e820_entries; i++) {
            struct boot_e820_entry entry = bp->e820_table[i];

            if (entry.type != E820_RAM) {
                continue;
            }

            uint64_t addr = entry.addr;
            while (addr < entry.addr + entry.size) {
                accept_page(addr);
                addr += 4096;
            }
        }

		setup_mptable(bp->hdr.root_flags);
	} else {
		entry = bp->scratch;
	}

	asm("xor %rax, %rax");
	asm("mov %0, %%rax"
			: /* a */
			:"r"(entry)
			: "rax");
	asm("xor %rsp, %rsp");
	asm("xor %rbp, %rbp");
	asm("xor %rsi, %rsi");
	asm("mov $0x8ff0, %rsp");
	asm("mov $0x8ff0, %rbp");
	asm("mov $0x7000, %rsi");
	asm("jmpq *%rax");

	// Not reached.
	return 0;
}
