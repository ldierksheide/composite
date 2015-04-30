#define ENABLE_SERIAL
#define ENABLE_TIMER

#include "assert.h"
#include "kernel.h"
#include "multiboot.h"
#include "string.h"
#include "comp.h"

#include <captbl.h>
#include <retype_tbl.h>
#include <component.h>
#include <thd.h>
#include <inv.h>
#include <mmap.h>

void
khalt(void)
{
	printk("Shutting down...\n");
	asm("mov $0x53,%ah");
	asm("mov $0x07,%al");
	asm("mov $0x001,%bx");
	asm("mov $0x03,%cx");
	asm("int $0x15");
}

void
multiboot_validate(struct multiboot *mb, u32_t mboot_magic)
{
	struct multiboot_mod_list *mods;
	struct multiboot_mem_list *mems;
	unsigned int i;

	if (mboot_magic != MULTIBOOT_EAX_MAGIC) {
		die("Not started from a multiboot loader!\n");
	}
	if ((mb->flags & MULTIBOOT_FLAGS_REQUIRED) != MULTIBOOT_FLAGS_REQUIRED) {
		die("Multiboot flags include %x but are missing one of %x\n", 
		    mb->flags, MULTIBOOT_FLAGS_REQUIRED);
	}

	mods = (struct multiboot_mod_list *)mb->mods_addr;
	mems = (struct multiboot_mem_list *)mb->mmap_addr;

	printk("System info (from multiboot):\n");
	printk("\tModules:\n");
	for (i = 0 ; i < mb->mods_count ; i++) {
		struct multiboot_mod_list *mod = &mods[i];
		
		printk("\t- %d: [%08x, %08x)\n", i, mod->mod_start, mod->mod_end);
	}
	printk("\tMemory regions:\n");
	for (i = 0 ; i < mb->mmap_length/sizeof(struct multiboot_mem_list) ; i++) {
		struct multiboot_mem_list *mem = &mems[i];
		
		printk("\t- %d (%s): [%08llx, %08llx)\n", i, 
		       mem->type == 1 ? "Available" : "Reserved ", mem->addr, mem->addr + mem->len);
	}
}

void 
kmain(struct multiboot *mboot, u32_t mboot_magic, u32_t esp)
{
	int bc = 0, ret;

	tss_init();
	gdt_init();
	idt_init();

#ifdef ENABLE_SERIAL
	serial_init();
#endif

#ifdef ENABLE_CONSOLE
	console_init();
#endif

#ifdef ENABLE_TIMER
	timer_init(100);
#endif	    
	multiboot_validate(mboot, mboot_magic);

	cos_init_memory();
	cap_init();
       	ltbl_init();
       	retype_tbl_init();
       	comp_init();
       	thd_init();
       	inv_init();

	if (mboot->mods_count != 1) {
		die("Boot failure: expecting a single module to load, received %d instead.\n", mboot->mods_count);
	}
	paging_init((struct multiboot_mod_list *)mboot->mods_addr);

	ret = kern_boot_comp((unsigned long)mboot->size);
	assert(ret == 0);

	user_init();
	khalt(); 
}