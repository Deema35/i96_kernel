#include <linux/module.h>
#include <linux/i2c.h>

#include "rda_combo.h"

struct i2c_client *rda_bt_core_client = NULL;

static int rda_bt_core_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	printk(KERN_INFO "rda_bt_core_detect \n");
	strcpy(info->type, RDA_BT_CORE_I2C_DEVNAME);
	return 0;
}

//void rda_combo_set_wake_lock(void);

#ifdef CONFIG_BLUEZ_SUPPORT
extern void hci_bt_wakeup_host(void);
#endif



static int rda_bt_core_probe(struct i2c_client *client)
{
	int result = 0;
	struct device *dev = &client->dev;
	
	rda_bt_core_client = client;
	dev_info(dev, "rda_bt_core_probe \n");
	return result;
}

static void rda_bt_core_remove(struct i2c_client *client)
{
	printk(KERN_INFO "rda_bt_core_remove \n");
	
}

static const struct i2c_device_id bt_core_i2c_id[] ={
	{RDA_BT_CORE_I2C_DEVNAME, RDA_I2C_CHANNEL},
	{}
};

MODULE_DEVICE_TABLE(i2c, bt_core_i2c_id);

static const struct of_device_id rda_bt_core_i2c_compatible[] = {
	{ .compatible = RDA_BT_CORE_I2C_DEVNAME },
	{ }
};
MODULE_DEVICE_TABLE(of, rda_bt_core_i2c_compatible);

static struct i2c_driver rda_bt_core_driver = {
	.driver = {
		.name = RDA_BT_CORE_I2C_DEVNAME,
		.of_match_table = rda_bt_core_i2c_compatible,
	},
	.class = I2C_CLASS_HWMON,
	.probe_new = rda_bt_core_probe,
	.remove = rda_bt_core_remove,
	.detect = rda_bt_core_detect,
	.id_table = bt_core_i2c_id,
};



module_i2c_driver(rda_bt_core_driver);
MODULE_LICENSE("GPL");