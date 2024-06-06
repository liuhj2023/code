#ifndef __X280_CACHE_CTRL_H_
#define __X280_CACHE_CTRL_H_

int x280_cache_init(struct device *dev);
void invalidate_dcache_range(unsigned long start, unsigned long stop);
void clean_dcache_range(unsigned long start, unsigned long stop);
void flush_dcache_range(unsigned long start, unsigned long stop);

#endif
