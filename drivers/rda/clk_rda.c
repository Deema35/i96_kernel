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
  */
#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include <linux/platform_device.h>


#include <rda/plat/devices.h>
#include <dt-bindings/rda/rda_clk_list.h>

extern int rda_apsys_init(struct platform_device *pdev);

extern struct clk_ops cpu_clk_rda_ops;
extern struct clk_ops bus_clk_rda_ops;
extern struct clk_ops mem_clk_rda_ops;
extern struct clk_ops usb_clk_rda_ops;
extern struct clk_ops axi_clk_rda_ops;
extern struct clk_ops ahb1_clk_rda_ops;
extern struct clk_ops apb1_clk_rda_ops;
extern struct clk_ops apb2_clk_rda_ops;
extern struct clk_ops gcg_clk_rda_ops;
extern struct clk_ops gpu_clk_rda_ops;
extern struct clk_ops vpu_clk_rda_ops;
extern struct clk_ops voc_clk_rda_ops;
extern struct clk_ops uart1_clk_rda_ops;
extern struct clk_ops uart2_clk_rda_ops;
extern struct clk_ops uart3_clk_rda_ops;
extern struct clk_ops spiflash_clk_rda_ops;
extern struct clk_ops bck_clk_rda_ops;
extern struct clk_ops dsi_clk_rda_ops;
extern struct clk_ops csi_clk_rda_ops;
extern struct clk_ops debug_clk_rda_ops;

extern struct clk_ops clk_out_clk_rda_ops;
extern struct clk_ops aux_clk_rda_ops;
extern struct clk_ops clk_32_clk_rda_ops;


extern struct clk_ops cam_clk_rda_ops;
extern struct clk_ops gouda_clk_rda_ops;
extern struct clk_ops dpi_clk_rda_ops;



static struct clk_hw	cpu_hw;
static struct clk_hw	bus_hw;
static struct clk_hw	mem_hw;
static struct clk_hw	usb_hw;
static struct clk_hw	axi_hw;
static struct clk_hw	ahb1_hw;
static struct clk_hw	apb1_hw;
static struct clk_hw	apb2_hw;
static struct clk_hw	gcg_hw;
static struct clk_hw	gpu_hw;
static struct clk_hw	vpu_hw;
static struct clk_hw	voc_hw;
static struct clk_hw	uart1_hw;
static struct clk_hw	uart2_hw;
static struct clk_hw	uart3_hw;
static struct clk_hw	spiflash_hw;
static struct clk_hw	gouda_hw;
static struct clk_hw	dpi_hw;
static struct clk_hw	camera_hw;
static struct clk_hw	bck_hw;
static struct clk_hw	dsi_hw;
static struct clk_hw	csi_hw;
static struct clk_hw	debug_hw;
static struct clk_hw	clk_out_hw;
static struct clk_hw	aux_clk_hw;
static struct clk_hw	clk_32k_hw;

static struct clk_lookup sys_clocks_lookups[26];

static int rda_clk_hw_register(int id, struct clk_hw *hw, const char *name, struct clk_ops *rda_clk_ops, const char *parent_name, int parentNum, unsigned long	flags,
 struct clk_lookup *lookup, struct platform_device *pdev, struct clk_onecell_data *clk_data)
{
	struct clk_init_data init;
	int reg;

	init.name = name;
	init.ops = rda_clk_ops;
	init.flags = flags;
	init.parent_names = &parent_name;
	init.num_parents = parentNum;

	hw->init = &init;

	reg = clk_register(&pdev->dev, hw);
	
	if (IS_ERR(reg))
	{
		dev_info(&pdev->dev, "clk  %s registration error", name);
		
		return reg;
	}
	
	clk_data->clks[id] = reg;
	
	lookup->con_id = name;
	lookup->clk = hw->clk;
	lookup->clk_hw = hw;
	
	return 0;
}



static int rda_clk_probe(struct platform_device *pdev)
{
	int ret = 0;
	
	int clk_num = 26;
	
	struct device_node *node = pdev->dev.of_node;
	
	struct clk_onecell_data *clk_data;
	
	clk_data = kzalloc(sizeof(*clk_data), GFP_KERNEL);
	
	if (!clk_data) return -ENXIO;
	
	clk_data->clks = kcalloc(clk_num, sizeof(*clk_data->clks), GFP_KERNEL);
		
	if (!clk_data->clks)
	{
		kfree(clk_data);
		return -ENXIO;
	}
	
	clk_data->clk_num = clk_num;
	
	for (int i = 0; i < clk_num; i++) clk_data->clks[i] = ERR_PTR(-ENOENT);
	
	
	dev_info(&pdev->dev, "rda_clk_probe");
	
	ret = rda_apsys_init(pdev);
	
	if (ret < 0) return ret;
	
	/* root clocks */
	ret = rda_clk_hw_register(CLK_RDA_CPU, &cpu_hw, "cpu", &cpu_clk_rda_ops, NULL, 0, 0, &sys_clocks_lookups[0], pdev, clk_data);
	if (ret < 0) return ret;
	ret = rda_clk_hw_register(CLK_RDA_BUS, &bus_hw, "bus", &bus_clk_rda_ops, NULL, 0, 0, &sys_clocks_lookups[1], pdev, clk_data);
	if (ret < 0) return ret;
	ret = rda_clk_hw_register(CLK_RDA_MEM, &mem_hw, "mem", &mem_clk_rda_ops, NULL, 0, 0, &sys_clocks_lookups[2], pdev, clk_data);
	if (ret < 0) return ret;
	
	/* childs of BUS */
	ret = rda_clk_hw_register(CLK_RDA_USB, &usb_hw, "usb", &usb_clk_rda_ops, "bus", 1, 0, &sys_clocks_lookups[3], pdev, clk_data);
	if (ret < 0) return ret;
	ret = rda_clk_hw_register(CLK_RDA_AXI, &axi_hw, "axi", &axi_clk_rda_ops, "bus", 1, 0, &sys_clocks_lookups[4], pdev, clk_data);
	if (ret < 0) return ret;
	ret = rda_clk_hw_register(CLK_RDA_AHB1, &ahb1_hw, "ahb1", &ahb1_clk_rda_ops, "bus", 1, 0, &sys_clocks_lookups[5], pdev, clk_data);
	if (ret < 0) return ret;
	ret = rda_clk_hw_register(CLK_RDA_APB1, &apb1_hw, "apb1", &apb1_clk_rda_ops, "bus", 1, 0, &sys_clocks_lookups[6], pdev, clk_data);
	if (ret < 0) return ret;
	ret = rda_clk_hw_register(CLK_RDA_APB2, &apb2_hw, "apb2", &apb2_clk_rda_ops, "bus", 1, 0, &sys_clocks_lookups[7], pdev, clk_data);
	if (ret < 0) return ret;
	ret = rda_clk_hw_register(CLK_RDA_GCG, &gcg_hw, "gcg", &gcg_clk_rda_ops, "bus", 1, 0, &sys_clocks_lookups[8], pdev, clk_data);
	if (ret < 0) return ret;
	ret = rda_clk_hw_register(CLK_RDA_GPU, &gpu_hw, "gpu", &gpu_clk_rda_ops, "bus", 1, 0, &sys_clocks_lookups[9], pdev, clk_data);
	if (ret < 0) return ret;
	ret = rda_clk_hw_register(CLK_RDA_VPU, &vpu_hw, "vpu", &vpu_clk_rda_ops, "bus", 1, 0, &sys_clocks_lookups[10], pdev, clk_data);
	if (ret < 0) return ret;
	ret = rda_clk_hw_register(CLK_RDA_VOC, &voc_hw, "voc", &voc_clk_rda_ops, "bus", 1, 0, &sys_clocks_lookups[11], pdev, clk_data);
	if (ret < 0) return ret;
	
	/* childs of APB2 */
	ret = rda_clk_hw_register(CLK_RDA_UART1, &uart1_hw, "uart1", &uart1_clk_rda_ops, "apb2", 1, 0, &sys_clocks_lookups[12], pdev, clk_data);
	if (ret < 0) return ret;
	ret = rda_clk_hw_register(CLK_RDA_UART2, &uart2_hw, "uart2", &uart2_clk_rda_ops, "apb2", 1, 0, &sys_clocks_lookups[13], pdev, clk_data);
	if (ret < 0) return ret;
	ret = rda_clk_hw_register(CLK_RDA_UART2, &uart3_hw, "uart3", &uart3_clk_rda_ops, "apb2", 1, 0, &sys_clocks_lookups[14], pdev, clk_data);
	if (ret < 0) return ret;
	
	/* childs of AHB1 */
	ret = rda_clk_hw_register(CLK_RDA_SPIFLASH, &spiflash_hw, "spiflash", &spiflash_clk_rda_ops, "apb1", 1, 0, &sys_clocks_lookups[15], pdev, clk_data);
	if (ret < 0) return ret;
	
	/* childs of GCG */
	ret = rda_clk_hw_register(CLK_RDA_GOUDA, &gouda_hw, "gouda", &gouda_clk_rda_ops, "gcg", 1, CLK_SET_RATE_PARENT, &sys_clocks_lookups[16], pdev, clk_data);
	if (ret < 0) return ret;
	ret = rda_clk_hw_register(CLK_RDA_DPI, &dpi_hw, "dpi", &gouda_clk_rda_ops, "gcg", 1, CLK_SET_RATE_PARENT, &sys_clocks_lookups[17], pdev, clk_data);
	if (ret < 0) return ret;
	ret = rda_clk_hw_register(CLK_RDA_CAMERA, &camera_hw, "camera", &gouda_clk_rda_ops, "gcg", 1, CLK_SET_RATE_PARENT, &sys_clocks_lookups[18], pdev, clk_data);
	if (ret < 0) return ret;
	
	ret = rda_clk_hw_register(CLK_RDA_BCK, &bck_hw, "bck", &bck_clk_rda_ops, NULL, 0, 0, &sys_clocks_lookups[19], pdev, clk_data);
	if (ret < 0) return ret;
	ret = rda_clk_hw_register(CLK_RDA_DSI, &dsi_hw, "dsi", &dsi_clk_rda_ops, NULL, 0, 0, &sys_clocks_lookups[20], pdev, clk_data);
	if (ret < 0) return ret;
	ret = rda_clk_hw_register(CLK_RDA_CSI, &csi_hw, "csi", &csi_clk_rda_ops, NULL, 0, 0, &sys_clocks_lookups[21], pdev, clk_data);
	if (ret < 0) return ret;
	ret = rda_clk_hw_register(CLK_RDA_DEBUG, &debug_hw, "debug", &debug_clk_rda_ops, NULL, 0, 0, &sys_clocks_lookups[22], pdev, clk_data);
	if (ret < 0) return ret;

	
	ret = rda_clk_hw_register(CLK_RDA_CLK_OUT, &clk_out_hw, "clk_out", &clk_out_clk_rda_ops, NULL, 0, 0, &sys_clocks_lookups[23], pdev, clk_data);
	if (ret < 0) return ret;
	ret = rda_clk_hw_register(CLK_RDA_AUX_CLK, &aux_clk_hw, "aux_clk", &aux_clk_rda_ops, NULL, 0, 0, &sys_clocks_lookups[24], pdev, clk_data);
	if (ret < 0) return ret;
	ret = rda_clk_hw_register(CLK_RDA_CLK_32K,&clk_32k_hw, "clk_32k", &clk_32_clk_rda_ops, NULL, 0, 0, &sys_clocks_lookups[25], pdev, clk_data);
	if (ret < 0) return ret;


	ret = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);
	
	return ret;
}



static const struct of_device_id rda_sysclk_dt_matches[] = {
	{ .compatible = "rda,8810pl-sysclk" },
	{ }
};
MODULE_DEVICE_TABLE(of, rda_sysclk_dt_matches);

static struct platform_driver rda_clk_driver = {
	.probe		= rda_clk_probe,
	.driver 	= {
		.name	= RDA_CLK_DRV_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = rda_sysclk_dt_matches,
	},
};

static int __init rda_clk_init(void)
{
	return platform_driver_register(&rda_clk_driver);

}

arch_initcall(rda_clk_init);