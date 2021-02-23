/* 
 * copied from https://github.com/WenyuanShao/eos/blob/fwp/src/components/lib/musl-1.1.11/include/pci.h 
 */

#ifndef PCI_H
#define PCI_H

#define PCI_BUS_MAX        255
#define PCI_DEVICE_MAX     32
#define PCI_FUNC_MAX       7
#define PCI_DEVICE_NUM     128
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC
#define PCI_DATA_NUM       0x10
#define PCI_BAR_NUM        6
#define PCI_BITMASK_32     0xFFFFFFFF

/* /1* pci CONFIG_ADDRESS register format *1/ */
/* struct pci_addr_layout { */
/* 	uint32 regNum:8; */
/* 	uint32 funcNum:3; */
/* 	uint32 devNum:5; */
/* 	uint32 busNum:8; */
/* 	uint32 reserved:7; */
/* 	uint32 enable:1; */
/* }; */
#define PCI_ADDR(bus, dev, func, reg) ((1 << 31) | ((bus) << 16) | ((dev) << 11) | ((func) << 8) | (reg))
#define PCI_VENDOR_ID(v)   ((v) & 0xFFFF)
#define PCI_DEVICE_ID(v)   (((v) >> 16) & 0xFFFF)
#define PCI_CLASS_ID(v)    (((v) >> 24) & 0xFF)
#define PCI_SUBCLASS_ID(v) (((v) >> 16) & 0xFF)
#define PCI_PROG_IF(v)     (((v) >> 8) & 0xFF)
#define PCI_HEADER(v)      (((v) >> 16) & 0xFF)

/* Data Structures */

struct pci_bar {
	union {
		u32_t raw;
		struct ioBAR {
			u32_t setBit:1;
			u32_t reserved:1;
			u32_t ioPort:30;
		} __attribute__((packed)) io;
		struct memBAR {
			u32_t clrBit:1;
			u32_t memType:2;
			u32_t prefetch:1;
			u32_t baseAddr:28;
		} __attribute__((packed)) mem;
	};
	u32_t mask;
	/* LED: add phys_addr */
	u64_t phys_addr;
} __attribute__((packed));

struct pci_dev {
	u32_t bus, dev, func;
	u16_t vendor;
	u16_t device;
	u8_t  classcode;
	u8_t  subclass;
	u8_t  progIF;
	u8_t  header;
	struct pci_bar bar[PCI_BAR_NUM];
	u32_t data[PCI_DATA_NUM];
	unsigned int index;
	void *drvdata;
} __attribute__((packed));

/* API */

int pci_scan(struct pci_dev *devices, int sz);
void pci_dev_print(struct pci_dev *devices, int sz);
u32_t pci_config_read(u32_t bus, u32_t dev, u32_t func, u32_t reg);
void pci_config_write(u32_t bus, u32_t dev, u32_t func, u32_t reg, u32_t v);
int pci_dev_get(struct pci_dev *devices, int sz, struct pci_dev *dev, u16_t dev_id, u16_t vendor_id);

int pci_num(void);

#endif /* PCI_H */