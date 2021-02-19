#include <cos_component.h>
#include <llprint.h>
#include <pci.h>

void
cos_init(void)
{
	printc("Hello world!\n");

	//while (1) ;
}

int
main(void)
{
	struct pci_dev my_dev;

	u16_t dev_id = 0x1237;
	u16_t vendor_id = 0x8086;
	int classcode = 6;

	int size = 8;
	int i;

	struct pci_dev my_devices[size];

	pci_scan(my_devices, size);
	pci_print(my_devices, size);

	if(pci_get_dev(my_devices, size, &my_dev, dev_id, vendor_id) != 0) {
		printc("get dev id failed\n");
	}

	if(my_dev.device != dev_id && my_dev.classcode != classcode) {
		printc("filling my_dev failed\n");
	}

	printc("number of devices = %d\n", pci_num_dev());

	while(1);
	return 0;
}
