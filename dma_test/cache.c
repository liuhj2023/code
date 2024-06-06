#include <asm/cache.h>
#include <asm/mmio.h>
#include <linux/io.h>
#include "cache.h"

/* TODO: only apply to x280, this is platform specific implementation, move to
 * plat later
 */

#define L3_CACHE_BYTES		L1_CACHE_BYTES

#define L3_CACHE_CTRL_BASE	0x02010000UL
#define L3_CACHE_CONFIG		0x0000
#define L3_CACHE_CMO_CMD	0x0280
#define L3_CACHE_FLUSH_COUNT	0x0288

#define L3_CACHE_CMD_INVAL		0
#define L3_CACHE_CMD_CLEAN		1
#define L3_CACHE_CMD_FLUSH		2
#define L3_CACHE_CMD_OP_OFFSET		0

#define L3_CACHE_CMD_PA			0
#define L3_CACHE_CMD_SETWAY		1
#define L3_CACHE_CMD_TYPE_OFFSET	2

#define L3_CACHE_CMD(addr, op, type)					\
	(								\
	addr |								\
	((unsigned long)(((L3_CACHE_CMD_ ## op)				\
			  << L3_CACHE_CMD_OP_OFFSET) |			\
	  ((L3_CACHE_CMD_ ## type) << L3_CACHE_CMD_TYPE_OFFSET)) << 56)	\
	)

static unsigned char __iomem *l3_cache_ctrl_base = NULL;

static inline void l3_cache_write64(unsigned int offset, uint64_t value)
{
	writeq(value, (void *)(l3_cache_ctrl_base + offset));
}

static inline uint64_t l3_cache_read64(unsigned int offset)
{
	return readq((void *)(l3_cache_ctrl_base + offset));
}


static void wait_l3_cache_flush_done(void)
{
	while (l3_cache_read64(L3_CACHE_FLUSH_COUNT))
		;
}

void invalidate_dcache_range(unsigned long start, unsigned long stop)
{
	unsigned long i = start & ~(L3_CACHE_BYTES - 1);

	for (; i < stop; i += L3_CACHE_BYTES)
		l3_cache_write64(L3_CACHE_CMO_CMD, L3_CACHE_CMD(i, INVAL, PA));

	/* TODO: do invalidate need wait? I donnot think so */
}

void clean_dcache_range(unsigned long start, unsigned long stop)
{
	unsigned long i = start & ~(L3_CACHE_BYTES - 1);

	for (; i < stop; i += L3_CACHE_BYTES)
		l3_cache_write64(L3_CACHE_CMO_CMD, L3_CACHE_CMD(i, CLEAN, PA));

	wait_l3_cache_flush_done();
}

void flush_dcache_range(unsigned long start, unsigned long stop)
{
	unsigned long i = start & ~(L3_CACHE_BYTES - 1);

	for (; i < stop; i += L3_CACHE_BYTES)
		l3_cache_write64(L3_CACHE_CMO_CMD, L3_CACHE_CMD(i, FLUSH, PA));

	wait_l3_cache_flush_done();
}

int x280_cache_init(struct device *dev)
{
	printk("x280 cache ctrl init\n");

	l3_cache_ctrl_base = devm_ioremap(dev, L3_CACHE_CTRL_BASE, 0x1000);
	if (!l3_cache_ctrl_base)
		return -ENOMEM;

	return 0;
}
