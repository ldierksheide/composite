#include <cos_component.h>
#include <llprint.h>
#include <pci.h>

struct pci_dev all_dev[6] = {
		{
			.bus = 0,	
			.dev = 0,	
			.func = 0,	
			.vendor = 0x8086,	
			.device = 0x1237,	
			.classcode = 6,	
			.subclass = 0,	
			.progIF = 0,	
			.header = 0,	
			.bar = 0, 
			.data = 0,	
			.index = 0,	
			.drvdata = 0
		},
		{
			.bus = 0,	
			.dev = 1,	
			.func = 0,	
			.vendor = 0x8086,	
			.device = 0x7000,	
			.classcode = 6,	
			.subclass = 1,	
			.progIF = 0,	
			.header = 0x80,	
			.bar = 0, 
			.data = 0,	
			.index = 0,	
			.drvdata = 0
		},
		{
			.bus = 0,	
			.dev = 1,	
			.func = 1,	
			.vendor = 0x8086,	
			.device = 0x7010,	
			.classcode = 1,	
			.subclass = 1,	
			.progIF = 0x80,	
			.header = 0,	
			.bar = 0, 
			.data = 0,	
			.index = 0,	
			.drvdata = 0
		},
		{
			.bus = 0,	
			.dev = 1,	
			.func = 3,	
			.vendor = 0x8086,	
			.device = 0x7113,	
			.classcode = 6,	
			.subclass = 0x80,	
			.progIF = 0,	
			.header = 0,	
			.bar = 0, 
			.data = 0,	
			.index = 0,	
			.drvdata = 0
		},
		{
			.bus = 0,	
			.dev = 2,	
			.func = 0,	
			.vendor = 0x1234,	
			.device = 0x1111,	
			.classcode = 3,	
			.subclass = 0,	
			.progIF = 0,	
			.header = 0,	
			.bar = 0, 
			.data = 0,	
			.index = 0,	
			.drvdata = 0
		},
		{
			.bus = 0,	
			.dev = 3,	
			.func = 0,	
			.vendor = 0x8086,	
			.device = 0x100e,	
			.classcode = 2,	
			.subclass = 0,	
			.progIF = 0,	
			.header = 0,	
			.bar = 0, 
			.data = 0,	
			.index = 0,	
			.drvdata = 0
		}
	};

void
cos_init(void)
{

}

int
main(void)
{
	int size;
	int i;
	size = pci_num();
	struct pci_dev my_devices[size];
	
	pci_scan(my_devices, size);
	for(i = 0 ; i < size ; i++) {
		if(my_devices[i].bus != all_dev[i].bus) {
			printc("Failure: got %x instead of %x for devices[%d] bus\n", my_devices[i].bus, all_dev[i].bus, i);
			return 0;
		}
		if(my_devices[i].dev != all_dev[i].dev) {
			printc("Failure: got %x instead of %x for devices[%d] dev\n", my_devices[i].dev, all_dev[i].dev, i);
			return 0;

		}
		else if(my_devices[i].func != all_dev[i].func) {
			printc("Failure: got %x instead of %x for devices[%d] func\n", my_devices[i].func, all_dev[i].func, i);
			return 0;		
		}
		else if(my_devices[i].vendor != all_dev[i].vendor) {
			printc("Failure: got %x instead of %x for devices[%d] vendor\n", my_devices[i].vendor, all_dev[i].vendor, i);
			return 0;		
		}
		else if(my_devices[i].device != all_dev[i].device) {
			printc("Failure: got %x instead of %x for devices[%d] device\n", my_devices[i].device, all_dev[i].device, i);
			return 0;		
		}
		else if(my_devices[i].classcode != all_dev[i].classcode) {
			printc("Failure: got %x instead of %x for devices[%d] classcode\n", my_devices[i].classcode, all_dev[i].classcode, i);
			return 0;
		}
		else if(my_devices[i].subclass != all_dev[i].subclass) {
			printc("Failure: got %x instead of %x for devices[%d] subclass\n", my_devices[i].subclass, all_dev[i].subclass, i);
			return 0;
		}
		else if(my_devices[i].progIF != all_dev[i].progIF) {
			printc("Failure: got %x instead of %x for devices[%d] progIF\n", my_devices[i].progIF, all_dev[i].progIF, i);
			return 0;
		}
		else if(my_devices[i].header != all_dev[i].header) {
			printc("Failure: got %x instead of %x for devices[%d] header\n", my_devices[i].header, all_dev[i].header, i);
			return 0;
		}
	}
	printc("Success\n");

	while(1);
	return 0;
}
