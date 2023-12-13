/*
 * arch/arm/plat-rda/cpuidle.c
 *
 * CPU idle driver for RDAmicro CPUs
 *
 * Copyright (c) 2014 RDA Micro, Inc.
 * Author: yingchunli<yingchunli@rdamicro.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cpu.h>
#include <linux/cpuidle.h>
#include <linux/hrtimer.h>
#include <asm/proc-fns.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <asm/cpuidle.h>
#include <asm/suspend.h>
#include <asm/io.h>

#include <rda/mach/iomap.h>
#include "ap_clk.h"
#include <rda/plat/intc.h>
#include <rda/plat/pm_ddr.h>
#include <rda/mach/board.h>

static void __iomem * gpu_base;

#if !defined(CONFIG_MACH_RDA8810E) && !defined(CONFIG_MACH_RDA8810H)
static DEFINE_PER_CPU(struct cpuidle_device, rda_idle_device);

static int rda_idle_enter_lp1(struct cpuidle_device *dev, struct cpuidle_driver *drv, int index);

#ifdef CONFIG_MACH_RDA8810
struct cpuidle_driver rda_idle_driver = {
	.name = "rda_idle",
	.owner = THIS_MODULE,
	.state_count = 2,
	.states = {
		[0] = ARM_CPUIDLE_WFI_STATE,
		[1] = {
			.enter			= rda_idle_enter_lp1,
			.exit_latency		= 200,
			.target_residency	= 1000,
			.power_usage		= 400,
			.name			= "LP1",
			.desc			= "CPU pll off",
		},
		
	},
};


#endif



static int rda_idle_enter_lp1(struct cpuidle_device *dev,
	struct cpuidle_driver *drv, int index)
{
	struct cpufreq_policy *policy;
	unsigned long old_freq;
	unsigned long min;

	policy = cpufreq_cpu_get(0);
	if (!policy)
		return index;

	/* setting cpu freq to the minist*/
	old_freq = policy->cur;
	min = policy->min;
	if (min != old_freq) apsys_adjust_cpu_clk_rate(min * 1000);

	apsys_cpupll_switch(0);
	cpu_do_idle();
	apsys_cpupll_switch(1);
	udelay(10);

	/*restore cpufreq if needed */
	if (old_freq != min) apsys_adjust_cpu_clk_rate(old_freq * 1000);
	cpufreq_cpu_put(policy);
	return index;
}

static int rda_vivante_gpu_reset(void)
{
	void __iomem *gpu_rst_reg = gpu_base + 0x1028;

	writel(0x1, gpu_rst_reg);
	return 0;
}

static int __init rda_cpuidle_init(void)
{
	int ret;
	unsigned int cpu;
	struct cpuidle_device *dev;
	//struct cpuidle_driver *drv = &rda_idle_driver;

	ret = cpuidle_register_driver(&rda_idle_driver);
	if (ret) {
		pr_err("CPUidle driver registration failed\n");
		return ret;
	}

	for_each_possible_cpu(cpu)
	{
		dev = &per_cpu(rda_idle_device, cpu);
		dev->cpu = cpu;

		//dev->state_count = drv->state_count;
		ret = cpuidle_register_device(dev);
		if (ret) {
			pr_err("CPU%u: CPUidle device registration failed\n",
				cpu);
			return ret;
		}
	}
	
	printk("rda_cpuidle_init_3");
	
	gpu_base = ioremap(RDA_GPU_PHYS, SZ_8K);
	
	if(!gpu_base) {
		pr_err("cannot remap for gpu\n");
		return 1;
	}
	
	rda_vivante_gpu_reset();
	
	return 0;
}
device_initcall(rda_cpuidle_init);
#endif