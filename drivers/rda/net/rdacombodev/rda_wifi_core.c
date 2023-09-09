#include <linux/module.h>
#include <linux/i2c.h>

#include "rda_combo.h"

struct i2c_client *rda_wifi_core_client = NULL;

extern void rda_wifi_power_off(void);

static int rda_wifi_core_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	printk(KERN_INFO "rda_wifi_core_detect \n");
	strcpy(info->type, RDA_WIFI_CORE_I2C_DEVNAME);
	return 0;
}

static int rda_wifi_core_probe(struct i2c_client *client)
{
	int result = 0;
	struct device *dev = &client->dev;

	rda_wifi_core_client = client;
	dev_info(dev, "rda_wifi_core_probe \n");
	return result;
}

static void rda_wifi_core_remove(struct i2c_client *client)
{
	printk(KERN_INFO "rda_wifi_core_remove \n");
	
}



static void rda_wifi_shutdown(struct i2c_client *client)
{
	printk(KERN_INFO "rda_wifi_shutdown \n");
#ifdef RDA_COMBO_FROM_FIRMWARE
	if(atomic_read(&wifi_fw_status))
#endif
		rda_wifi_power_off();
}

static const struct i2c_device_id wifi_core_i2c_id[] = {
	{"rda_wifi_core_i2c", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, wifi_core_i2c_id);

static const struct of_device_id rda_wifi_core_i2c_compatible[] = {
	{ .compatible = "rda_wifi_core_i2c" },
	{ }
};
MODULE_DEVICE_TABLE(of, rda_wifi_core_i2c_compatible);


static struct i2c_driver rda_wifi_core_driver = {
	.driver = {
		.name = "rda_wifi_core_i2c",
		.of_match_table = rda_wifi_core_i2c_compatible,
	},
	.class = I2C_CLASS_HWMON,
	.probe_new = rda_wifi_core_probe,
	.remove = rda_wifi_core_remove,
	.detect = rda_wifi_core_detect,
	.shutdown = rda_wifi_shutdown,
	.id_table = wifi_core_i2c_id,
};




module_i2c_driver(rda_wifi_core_driver);

MODULE_LICENSE("GPL");