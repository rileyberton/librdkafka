/*
 * Copyright 2011-2013 AppNexus, Inc.
 *
 * This is unpublished proprietary source code of AppNexus, Inc.
 * The copyright notice above does not evidence any actual or
 * intended publication of such source code.
 *
 * Redistribution of this material is strictly prohibited.
 */

#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#include "../rd_rdtsc.h"
#include "cpuid.h"

static uint64_t
rd_x86_64_i_rdtsc(void)
{
	uint32_t eax = 0, edx;

	__asm__ __volatile__("cpuid;"
			     "rdtsc;"
			        : "+a" (eax), "=d" (edx)
				:
				: "%ecx", "%ebx", "memory");

	__asm__ __volatile__("xorl %%eax, %%eax;"
			     "cpuid;"
				:
				:
				: "%eax", "%ebx", "%ecx", "%edx", "memory");

	return (((uint64_t)edx << 32) | eax);
}

static uint64_t
rd_x86_64_i_rdtscp(void)
{
	uint32_t eax = 0, edx;

	__asm__ __volatile__("rdtscp"
				: "+a" (eax), "=d" (edx)
				:
				: "%ecx", "memory");

	return (((uint64_t)edx << 32) | eax);
}

rd_rdtsc_t *
rd_probe_rdtsc(const char **description)
{

	/*
	 * Intel processors typically have a synchronized clock across
	 * sockets. This is not the case for AMD, so we cannot rely on
	 * it as a timer source.
	 */
	if (cpuid_vendor() != CPUID_VENDOR_INTEL)
		return NULL;

	if (cpuid_feature(CPUID_FEATURE_RDTSCP) == true) {
		*description = "rdtscp";
		return rd_x86_64_i_rdtscp;
	}

	if (cpuid_feature(CPUID_FEATURE_TSC) == true) {
		*description = "rdtsc";
		return rd_x86_64_i_rdtsc;
	}

	return NULL;
}

