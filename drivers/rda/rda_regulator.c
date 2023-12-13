/*
 * Copyright (C) 2012 RDA Communications Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
 #include <linux/of.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <asm/system_misc.h>
#include <asm/io.h>

#include <rda/plat/md_sys.h>
#include <rda/plat/reg_md_sysctrl.h>
#include <rda/plat/boot_mode.h>
#include <rda/mach/regulator.h>


#include "regulator-devices.h"



/* rda regulator struct for driver*/
struct rda_regulator {
	struct regulator_dev *rdev;
	struct rda_reg_config *config;

	struct msys_device *msys_dev;
	u32 cur_val;

	void *private;
};

struct rda_pm_param {
	u32 pm_id;
	u32 pm_val;
};

static void __iomem *rda_sysctrl = NULL;
static struct rda_regulator rda_power_ctrl;

static unsigned int rda_regulator_send_cmd(struct rda_regulator *rda_reg, u32 value)
{
	struct rda_reg_config *rda_cfg = rda_reg->config;
	struct rda_pm_param param;
	struct client_cmd cmd_set;

	memset(&cmd_set, 0, sizeof(cmd_set));

	cmd_set.pmsys_dev = rda_reg->msys_dev;
	cmd_set.mod_id = SYS_PM_MOD;
	cmd_set.mesg_id = rda_cfg->msys_cmd;

	param.pm_id = rda_cfg->pm_id;
	param.pm_val = value;

	cmd_set.pdata = (void *)&param;
	cmd_set.data_size = sizeof(param);

	return rda_msys_send_cmd(&cmd_set);
}

/*rda ldo specific func*/
int rda_regulator_do_init(void *driver_data)
{
	struct rda_regulator *rda_reg = (struct rda_regulator *)driver_data;
	struct rda_reg_config *rda_cfg = rda_reg->config;
	struct rda_reg_def *rda_def = (struct rda_reg_def *)rda_cfg->init_data->driver_data;
	struct platform_device *pdev = (struct platform_device *)rda_reg->private;
	unsigned int ret = 0;
	u32 param = 0;
	int reg_id;

	reg_id = pdev->id;

	/* Skip these ldos. */
/*#ifdef CONFIG_LEDS_RDA
	if (reg_id == rda_ldo_keypad) {
		return 0;
	}
#else
	if (reg_id == rda_ldo_keypad || reg_id == rda_ldo_ledr ||
		reg_id == rda_ldo_ledg || reg_id == rda_ldo_ledb) {
		return 0;
	}
#endif*/ /* CONFIG_LEDS_RDA */

	switch (reg_id)
	{
		case rda_ldo_cam:
		case rda_ldo_sdmmc:
		case rda_ldo_fm:
		case rda_ldo_bt:
		case rda_ldo_lcd:
		case rda_ldo_cam_flash:
		case rda_ldo_i2c:
		case rda_ldo_vibrator:
		
		case rda_ldo_keypad:
		case rda_ldo_backlight:
		case rda_ldo_ledr:
		case rda_ldo_ledg:
		case rda_ldo_ledb:
		
			param = (rda_def->def_val == 1800000 ? POWER_OFF : POWER_ON);
			break;
		
		
		
		
			param = rda_def->def_val / 10000 - 1;
			break;
			
		case rda_ldo_usb:
		
			/* usb ldo has been open in bootloader, so re-init it to off */
			param = POWER_OFF;
			break;
			
		default: param = POWER_OFF;
	}

	
		

	ret = rda_regulator_send_cmd(rda_reg, param);
	if (ret > 0) {
		printk(KERN_ERR "<rda-regulator> : Failed as sending command to regulator.%d!\n", reg_id);
		rda_reg->cur_val = ((int *)rda_cfg->table)[0];
	} else {
		rda_reg->cur_val = rda_def->def_val;
	}

	return 0;
}

/* standard regulator dev ops*/
static int rda_regulator_enable(struct regulator_dev *rdev)
{
	struct rda_regulator *rda_reg = rdev_get_drvdata(rdev);
	struct rda_reg_config *rda_cfg = rda_reg->config;
	int *table = (int *)rda_cfg->table;
	int tsize = rda_cfg->tsize;
	unsigned int ret = 0;

	pr_debug("%s\n", __func__);

	if (rda_cfg->msys_cmd != SYS_PM_CMD_EN) {
		return 0;
	}

	if (rda_reg->cur_val == table[tsize - 1]) {
		return 0;
	}

	ret = rda_regulator_send_cmd(rda_reg, POWER_ON);
	if (ret > 0) {
		printk(KERN_ERR "<rda-regulator> : Failed as enabling!\n");
		return -EINVAL;
	}

	rda_reg->cur_val = table[tsize - 1];

	return 0;
}

static int rda_regulator_disable(struct regulator_dev *rdev)
{
	struct rda_regulator *rda_reg = rdev_get_drvdata(rdev);
	struct rda_reg_config *rda_cfg = rda_reg->config;
	int *table = (int *)rda_cfg->table;
	unsigned int ret = 0;

	pr_debug("%s\n", __func__);

	if (rda_cfg->msys_cmd != SYS_PM_CMD_EN) {
		return 0;
	}

	if (rda_reg->cur_val == table[0]) {
		return 0;
	}

	ret = rda_regulator_send_cmd(rda_reg, POWER_OFF);
	if (ret > 0) {
		printk(KERN_ERR "<rda-regulator> : Failed as disabling!\n");
		return -EINVAL;
	}

	rda_reg->cur_val = table[0];

	return 0;
}

static int rda_regulator_is_enabled(struct regulator_dev *rdev)
{
	struct rda_regulator *rda_reg = rdev_get_drvdata(rdev);
	struct rda_reg_config *rda_cfg = rda_reg->config;
	int *table = (int *)rda_cfg->table;
	int tsize = rda_cfg->tsize;

	pr_debug("%s\n", __func__);

        if (rda_reg->cur_val == table[tsize - 1]) {
		return 1;
	}

	return 0;
}

static int rda_regulator_set_voltage_sel(struct regulator_dev *rdev, unsigned selector)
{
	struct rda_regulator *rda_reg = rdev_get_drvdata(rdev);
	struct rda_reg_config *config = rda_reg->config;
	int *table = (int *)config->table;
	u32 tsize = config->tsize;
	u32 value = selector;
	unsigned int ret = 0;

	if (rda_reg->config->msys_cmd != SYS_PM_CMD_SET_LEVEL) {
		return -EINVAL;
	}

	if (value >= tsize) {
		return -EINVAL;
	}

	if (rda_reg->cur_val == table[value]) {
		return 0;
	}

	ret = rda_regulator_send_cmd(rda_reg, value);
	if (ret > 0) {
		printk(KERN_ERR "<rda-regulator> : Failed as setting volt!\n");
		return -EINVAL;
	}

	rda_reg->cur_val = table[value];

	return 0;
}

static int rda_regulator_get_voltage_sel(struct regulator_dev *rdev)
{
	struct rda_regulator *rda_reg = rdev_get_drvdata(rdev);
	struct rda_reg_config *config = rda_reg->config;
	int *table = (int *)config->table;
	int tsize = config->tsize;
	int i;

	for (i = 0; i < tsize; i++) {
		if (table[i] == rda_reg->cur_val) {
			printk(KERN_ERR "<rda-regulator> : voltage_sel = %d, pm_id = %d cur_val=%d\n", i, config->pm_id, rda_reg->cur_val);
			return i;
		}
	}

	return -EINVAL;
}

static int rda_regulator_list_voltage(struct regulator_dev *rdev, unsigned selector)
{
	struct rda_regulator *rda_reg = rdev_get_drvdata(rdev);
	struct rda_reg_config *rda_config = rda_reg->config;

	if (selector >= rdev->desc->n_voltages) {
		return -EINVAL;
	}

	/* selector is from 0 to max. */
	return ((int *)rda_config->table)[selector];
}

static int rda_regulator_set_mode(struct regulator_dev *rdev, unsigned int mode)
{
	return 0;
}

static unsigned int rda_regulator_get_mode(struct regulator_dev *rdev)
{
	pr_debug("%s\n", __func__);

	return REGULATOR_MODE_NORMAL;
}

static int rda_regulator_set_current_limit(struct regulator_dev *rdev, int min_uA, int max_uA)
{
	struct rda_regulator *rda_reg = rdev_get_drvdata(rdev);
	struct rda_reg_config *config = rda_reg->config;
	int *table = (int *)config->table;
	int tsize = config->tsize;
	u32 value = table[1];
	int i;
	unsigned int ret = 0;

	if (rda_reg->config->msys_cmd != SYS_PM_CMD_SET_LEVEL) {
		return -EINVAL;
	}

	if (min_uA < table[0] || max_uA > table[tsize - 1]) {
		return -EINVAL;
	}

	for (i = 0; i < tsize; i++) {
		if (min_uA <= table[i]) {
			value = table[i];
			break;
		}
	}

	if (rda_reg->cur_val == value) {
		return 0;
	}

	pr_debug("%s : min = %d, max = %d, set = %d\n", __func__, min_uA, max_uA, value);

	/* Unit is mA which can be handled by BP. */
	ret = rda_regulator_send_cmd(rda_reg, value / 1000);
	if (ret > 0) {
		printk(KERN_ERR "<rda-regulator> : Failed as setting volt!\n");
		return -EINVAL;
	}

	rda_reg->cur_val = value;

	return 0;
}

static int rda_regulator_get_current_limit(struct regulator_dev *rdev)
{
	struct rda_regulator *rda_reg = rdev_get_drvdata(rdev);

	if (rda_reg->config->msys_cmd != SYS_PM_CMD_SET_LEVEL) {
		return -EINVAL;
	}

	return rda_reg->cur_val;
}

static struct regulator_ops rda_regulator_ops = {
	.enable			= rda_regulator_enable,
	.disable		= rda_regulator_disable,
	.is_enabled		= rda_regulator_is_enabled,
	/* If we have used set_voltage_sel, we cann't use set_voltage. If so core of regulator will report warning. */
	.set_voltage_sel	= rda_regulator_set_voltage_sel,
	/* If we have used get_voltage_sel, we cann't use get_voltage. If so core of regulator will report warning. */
	.get_voltage_sel	= rda_regulator_get_voltage_sel,
	.list_voltage		= rda_regulator_list_voltage,
	.set_mode		= rda_regulator_set_mode,
	.get_mode		= rda_regulator_get_mode,
	.set_current_limit	= rda_regulator_set_current_limit,
	.get_current_limit	= rda_regulator_get_current_limit,
};



static u32 GetDeviceTreeProperty(struct platform_device *pdev, const char *PropertyName)
{
	const void *ptr;
	
	ptr = of_get_property(pdev->dev.of_node, PropertyName, NULL);

	if (!ptr) dev_info(&pdev->dev,"Can't read %s", PropertyName);
	
	return be32_to_cpup(ptr);
    
}

static void GetDeviceTreeStringProperty(struct platform_device *pdev, char *StringContainer, const char *PropertyName)
{
	const void *ptr;
	
	ptr = of_get_property(pdev->dev.of_node, PropertyName, NULL);

	if (!ptr)
	{
		dev_info(&pdev->dev,"Can't read %s", PropertyName);
		return;
	}
	
	strcpy (StringContainer, (char *)ptr);
    
}

struct ArraySt
{
	u32* Array;
	int Num;
};

static int GetArrayProperty(struct platform_device *pdev, const char *PropertyName, struct ArraySt* ArraySt)
{
	int tmp = 0;
	u32 value;

	if (!of_get_property(pdev->dev.of_node, PropertyName, &tmp))
	{
		dev_info(&pdev->dev,"Can't read array %s", PropertyName);
		return 1;
	}
	
	ArraySt->Num = tmp / (sizeof(u32));
	
	ArraySt->Array = kzalloc(sizeof(u32) * ArraySt->Num, GFP_KERNEL);

	for (int i = 0; i < ArraySt->Num; i++)
	{
		if (of_property_read_u32_index(pdev->dev.of_node, PropertyName, i, &value))
		{
			dev_info(&pdev->dev,"Can't read array member %s", PropertyName);
			return 1;
		}
		ArraySt->Array[i] = value;

	}
	
	return 0;
}
	
	

/* standard regulator driver for system mount*/
static int rda_regulator_probe(struct platform_device *pdev)
{
	struct regulator_desc *rda_regs_desc = NULL; 
	struct rda_regulator *rda_reg = NULL;
	struct rda_reg_config *rda_config = NULL;
	struct regulator_init_data *init_data = NULL;
	char *suply_name = NULL;
	
	struct regulator_consumer_supply *consumer_supply = NULL;
	struct rda_reg_def *reg_default = NULL;
	struct regulator_config *config = NULL;
	struct ArraySt ArrayTmp;
	
	u32 *vtable = NULL;
	int ret = 0;
	
	
	
	rda_reg = kzalloc(sizeof(struct rda_regulator), GFP_KERNEL);
	rda_config = kzalloc(sizeof(struct rda_reg_config), GFP_KERNEL);
	init_data = kzalloc(sizeof(struct regulator_init_data), GFP_KERNEL);
	rda_regs_desc = kzalloc(sizeof(struct regulator_desc), GFP_KERNEL);
	
	consumer_supply = kzalloc(sizeof(struct regulator_consumer_supply), GFP_KERNEL);
	reg_default = kzalloc(sizeof(struct rda_reg_def), GFP_KERNEL);
	config = kzalloc(sizeof(struct regulator_config), GFP_KERNEL);
	
	suply_name = kzalloc(sizeof(char) * 15, GFP_KERNEL);
	
	
	if (!rda_reg || !rda_config || !init_data || !rda_regs_desc || !suply_name) {
		dev_err(&pdev->dev, "Unable to allocate private data\n");
		return -ENOMEM;
	}
	
	rda_regs_desc->id = GetDeviceTreeProperty(pdev, "id");
	GetDeviceTreeStringProperty(pdev, suply_name, "suply_consumer_name");
	rda_regs_desc->name = suply_name;
	rda_regs_desc->supply_name = suply_name;
	rda_regs_desc->ops = &rda_regulator_ops;
	
	if (GetDeviceTreeProperty(pdev, "regulator_type"))
	{
		rda_regs_desc->type	= REGULATOR_CURRENT;
	}
	else 
	{
		rda_regs_desc->type	= REGULATOR_VOLTAGE;
	}
	
	rda_regs_desc->owner= THIS_MODULE;
	rda_regs_desc->regulators_node = NULL;
	rda_regs_desc->of_match = NULL;
	rda_regs_desc->poll_enabled_time = false;
	
	pdev->id = rda_regs_desc->id;
	ret = GetArrayProperty(pdev, "vtable", &ArrayTmp);
	if (ret)
	{
		dev_err(&pdev->dev, "Unable get vtable\n");
		return -ENOMEM;
	}
	vtable = ArrayTmp.Array;
	consumer_supply->supply = rda_regs_desc->name;
	
	
	reg_default->def_val = GetDeviceTreeProperty(pdev, "def_val");
	rda_reg->config = rda_config;
	rda_reg->config->pm_id = GetDeviceTreeProperty(pdev, "pm_id");
	rda_reg->config->msys_cmd = GetDeviceTreeProperty(pdev, "msys_cmd");
	rda_reg->config->table = (void *)vtable;
	rda_reg->config->tsize = ArrayTmp.Num;
	
	init_data->constraints.name = rda_regs_desc->name;
	init_data->constraints.min_uV = GetDeviceTreeProperty(pdev, "regulator-min-microvolt");
	init_data->constraints.max_uV = GetDeviceTreeProperty(pdev, "regulator-max-microvolt");
	init_data->constraints.min_uA = 0;
	init_data->constraints.max_uA = 0;
	init_data->constraints.valid_modes_mask = GetDeviceTreeProperty(pdev, "valid_modes_mask");
	init_data->constraints.valid_ops_mask = GetDeviceTreeProperty(pdev, "valid_ops_mask");
	init_data->consumer_supplies = consumer_supply;
	init_data->num_consumer_supplies = 1;
	init_data->regulator_init = rda_regulator_do_init;
	init_data->driver_data = reg_default;
	init_data->supply_regulator = NULL;
	
	
	rda_reg->config->init_data = init_data;
	
	rda_reg->private = (void *)pdev;

	
	/* Set the size of items. */
	rda_regs_desc->n_voltages = rda_reg->config->tsize;

	rda_reg->msys_dev = rda_msys_alloc_device();
	if (!rda_reg->msys_dev) {
		ret = -EINVAL;
		goto out;
	}
	
	rda_reg->msys_dev->module = SYS_PM_MOD;
	rda_reg->msys_dev->name = rda_regs_desc->name;
	rda_msys_register_device(rda_reg->msys_dev);
	
	config->dev = &pdev->dev;
	config->of_node = pdev->dev.of_node;
	config->driver_data = rda_reg;
	config->init_data = rda_reg->config->init_data;
	rda_reg->rdev = devm_regulator_register(&pdev->dev, rda_regs_desc,config);
	if (IS_ERR(rda_reg->rdev)) {
		ret = PTR_ERR(rda_reg->rdev);
		dev_err(&pdev->dev, "Failed to register withe regulator!\n");
		goto out;
	}
	platform_set_drvdata(pdev, rda_reg);

	return 0;

out:
	if (rda_reg->msys_dev) {
		rda_msys_unregister_device(rda_reg->msys_dev);
		rda_msys_free_device(rda_reg->msys_dev);
	}

	if (rda_reg) {
		kfree(rda_reg);
	}
	if (rda_config) {
		kfree(rda_config);
	}
	
	if (init_data) {
		kfree(init_data);
	}
	
	if (rda_regs_desc) {
		kfree(rda_regs_desc);
	}
	
	if (consumer_supply) {
		kfree(consumer_supply);
	}
	
	if (reg_default) {
		kfree(reg_default);
	}
	
	if (config) {
		kfree(config);
	}
	
	if (suply_name) {
		kfree(suply_name);
	}
	
	if (vtable) {
		kfree(vtable);
	}
	
	return ret;
}

static int __exit rda_regulator_remove(struct platform_device *pdev)
{
	struct rda_regulator *rda_reg = platform_get_drvdata(pdev);
	struct regulator_dev *rdev = rda_reg->rdev;
	struct msys_device *msys_dev = rda_reg->msys_dev;

	pr_debug("<rda-regulator> : remove\n");

	if (msys_dev) {
		rda_msys_unregister_device(msys_dev);
		rda_msys_free_device(msys_dev);
	}

	if (rdev) {
		regulator_unregister(rdev);
	}

	platform_set_drvdata(pdev, NULL);

	kfree(rda_reg);

	return 0;
}

static const struct of_device_id rda_regulator_matches[] = {
	{ .compatible = "rda,8810pl-regulator" },
	{ }
};
MODULE_DEVICE_TABLE(of, rda_regulator_matches);

static struct platform_driver rda_regulator_driver = {
	.driver = {
		.name	= "regulator-rda",
		.owner	= THIS_MODULE,
		.of_match_table = rda_regulator_matches,
	},
	.probe	= rda_regulator_probe,
	.remove	= rda_regulator_remove,
	
};

static int rda_pm_restart(struct notifier_block *nb, unsigned long action,  void *data)
{
	struct client_cmd cmd_set;
	struct rda_pm_param param;
	unsigned int ret;
	HWP_SYS_CTRL_T *pctrl = (HWP_SYS_CTRL_T *)rda_sysctrl;

	//rda_set_boot_mode(cmd);

	/* Power-off backlight firstly. */
	memset(&cmd_set, 0, sizeof(cmd_set));
	cmd_set.pmsys_dev = rda_power_ctrl.msys_dev;
	cmd_set.mod_id = SYS_PM_MOD;
	cmd_set.mesg_id = SYS_PM_CMD_SET_LEVEL;

	param.pm_id = 2;
	param.pm_val = 0;

	cmd_set.pdata = (void *)&param;
	cmd_set.data_size = sizeof(param);

	ret = rda_msys_send_cmd(&cmd_set);
	if (ret > 0) {
		/* Just print a warning and continue to reboot. */
		printk("<rda-reg> : Failure as setting backlight!\n");
	}

	memset(&cmd_set, 0, sizeof(cmd_set));
	cmd_set.pmsys_dev = rda_power_ctrl.msys_dev;
	cmd_set.mod_id = SYS_GEN_MOD;
	cmd_set.mesg_id = SYS_GEN_CMD_RESET;

	rda_msys_send_cmd_timeout(&cmd_set, 6000);

	/* Try to reboot by accessing modem registers directly */
	for (;;) {
		pctrl->REG_DBG = SYS_CTRL_PROTECT_UNLOCK;
		pctrl->Sys_Rst_Set= SYS_CTRL_SOFT_RST;
	}

	return NOTIFY_DONE;
}

static void rda_pm_power_off(void)
{
	struct client_cmd cmd_set;
	HWP_SYS_CTRL_T *pctrl = (HWP_SYS_CTRL_T *)rda_sysctrl;

	memset(&cmd_set, 0, sizeof(cmd_set));

	cmd_set.pmsys_dev = rda_power_ctrl.msys_dev;
	cmd_set.mod_id = SYS_GEN_MOD;
	cmd_set.mesg_id = SYS_GEN_CMD_SHDW;

	rda_msys_send_cmd_timeout(&cmd_set, 6000);

	/*
	 * Failed to power off
	 * Try to reboot by accessing modem registers directly
	 */
	for (;;) {
		pctrl->REG_DBG = SYS_CTRL_PROTECT_UNLOCK;
		pctrl->Sys_Rst_Set= SYS_CTRL_SOFT_RST;
	}

	return;
}

static struct notifier_block rda_pm_restart_nb = {
	.notifier_call = rda_pm_restart,
	.priority = 129,
};

void rda_init_shdw(void)
{
	pm_power_off = rda_pm_power_off;
	register_restart_handler(&rda_pm_restart_nb);

	return;
}

static int __init rda_regulator_init(void)
{
	rda_sysctrl = ioremap(RDA_MD_SYSCTRL_PHYS, RDA_MD_SYSCTRL_SIZE);
	if (!rda_sysctrl) {
		return -ENXIO;
	}

	rda_power_ctrl.msys_dev = rda_msys_alloc_device();
	if (!rda_power_ctrl.msys_dev) {
		iounmap(rda_sysctrl);
		return EINVAL;
	}

	rda_power_ctrl.msys_dev->module = SYS_GEN_MOD;
	rda_power_ctrl.msys_dev->name = "rda_power_ctrl";
	rda_msys_register_device(rda_power_ctrl.msys_dev);
	
	rda_init_shdw();

	return platform_driver_register(&rda_regulator_driver);
}





void rda_ap_reset(void)
{
	HWP_SYS_CTRL_T *pctrl = (HWP_SYS_CTRL_T *)rda_sysctrl;

	for (;;) {
		pctrl->REG_DBG = SYS_CTRL_PROTECT_UNLOCK;
		pctrl->Sys_Rst_Set= SYS_CTRL_SOFT_RST;
	}
}



subsys_initcall(rda_regulator_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("feifanqi <feifanqi@rdamicro.com>");
MODULE_DESCRIPTION("RDA Regulator Driver");

