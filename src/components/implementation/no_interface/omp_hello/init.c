#include <cos_kernel_api.h>
#include <cos_defkernel_api.h>
#include <llprint.h>
#include <sl.h>
#include <cos_omp.h>
#include <hypercall.h>

int main(void);

void
cos_exit(int x)
{
	PRINTC("Exit code: %d\n", x);
	while (1) ;
}

static void
cos_main(void *d)
{
	main();

	while (1) ;
}

extern void cos_gomp_init(void);

void
cos_init(void *d)
{
	struct cos_defcompinfo *defci = cos_defcompinfo_curr_get();
	struct cos_compinfo *   ci    = cos_compinfo_get(defci);
	int i;
	static volatile unsigned long first = NUM_CPU + 1, init_done[NUM_CPU] = { 0 };

	PRINTC("In an OpenMP program!\n");
	if (ps_cas((unsigned long *)&first, NUM_CPU + 1, cos_cpuid())) {
		cos_meminfo_init(&(ci->mi), BOOT_MEM_KM_BASE, COS_MEM_KERN_PA_SZ, BOOT_CAPTBL_SELF_UNTYPED_PT);
		cos_defcompinfo_init();
	} else {
		while (!ps_load((unsigned long *)&init_done[first])) ;

		cos_defcompinfo_sched_init();
	}
	ps_faa((unsigned long *)&init_done[cos_cpuid()], 1);

	/* make sure the INITTHD of the scheduler is created on all cores.. for cross-core sl initialization to work! */
	for (i = 0; i < NUM_CPU; i++) {
		while (!ps_load((unsigned long *)&init_done[i])) ;
	}
	sl_init(SL_MIN_PERIOD_US*100);
	cos_gomp_init();
	hypercall_comp_init_done();

	if (!cos_cpuid()) {
		struct sl_thd *t = NULL;

		t = sl_thd_alloc(cos_main, NULL);
		assert(t);
		sl_thd_param_set(t, sched_param_pack(SCHEDP_PRIO, TCAP_PRIO_MAX));
	}

	sl_sched_loop_nonblock();

	PRINTC("Should never get here!\n");
	assert(0);
}

