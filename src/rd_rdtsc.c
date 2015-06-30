/*
 * Copyright 2011-2013 AppNexus, Inc.
 *
 * This is unpublished proprietary source code of AppNexus, Inc.
 * The copyright notice above does not evidence any actual or
 * intended publication of such source code.
 *
 * Redistribution of this material is strictly prohibited.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include "rd_rdtsc.h"

#ifndef RD_CALIBRATE_FREQUENCY
#define RD_CALIBRATE_FREQUENCY 300
#endif

enum facilities {
	RD_RDTSC,
	RD_LENGTH
};

struct rd_table {
	const char *resource;
	const char *implementation;
	const char *notes;
};

static struct rd_table table[] = {
	[RD_RDTSC] = {
		.resource = "rdtsc",
		.implementation = NULL
	}
};

static double rdtsc_scale;

#define MIN_GETTIME_SAMPLES   50
#define MAX_GETTIME_SAMPLES   100000
#define GETTIME_STDERR_TARGET 850.0

#define MIN_SLEEP_SAMPLES     50
#define MAX_SLEEP_SAMPLES     1000
#define SLEEP_STDERR_TARGET   50.0

#define MIN_STDERR_BOUNDED    5

#define max(A,B) (A<B?B:A)

double
rd_rdtsc_scale(uint64_t ticks)
{

	return (double)ticks / rdtsc_scale;
}

unsigned long long
rd_us_to_rdtsc(unsigned long long us)
{

	return us * rdtsc_scale;
}

unsigned long long
rd_rdtsc_to_us(uint64_t ticks)
{

	return (unsigned long long)llround(rd_rdtsc_scale(ticks));
}

static bool
rd_rdtsc_calibrate_scale(rd_rdtsc_t *rdtsc)
{
	int i, j, k;
	uint64_t start, end;
	double gettime_sum, gettime_sum_sq, gettime_mean, gettime_std_err,
	       rdtsc_sum, rdtsc_sum_sq, rdtsc_mean, rdtsc_std_err, diff;

	gettime_sum = 0.0;
	gettime_sum_sq = 0.0;
	gettime_mean = 0.0;
	gettime_std_err = 0.0;
	rdtsc_mean = 0.0;
	rdtsc_std_err = 0.0;

	for (i = 1, j = k = 0; i <= MAX_GETTIME_SAMPLES; ++i) {
		struct timespec ts;

		start = rdtsc();
		clock_gettime(CLOCK_MONOTONIC, &ts);
		end = rdtsc();
		if (end < start)
			continue;
		++j;
		diff = end - start;
		gettime_sum += diff;
		gettime_sum_sq += diff * diff;
		if (i >= MIN_GETTIME_SAMPLES) {
			gettime_mean = gettime_sum / j;
			gettime_std_err = (gettime_sum_sq - gettime_sum * gettime_sum / j) / (j - 1);
			gettime_std_err = sqrt(max(gettime_std_err, 0.0));
			if (gettime_std_err > GETTIME_STDERR_TARGET)
				k = 0;
			else if (++k >= MIN_STDERR_BOUNDED)
				break;
		}
	}

	rdtsc_sum = 0.0;
	rdtsc_sum_sq = 0.0;
	for (i = 1, j = k = 0; i <= MAX_SLEEP_SAMPLES; ++i) {
		struct timespec start_ts, end_ts,
			sleep_ts = { 
				.tv_sec = 0,
				.tv_nsec = 1000 * 1000 * ((i % 10) + 1), 
			};

		start = rdtsc();
		clock_gettime(CLOCK_MONOTONIC, &start_ts);
		nanosleep(&sleep_ts, NULL);
		clock_gettime(CLOCK_MONOTONIC, &end_ts);
		end = rdtsc();
		diff = 1.0e+6 * (end_ts.tv_sec - start_ts.tv_sec)
				+ 1.0e-3 * (end_ts.tv_nsec - start_ts.tv_nsec);
		if (diff <= 0.0)
			continue;
		++j;
		diff = ((double)(end - start) - gettime_mean) / diff;
		rdtsc_sum += diff;
		rdtsc_sum_sq += diff * diff;
		if (j >= MIN_SLEEP_SAMPLES) {
			rdtsc_mean = rdtsc_sum / j;
			rdtsc_std_err = (rdtsc_sum_sq - rdtsc_sum * rdtsc_sum / j) / (j - 1);
			rdtsc_std_err = sqrt(max(rdtsc_std_err, 0.0));
			if (rdtsc_std_err > SLEEP_STDERR_TARGET)
				k = 0;
			else if (++k >= MIN_STDERR_BOUNDED)
				break;
		}
	}

	if (rdtsc_std_err > SLEEP_STDERR_TARGET) {
		return false;
	} else if (rdtsc_scale != 0) {
		double drift = rdtsc_mean - rdtsc_scale;

	}

	rdtsc_scale = rdtsc_mean;
	if (table[RD_RDTSC].notes != NULL) {
		free((char *)table[RD_RDTSC].notes);
		table[RD_RDTSC].notes = NULL;
	}

	asprintf((char **)&table[RD_RDTSC].notes, "%lf ticks/us, deviation %lf, %d iterations",
	        rdtsc_scale, rdtsc_std_err, j);

	return true; 
}

static uint64_t
rd_rdtsc_gettime(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

void
rd_rdtsc_calibrate(void)
{
	rd_rdtsc_calibrate_scale(rd_rdtsc);
	return;
}

void
rd_rdtsc_probe(void)
{
	rd_rdtsc_t *r;

	r = rd_probe_rdtsc(&table[RD_RDTSC].implementation);
	if (r != NULL && rd_rdtsc_calibrate_scale(r) == true) {
		rd_rdtsc = r;
	}

	if (rd_rdtsc == NULL) {
		rdtsc_scale = 1000.0;
		rd_rdtsc = rd_rdtsc_gettime;
		table[RD_RDTSC].implementation = "clock_gettime(CLOCK_MONOTONIC)";
	}

	return;
}

