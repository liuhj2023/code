// SPDX-License-Identifier: GPL-2.0
#include <linux/types.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include "cache.h"

#define DRV_NAME	"dma-test"
#define MEM_TEST_SIZE	0x2000
#define GDMA_NUM	1
#define PATTERN		0x5f

struct dma_test_dev {
	unsigned char __iomem	*membase;
	unsigned int		irq;
};

static struct dma_test_dev dma_test;

static int dma_copy(dma_addr_t dest, dma_addr_t src, size_t size, int index)
{
	/*TODO*/
	return 0;
}

static int dma_test_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct device *dev = &pdev->dev;
	dma_addr_t dma_addr_src, dma_addr_dest;
	void *va_src, *va_dest;
	int gdma_index;
	size_t i;

	if (x280_cache_init(dev))
		return -ENOMEM;

	printk(KERN_INFO "dma test probe\n");

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	dma_test.membase = devm_ioremap_resource(dev, res);
	if (IS_ERR(dma_test.membase))
		return PTR_ERR(dma_test.membase);

	if (dma_set_mask_and_coherent(dev, DMA_BIT_MASK(40))) {
		printk("cannot use 40 bit DMA\n");
		return -ENODEV;
	}

	va_src = dma_alloc_coherent(dev, MEM_TEST_SIZE, &dma_addr_src, GFP_KERNEL);
	if (!va_src) {
		printk("dma alloc error\n");
		return -ENOMEM;
	}

	va_dest = dma_alloc_coherent(dev, MEM_TEST_SIZE, &dma_addr_dest, GFP_KERNEL);
	if (!va_dest) {
		printk("dma alloc error\n");
		dma_free_coherent(dev, MEM_TEST_SIZE, va_src, dma_addr_src);
		return -ENOMEM;
	}

	for (gdma_index = 0; gdma_index < GDMA_NUM; gdma_index++) {
		memset(va_src, PATTERN, MEM_TEST_SIZE);
		memset(va_dest, 0x0, MEM_TEST_SIZE);

		flush_dcache_range(pfn_to_phys(vmalloc_to_pfn(va_src)),
				   pfn_to_phys(vmalloc_to_pfn(va_src)) + MEM_TEST_SIZE);

		flush_dcache_range(pfn_to_phys(vmalloc_to_pfn(va_dest)),
					       pfn_to_phys(vmalloc_to_pfn(va_dest)) + MEM_TEST_SIZE);
#if 1
		dma_copy(dma_addr_dest, dma_addr_src, MEM_TEST_SIZE, gdma_index);
#else
		memcpy(va_dest, va_src, MEM_TEST_SIZE);
#endif

		for (i = 0; i < MEM_TEST_SIZE; i++) {
			if (((unsigned char *)va_dest)[i] != PATTERN) {
				printk("gdma%d copy error\n", gdma_index);
				break;
			}
		}
		printk("gdma%d copy success\n", gdma_index);
	}

	dma_free_coherent(dev, MEM_TEST_SIZE, va_src, dma_addr_src);
	dma_free_coherent(dev, MEM_TEST_SIZE, va_dest, dma_addr_dest);

	return 0;
}

static int dma_test_remove(struct platform_device *pdev)
{
	printk(KERN_INFO "dma test remove\n");

	return 0;
}

static const struct of_device_id dma_test_of_match[] = {
        { .compatible = "dma-test", },
        { }
};
MODULE_DEVICE_TABLE(of, dma_test_of_match);

static struct platform_driver dma_test_driver = {
	.probe	= dma_test_probe,
	.remove	= dma_test_remove,
	.driver	= {
		.name    = DRV_NAME,
		.of_match_table = dma_test_of_match,
	},
};

static int __init dma_test_init(void)
{
	printk(KERN_INFO "dma test init\n");
	platform_driver_register(&dma_test_driver);

	return 0;
}

static void __exit dma_test_exit(void)
{
	printk(KERN_INFO "dma test exit\n");
	platform_driver_unregister(&dma_test_driver);
}

module_init(dma_test_init);
module_exit(dma_test_exit);

MODULE_DESCRIPTION("dma test driver");
MODULE_LICENSE("GPL v2");
