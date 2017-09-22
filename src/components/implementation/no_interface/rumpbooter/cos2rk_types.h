#ifndef COS2RK_TYPES_H
#define COS2RK_TYPES_H

#include "vk_types.h"

#define HW_ISR_LINES 32
#define HW_ISR_FIRST 1

/* The amount of memory to give RK to start in bytes */
#define RK_TOTAL_MEM (1<<26) //64MB

#define RK_IRQ_IO 15

#define IRQ_DOM0_VM 22 /* DOM0's message line to VM's, in VM's */
#define IRQ_VM1 21     /* VM1's message line to DOM0, so in DOM0 */
#define IRQ_VM2 27     /* VM2's message line to DOM0, so in DOM0 */

#define COS2RK_MEM_SHM_BASE	VK_VM_SHM_BASE
#define COS2RK_VIRT_MACH_COUNT	VM_COUNT
#define COS2RK_VIRT_MACH_MEM_SZ(vmid) VM_UNTYPED_SIZE(vmid) //128MB
#define COS2RK_SHM_VM_SZ	VM_SHM_SZ //4MB
#define COS2RK_SHM_ALL_SZ	VM_SHM_ALL_SZ

#undef PRINT_CPU_USAGE
#define MIN_CYCS (1<<12)

#define BOOTUP_ITERS 100

enum vm_prio {
	PRIO_HIGH = TCAP_PRIO_MAX,
	PRIO_MID  = TCAP_PRIO_MAX + 1,
	PRIO_LOW  = TCAP_PRIO_MAX + 2,
};

extern unsigned int cycs_per_usec;
extern u64_t t_vm_cycs;
extern u64_t t_dom_cycs;

#endif /* COS2RK_TYPES_H */
