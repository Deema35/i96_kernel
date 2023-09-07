/*
 * Copyright (C) 2013 RDA Microelectronics Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#include "ap_clk.h"
#include <linux/clk-provider.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/string.h>




// clk_prepare.....................................................

static int clk_rda_prepare(struct clk_hw *hw)
{
	return 0;
}

static int clk_rda_usb_prepare(struct clk_hw *hw)
{
	apsys_enable_usb_clk(1);
	return 0;
}

// clk_unprepare.....................................................

static void clk_rda_unprepare(struct clk_hw *hw)
{
	
}

static void clk_rda_usb_unprepare(struct clk_hw *hw)
{
	apsys_enable_usb_clk(0);
	
}

// clk_enable......................................................

static int clk_rda_enable(struct clk_hw *hw)
{
	return 0;
}

static int clk_rda_gouda_enable(struct clk_hw *hw)
{
	apsys_enable_gouda_clk(1);
	return 0;
}

static int clk_rda_dpi_enable(struct clk_hw *hw)
{
	apsys_enable_dpi_clk(1);
	return 0;
}

static int clk_rda_dsi_enable(struct clk_hw *hw)
{
	apsys_enable_dsi_clk(1);
	return 0;
}

static int clk_rda_camera_enable(struct clk_hw *hw)
{
	apsys_enable_camera_clk(1);
	return 0;
}

static int clk_rda_gpu_enable(struct clk_hw *hw)
{
	apsys_enable_gpu_clk(1);
	return 0;
}

static int clk_rda_vpu_enable(struct clk_hw *hw)
{
	apsys_enable_vpu_clk(1);
	return 0;
}

static int clk_rda_voc_enable(struct clk_hw *hw)
{
	apsys_enable_voc_clk(1);
	return 0;
}

static int clk_rda_spiflash_enable(struct clk_hw *hw)
{
	apsys_enable_spiflash_clk(1);
	return 0;
}

static int clk_rda_uart1_enable(struct clk_hw *hw)
{
	apsys_enable_uart1_clk(1);
	return 0;
}

static int clk_rda_uart2_enable(struct clk_hw *hw)
{
	apsys_enable_uart2_clk(1);
	return 0;
}

static int clk_rda_uart3_enable(struct clk_hw *hw)
{
	apsys_enable_uart3_clk(1);
	return 0;
}

static int clk_rda_bck_enable(struct clk_hw *hw)
{
	apsys_enable_bck_clk(1);
	return 0;
}

static int clk_rda_csi_enable(struct clk_hw *hw)
{
	apsys_enable_csi_clk(1);
	return 0;
}

static int clk_rda_debug_enable(struct clk_hw *hw)
{
	apsys_enable_debug_clk(1);
	return 0;
}

static int clk_rda_clk_out_enable(struct clk_hw *hw)
{
	apsys_enable_clk_out(1);
	return 0;
}

static int clk_rda_aux_enable(struct clk_hw *hw)
{
	apsys_enable_aux_clk(1);
	return 0;
}

static int clk_rda_clk_32_enable(struct clk_hw *hw)
{
	apsys_enable_clk_32k(1);
	return 0;
}

static int clk_rda_bus_enable(struct clk_hw *hw)
{
	apsys_enable_bus_clk(1);
	return 0;
}

// clk_disable......................................................

static void clk_rda_disable(struct clk_hw *hw)
{
	
}

static void clk_rda_gouda_disable(struct clk_hw *hw)
{
	apsys_enable_gouda_clk(0);
	
}

static void clk_rda_dpi_disable(struct clk_hw *hw)
{
	apsys_enable_dpi_clk(0);
	
}

static void clk_rda_dsi_disable(struct clk_hw *hw)
{
	apsys_enable_dsi_clk(0);
	
}

static void clk_rda_camera_disable(struct clk_hw *hw)
{
	apsys_enable_camera_clk(0);
	
}

static void clk_rda_gpu_disable(struct clk_hw *hw)
{
	apsys_enable_gpu_clk(0);
	
}

static void clk_rda_vpu_disable(struct clk_hw *hw)
{
	apsys_enable_vpu_clk(0);
	
}

static void clk_rda_voc_disable(struct clk_hw *hw)
{
	apsys_enable_voc_clk(0);
	
}

static void clk_rda_spiflash_disable(struct clk_hw *hw)
{
	apsys_enable_spiflash_clk(0);
	
}

static void clk_rda_uart1_disable(struct clk_hw *hw)
{
	apsys_enable_uart1_clk(0);
	
}

static void clk_rda_uart2_disable(struct clk_hw *hw)
{
	apsys_enable_uart2_clk(0);
	
}

static void clk_rda_uart3_disable(struct clk_hw *hw)
{
	apsys_enable_uart3_clk(0);
	
}

static void clk_rda_bck_disable(struct clk_hw *hw)
{
	apsys_enable_bck_clk(0);
	
}

static void clk_rda_csi_disable(struct clk_hw *hw)
{
	apsys_enable_csi_clk(0);
	
}

static void clk_rda_debug_disable(struct clk_hw *hw)
{
	apsys_enable_debug_clk(0);
	
}

static void clk_rda_clk_out_disable(struct clk_hw *hw)
{
	apsys_enable_clk_out(0);
	
}

static void clk_rda_aux_disable(struct clk_hw *hw)
{
	apsys_enable_aux_clk(0);
	
}

static void clk_rda_clk_32_disable(struct clk_hw *hw)
{
	apsys_enable_clk_32k(0);
	
}

static void clk_rda_bus_disable(struct clk_hw *hw)
{
	apsys_enable_bus_clk(0);
	
}

// clk_rda_recalc_rate..........................................

static unsigned long clk_rda_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	unsigned long rate = 0;
	return rate;
}

static unsigned long clk_rda_cpu_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	unsigned long rate = 0;
	rate = apsys_get_cpu_clk_rate();
	return rate;
}

static unsigned long clk_rda_bus_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	unsigned long rate = 0;
	rate = apsys_get_bus_clk_rate();
	return rate;
}

static unsigned long clk_rda_mem_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	unsigned long rate = 0;
	rate = apsys_get_mem_clk_rate();
	return rate;
}

static unsigned long clk_rda_usb_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	unsigned long rate = 0;
	rate = apsys_get_usb_clk_rate();
	return rate;
}

static unsigned long clk_rda_axi_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	unsigned long rate = 0;
	rate = apsys_get_axi_clk_rate();
	return rate;
}

static unsigned long clk_rda_ahb1_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	unsigned long rate = 0;
	rate = apsys_get_ahb1_clk_rate();
	return rate;
}

static unsigned long clk_rda_apb1_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	unsigned long rate = 0;
	rate = apsys_get_apb1_clk_rate();
	return rate;
}

static unsigned long clk_rda_apb2_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	unsigned long rate = 0;
	rate = apsys_get_apb2_clk_rate();
	return rate;
}

static unsigned long clk_rda_gpu_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	unsigned long rate = 0;
	rate = apsys_get_gpu_clk_rate();
	return rate;
}

static unsigned long clk_rda_vpu_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	unsigned long rate = 0;
	rate = apsys_get_vpu_clk_rate();
	return rate;
}

static unsigned long clk_rda_voc_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	unsigned long rate = 0;
	rate = apsys_get_voc_clk_rate();
	return rate;
}

static unsigned long clk_rda_spiflash_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	unsigned long rate = 0;
	rate = apsys_get_spiflash_clk_rate();
	return rate;
}

static unsigned long clk_rda_uart1_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	unsigned long rate = 0;
	rate = apsys_get_uart_clk_rate(0);
	return rate;
}

static unsigned long clk_rda_uart2_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	unsigned long rate = 0;
	rate = apsys_get_uart_clk_rate(1);
	return rate;
}

static unsigned long clk_rda_uart3_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	unsigned long rate = 0;
	rate = apsys_get_uart_clk_rate(2);
	return rate;
}

static unsigned long clk_rda_bck_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	unsigned long rate = 0;
	rate = apsys_get_bck_clk_rate();
	return rate;
}

static unsigned long clk_rda_gcg_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	unsigned long rate = 0;
	rate = apsys_get_gcg_clk_rate();
	return rate;
}

// clk_rda_set_rate.........................................

static int clk_rda_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
	return 0;
}

static int clk_rda_cpu_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
	apsys_set_cpu_clk_rate(rate);
	return 0;
}

static int clk_rda_bus_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
	apsys_set_bus_clk_rate(rate);
	return 0;
}

static int clk_rda_mem_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
	apsys_set_mem_clk_rate(rate);
	return 0;
}

static int clk_rda_usb_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
	apsys_set_usb_clk_rate(rate);
	return 0;
}

static int clk_rda_axi_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
	apsys_set_axi_clk_rate(rate);
	return 0;
}

static int clk_rda_ahb1_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
	apsys_set_ahb1_clk_rate(rate);
	return 0;
}

static int clk_rda_apb1_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
	apsys_set_apb1_clk_rate(rate);
	return 0;
}

static int clk_rda_apb2_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
	apsys_set_apb2_clk_rate(rate);
	return 0;
}

static int clk_rda_gpu_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
	apsys_set_gpu_clk_rate(rate);
	return 0;
}

static int clk_rda_vpu_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
	apsys_set_vpu_clk_rate(rate);
	return 0;
}

static int clk_rda_voc_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
	apsys_set_voc_clk_rate(rate);
	return 0;
}

static int clk_rda_spiflash_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
	apsys_set_spiflash_clk_rate(rate);
	return 0;
}

static int clk_rda_uart1_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
	apsys_set_uart_clk_rate(0, rate);
	return 0;
}

static int clk_rda_uart2_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
	apsys_set_uart_clk_rate(1, rate);
	return 0;
}

static int clk_rda_uart3_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
	apsys_set_uart_clk_rate(2, rate);
	return 0;
}

static int clk_rda_bck_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
	apsys_set_bck_clk_rate(rate);
	return 0;
}

static int clk_rda_gcg_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
	apsys_set_gcg_clk_rate(rate);
	return 0;
}

static int clk_rda_dsi_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
	apsys_set_dsi_clk_rate(rate);
	return 0;
}

// clk_rda_ops_init.........................................................

/*static int clk_rda_ops_init(struct clk_hw *hw)
{
	return 0;
}*/

static int clk_rda_gouda_ops_init(struct clk_hw *hw)
{
	apsys_enable_gouda_clk(1);
	return 0;
}



static long clk_rda_round_rate(struct clk_hw *hw, unsigned long rate, unsigned long *unused)
{
	return (long)rate;
}




struct clk_ops cpu_clk_rda_ops = {
	.prepare = clk_rda_prepare,
	.unprepare = clk_rda_unprepare,
	.enable = clk_rda_enable,
	.disable = clk_rda_disable,
	.set_rate = clk_rda_cpu_set_rate,
	.round_rate = clk_rda_round_rate,
	.recalc_rate = clk_rda_cpu_recalc_rate,
};

struct clk_ops bus_clk_rda_ops = {
	.prepare = clk_rda_prepare,
	.unprepare = clk_rda_unprepare,
	.enable = clk_rda_bus_enable,
	.disable = clk_rda_bus_disable,
	.set_rate = clk_rda_bus_set_rate,
	.round_rate = clk_rda_round_rate,
	.recalc_rate = clk_rda_bus_recalc_rate,
};

struct clk_ops mem_clk_rda_ops = {
	.prepare = clk_rda_prepare,
	.unprepare = clk_rda_unprepare,
	.enable = clk_rda_enable,
	.disable = clk_rda_disable,
	.set_rate = clk_rda_mem_set_rate,
	.round_rate = clk_rda_round_rate,
	.recalc_rate = clk_rda_mem_recalc_rate,
};

struct clk_ops usb_clk_rda_ops = {
	.prepare = clk_rda_usb_prepare,
	.unprepare = clk_rda_usb_unprepare,
	.enable = clk_rda_enable,
	.disable = clk_rda_disable,
	.set_rate = clk_rda_usb_set_rate,
	.round_rate = clk_rda_round_rate,
	.recalc_rate = clk_rda_usb_recalc_rate,
};

struct clk_ops axi_clk_rda_ops = {
	.prepare = clk_rda_prepare,
	.unprepare = clk_rda_unprepare,
	.enable = clk_rda_enable,
	.disable = clk_rda_disable,
	.set_rate = clk_rda_axi_set_rate,
	.round_rate = clk_rda_round_rate,
	.recalc_rate = clk_rda_axi_recalc_rate,
};

struct clk_ops ahb1_clk_rda_ops = {
	.prepare = clk_rda_prepare,
	.unprepare = clk_rda_unprepare,
	.enable = clk_rda_enable,
	.disable = clk_rda_disable,
	.set_rate = clk_rda_ahb1_set_rate,
	.round_rate = clk_rda_round_rate,
	.recalc_rate = clk_rda_ahb1_recalc_rate,
};

struct clk_ops apb1_clk_rda_ops = {
	.prepare = clk_rda_prepare,
	.unprepare = clk_rda_unprepare,
	.enable = clk_rda_enable,
	.disable = clk_rda_disable,
	.set_rate = clk_rda_apb1_set_rate,
	.round_rate = clk_rda_round_rate,
	.recalc_rate = clk_rda_apb1_recalc_rate,
};

struct clk_ops apb2_clk_rda_ops = {
	.prepare = clk_rda_prepare,
	.unprepare = clk_rda_unprepare,
	.enable = clk_rda_enable,
	.disable = clk_rda_disable,
	.set_rate = clk_rda_apb2_set_rate,
	.round_rate = clk_rda_round_rate,
	.recalc_rate = clk_rda_apb2_recalc_rate,
};

struct clk_ops gcg_clk_rda_ops = {
	.prepare = clk_rda_prepare,
	.unprepare = clk_rda_unprepare,
	.enable = clk_rda_enable,
	.disable = clk_rda_disable,
	.set_rate = clk_rda_gcg_set_rate,
	.round_rate = clk_rda_round_rate,
	.recalc_rate = clk_rda_gcg_recalc_rate,
};

struct clk_ops gpu_clk_rda_ops = {
	.prepare = clk_rda_prepare,
	.unprepare = clk_rda_unprepare,
	.enable = clk_rda_gpu_enable,
	.disable = clk_rda_gpu_disable,
	.set_rate = clk_rda_gpu_set_rate,
	.round_rate = clk_rda_round_rate,
	.recalc_rate = clk_rda_gpu_recalc_rate,
};

struct clk_ops vpu_clk_rda_ops = {
	.prepare = clk_rda_prepare,
	.unprepare = clk_rda_unprepare,
	.enable = clk_rda_vpu_enable,
	.disable = clk_rda_vpu_disable,
	.set_rate = clk_rda_vpu_set_rate,
	.round_rate = clk_rda_round_rate,
	.recalc_rate = clk_rda_vpu_recalc_rate,
};

struct clk_ops voc_clk_rda_ops = {
	.prepare = clk_rda_prepare,
	.unprepare = clk_rda_unprepare,
	.enable = clk_rda_voc_enable,
	.disable = clk_rda_voc_disable,
	.set_rate = clk_rda_voc_set_rate,
	.round_rate = clk_rda_round_rate,
	.recalc_rate = clk_rda_voc_recalc_rate,
};

struct clk_ops uart1_clk_rda_ops = {
	.prepare = clk_rda_prepare,
	.unprepare = clk_rda_unprepare,
	.enable = clk_rda_uart1_enable,
	.disable = clk_rda_uart1_disable,
	.set_rate = clk_rda_uart1_set_rate,
	.round_rate = clk_rda_round_rate,
	.recalc_rate = clk_rda_uart1_recalc_rate,
};

struct clk_ops uart2_clk_rda_ops = {
	.prepare = clk_rda_prepare,
	.unprepare = clk_rda_unprepare,
	.enable = clk_rda_uart2_enable,
	.disable = clk_rda_uart2_disable,
	.set_rate = clk_rda_uart2_set_rate,
	.round_rate = clk_rda_round_rate,
	.recalc_rate = clk_rda_uart2_recalc_rate,
};

struct clk_ops uart3_clk_rda_ops = {
	.prepare = clk_rda_prepare,
	.unprepare = clk_rda_unprepare,
	.enable = clk_rda_uart3_enable,
	.disable = clk_rda_uart3_disable,
	.set_rate = clk_rda_uart3_set_rate,
	.round_rate = clk_rda_round_rate,
	.recalc_rate = clk_rda_uart3_recalc_rate,
};

struct clk_ops spiflash_clk_rda_ops = {
	.prepare = clk_rda_prepare,
	.unprepare = clk_rda_unprepare,
	.enable = clk_rda_spiflash_enable,
	.disable = clk_rda_spiflash_disable,
	.set_rate = clk_rda_spiflash_set_rate,
	.round_rate = clk_rda_round_rate,
	.recalc_rate = clk_rda_spiflash_recalc_rate,
};

struct clk_ops bck_clk_rda_ops = {
	.prepare = clk_rda_prepare,
	.unprepare = clk_rda_unprepare,
	.enable = clk_rda_bck_enable,
	.disable = clk_rda_bck_disable,
	.set_rate = clk_rda_bck_set_rate,
	.round_rate = clk_rda_round_rate,
	.recalc_rate = clk_rda_bck_recalc_rate,
};

struct clk_ops dsi_clk_rda_ops = {
	.prepare = clk_rda_prepare,
	.unprepare = clk_rda_unprepare,
	.enable = clk_rda_dsi_enable,
	.disable = clk_rda_dsi_disable,
	.set_rate = clk_rda_dsi_set_rate,
	.round_rate = clk_rda_round_rate,
	.recalc_rate = clk_rda_recalc_rate,
};

struct clk_ops csi_clk_rda_ops = {
	.prepare = clk_rda_prepare,
	.unprepare = clk_rda_unprepare,
	.enable = clk_rda_csi_enable,
	.disable = clk_rda_csi_disable,
	.set_rate = clk_rda_set_rate,
	.round_rate = clk_rda_round_rate,
	.recalc_rate = clk_rda_recalc_rate,
};

struct clk_ops debug_clk_rda_ops = {
	.prepare = clk_rda_prepare,
	.unprepare = clk_rda_unprepare,
	.enable = clk_rda_debug_enable,
	.disable = clk_rda_debug_disable,
	.set_rate = clk_rda_set_rate,
	.round_rate = clk_rda_round_rate,
	.recalc_rate = clk_rda_recalc_rate,
};

struct clk_ops clk_out_clk_rda_ops = {
	.prepare = clk_rda_enable,
	.unprepare = clk_rda_disable,
	.enable	= clk_rda_clk_out_enable,
	.disable = clk_rda_clk_out_disable,
	.set_rate = clk_rda_set_rate,
	.round_rate = clk_rda_round_rate,
	.recalc_rate = clk_rda_recalc_rate,
};

struct clk_ops aux_clk_rda_ops = {
	.prepare = clk_rda_enable,
	.unprepare = clk_rda_disable,
	.enable	= clk_rda_aux_enable,
	.disable = clk_rda_aux_disable,
	.set_rate = clk_rda_set_rate,
	.round_rate = clk_rda_round_rate,
	.recalc_rate = clk_rda_recalc_rate,
};

struct clk_ops clk_32_clk_rda_ops = {
	.prepare = clk_rda_enable,
	.unprepare = clk_rda_disable,
	.enable	= clk_rda_clk_32_enable,
	.disable = clk_rda_clk_32_disable,
	.set_rate = clk_rda_set_rate,
	.round_rate = clk_rda_round_rate,
	.recalc_rate = clk_rda_recalc_rate,
};

struct clk_ops gouda_clk_rda_ops = {
	.enable = clk_rda_gouda_enable,
	.disable = clk_rda_gouda_disable,
	/* take parent rate */
	.set_rate = NULL,
	.round_rate = NULL,
	.recalc_rate = NULL,
	.init = clk_rda_gouda_ops_init,
};

struct clk_ops dpi_clk_rda_ops = {
	.enable = clk_rda_dpi_enable,
	.disable = clk_rda_dpi_disable,
	/* take parent rate */
	.set_rate = NULL,
	.round_rate = NULL,
	.recalc_rate = NULL,
};

struct clk_ops cam_clk_rda_ops = {
	.enable = clk_rda_camera_enable,
	.disable = clk_rda_camera_disable,
	/* take parent rate */
	.set_rate = NULL,
	.round_rate = NULL,
	.recalc_rate = NULL,
};


