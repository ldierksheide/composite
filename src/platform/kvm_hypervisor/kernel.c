#include "assert.h"
#include "kernel.h"
#include "multiboot.h"
#include "string.h"
#include "boot_comp.h"
#include "mem_layout.h"
#include "chal_cpu.h"

#include <captbl.h>
#include <retype_tbl.h>
#include <component.h>
#include <thd.h>

#include "hypercall_opcodes.h"

#define ADDR_STR_LEN 8

boot_state_t initialization_state = INIT_BOOTED;

void
boot_state_transition(boot_state_t from, boot_state_t to)
{
	assert(initialization_state == from);
	assert((to - from) == 1); /* transitions must be linear */
	initialization_state = to;
}

struct mem_layout glb_memlayout;
volatile int cores_ready[NUM_CPU];

extern u8_t end, end_all; /* from the linker script */

#define MEM_KB_ONLY(x) (((x) & ((1 << 20) - 1)) >> 10)
#define MEM_MB_ONLY(x) ((x) >> 20)

extern u8_t _binary_constructor_start, _binary_constructor_end;

static void
print(const char* msg) 
{
	struct hyp_shared *hyp_print;
	
	hyp_print = (struct hyp_shared*)(COS_MEM_KERN_START_VA);
	hyp_print->opcode = HYP_printk;
	hyp_print->args[1] = strnlen(msg, 2048);
	hyp_print->args[0] = (u32_t*)(msg - (char*)(COS_MEM_KERN_START_VA));

	asm("out %0,%1" : /* empty */ : "a" (msg), "Nd" (0xE9) : "memory");	
}

u32_t
get_phys_mem(void)
{
	struct hyp_shared *hyp_physmem;
	const char *msg = "";
	
	hyp_physmem = (struct hyp_shared*)(COS_MEM_KERN_START_VA);
	hyp_physmem->opcode = HYP_physmem;

	/* trigger hypercall */
	asm("out %0,%1" :  /*empty*/  : "a" (msg), "Nd" (0xE9) : "memory");	

	if(hyp_physmem->ret[0] != 0) {
		return -1;
	}

	return hyp_physmem->ret[1];
}

void
kern_memory_setup(void)
{
	u32_t size_physmem;

	glb_memlayout.kern_end = &end + sizeof(end);

	/* Booter : 64MB, these are kernel addresses that will never be used in fact */
	glb_memlayout.mod_start 	= &_binary_constructor_start;
	glb_memlayout.mod_end   	= &_binary_constructor_end;
	printk("[%08x, %08x)\n", &_binary_constructor_start, &_binary_constructor_end);

	/* Requires that bss segment is after initial component */	
	glb_memlayout.kern_boot_heap = mem_boot_start(); 

	glb_memlayout.allocs_avail   = 1;

	chal_kernel_mem_pa 			 = chal_va2pa(mem_kmem_start());

	if((phys_mem = get_phys_mem()) < 0) {
		printk("Failed to get physical memory size\n");
		return;  
	}

	glb_memlayout.kmem_end 		 = (u8_t*)(phys_mem + COS_MEM_KERN_START_VA);

	/* Validate the memory layout */
	assert(mem_kmem_end() >= mem_boot_end());
	assert(mem_utmem_start() >= mem_kmem_start());
	assert(mem_utmem_start() >= mem_boot_end());
	assert(mem_utmem_end() <= mem_kmem_end());

}

void
kmain(struct multiboot *mboot, u32_t mboot_magic, u32_t esp)
{
	/* Register printk_handler */
	void (*print_ptr)(const char*) = &print;
	printk_register_handler(print_ptr);

	const char *p;

#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
	unsigned long max;

	tss_init(INIT_CORE);
	gdt_init(INIT_CORE);
	idt_init(INIT_CORE);

#ifdef ENABLE_SERIAL
	//serial_init();
#endif
#ifdef ENABLE_CONSOLE
	//console_init();
#endif
#ifdef ENABLE_VGA
	//vga_init();
#endif
	boot_state_transition(INIT_BOOTED, INIT_CPU);

	max = MAX((unsigned long)mboot->mods_addr,
	          MAX((unsigned long)mboot->mmap_addr, (unsigned long)(chal_va2pa(&end))));
	kern_paging_map_init((void *)(max + PGD_SIZE));
	kern_memory_setup();
	boot_state_transition(INIT_CPU, INIT_MEM_MAP);

	chal_init();	
	cap_init();
	ltbl_init();
	retype_tbl_init();
	comp_init();
	thd_init();
	boot_state_transition(INIT_MEM_MAP, INIT_DATA_STRUCT);

	paging_init();
	boot_state_transition(INIT_DATA_STRUCT, INIT_UT_MEM);

	/* Have not set these up yet */
	//acpi_init();
	//lapic_init();
	//timer_init();
	boot_state_transition(INIT_UT_MEM, INIT_KMEM);

	kern_boot_comp(INIT_CORE);
	printk("got through kern_boot_comp()\n");

	//smp_init(cores_ready);
	cores_ready[INIT_CORE] = 1;


	kern_boot_upcall();

	/* should not get here... */
	khalt();

}

void
smp_kmain(void)
{
	volatile cpuid_t cpu_id = get_cpuid();
	struct cos_cpu_local_info *cos_info = cos_cpu_local_info();

	printk("Initializing CPU %d\n", cpu_id);
	tss_init(cpu_id);
	gdt_init(cpu_id);
	idt_init(cpu_id);

	chal_cpu_init();
	kern_boot_comp(cpu_id);
	lapic_init();

	printk("New CPU %d Booted\n", cpu_id);
	cores_ready[cpu_id] = 1;
	/* waiting for all cored booted */
	while(cores_ready[INIT_CORE] == 0);

	kern_boot_upcall();

	while(1) ;
}

extern void shutdown_apm(void);
extern void outw(unsigned short __val, unsigned short __port);

void
khalt(void)
{
	static int method = 0;

	if (method == 0) printk("Shutting down...\n");
	/*
	 * Use the case statement as we shutdown in the fault handler,
	 * thus faults on shutdown require that we bypass faulty
	 * shutdown handlers.
	 */
	switch(method) {
	case 0:
		method++;
		printk("\ttry acpi");
		acpi_shutdown();
		printk("...FAILED\n");
	case 1:
		method++;
		printk("\ttry apm");
		shutdown_apm();
		printk("...FAILED\n");
	case 2:
		method++;
		printk("\t...try emulator magic");
		outw(0xB004, 0x0 | 0x2000);
		printk("...FAILED\n");
	}
	/* last resort */
	printk("\t...spinning\n");
	while (1) ;
}