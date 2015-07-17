/*
 * Copyright 2011-2013 AppNexus, Inc.
 *
 * This is unpublished proprietary source code of AppNexus, Inc.
 * The copyright notice above does not evidence any actual or
 * intended publication of such source code.
 *
 * Redistribution of this material is strictly prohibited.
 */

#ifndef _RD_TSC_H
#define _RD_TSC_H

#include <stdbool.h>
#include <stdint.h>

#ifndef __x86_64__
#error "Unsupported platform."
#endif

typedef uint64_t rd_rdtsc_t(void);
rd_rdtsc_t *rd_probe_rdtsc(const char **);

void rd_rdtsc_probe(void);

/*
 * Public functions.
 */

rd_rdtsc_t *rd_rdtsc;

/**
 * Returns number of microseconds from ticks.
 */
unsigned long long rd_us_to_rdtsc(unsigned long long);

/**
 * Returns number of microseconds in a tick interval.
 * Provides microsecond granularity;
 */
uint64_t rd_rdtsc_scale(uint64_t);

/**
 * Returns number of microseconds in a tick interval
 * in integer format. The number of microseconds is
 * rounded up to the nearest microsecond.
 *
 * For example,
 *   0.50 microseconds -> 1 microsecond
 *   0.49 microseconds -> 0 microseconds
 */
unsigned long long rd_rdtsc_to_us(uint64_t);

void rd_rdtsc_calibrate(void);

#endif /* _AN_MD_H */
