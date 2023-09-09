#include <linux/module.h>
#include <linux/i2c.h>

#include "rda_combo.h"

struct i2c_client *rda_wifi_rf_client = NULL;

extern u32 rda_wlan_version(void);

static int rda_wifi_rf_probe(struct i2c_client *client)
{
	int result = 0;
	struct device *dev = &client->dev;
	u32 version;

	rda_wifi_rf_client = client;
	version = rda_wlan_version();
	dev_info(dev, "rda_wifi_rf_probe rda_wlan_version = %lu\n", (unsigned long)version);
	return result;
}

static void rda_wifi_rf_remove(struct i2c_client *client)
{
	printk(KERN_INFO "rda_wifi_rf_remove \n");
	
}

static int rda_wifi_rf_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	printk(KERN_INFO "rda_wifi_rf_detect \n");
	strcpy(info->type, RDA_WIFI_RF_I2C_DEVNAME);
	return 0;
}



static const struct i2c_device_id wifi_rf_i2c_id[] ={
	{"rda_wifi_rf_i2c", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, wifi_rf_i2c_id);

static const struct of_device_id rda_wifi_rf_i2c_compatible[] = {
	{ .compatible = "rda_wifi_rf_i2c" },
	{ }
};
MODULE_DEVICE_TABLE(of, rda_wifi_rf_i2c_compatible);

static struct i2c_driver rda_wifi_rf_driver = {
	.driver = {
		.name = "rda_wifi_rf_i2c",
		.of_match_table = rda_wifi_rf_i2c_compatible,
	},
	.class = I2C_CLASS_HWMON,
	.probe_new = rda_wifi_rf_probe,
	.remove = rda_wifi_rf_remove,
	.detect = rda_wifi_rf_detect,
	.id_table = wifi_rf_i2c_id,
};


module_i2c_driver(rda_wifi_rf_driver);

MODULE_LICENSE("GPL");