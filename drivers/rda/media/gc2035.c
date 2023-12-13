// SPDX-License-Identifier: GPL-2.0-only
/*
 * gc2035 Camera Driver
 *
 * Copyright (C) 2010 Alberto Panizzo <maramaopercheseimorto@gmail.com>
 *
 * Based on ov772x, ov9640 drivers and previous non merged implementations.
 *
 * Copyright 2005-2009 Freescale Semiconductor, Inc. All Rights Reserved.
 * Copyright (C) 2006, OmniVision
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/clk.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/v4l2-mediabus.h>
#include <linux/videodev2.h>
#include <rda/mach/regulator.h>

#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-image-sizes.h>

#include "rda_sensor.h"

/*
 * Struct
 */
 
 #define BIT	8
 
extern void rcam_pdn(bool pdn);
extern void rcam_rst(bool pdn);
extern void rcam_clk(bool out, int freq);


struct gc2035_win_size {
	char				*name;
	u32				width;
	u32				height;
	const struct sensor_reg	*regs;
};


struct gc2035_priv
{
	struct v4l2_subdev		subdev;
#if defined(CONFIG_MEDIA_CONTROLLER)
	struct media_pad pad;
#endif
	struct v4l2_ctrl_handler	hdl;
	u32	cfmt_code;
	struct clk			*clk;
	const struct gc2035_win_size	*win;

	struct gpio_desc *resetb_gpio;
	struct gpio_desc *pwdn_gpio;
	struct regulator *cam_reg;

	struct mutex lock; /* lock to protect streaming and power_count */
	bool streaming;
	int power_count;
	int exp_def;
	int awb_def;
};

/*
 * Registers settings
 */

//#define ENDMARKER { 0xff, 0xff }

static struct sensor_reg init_gc2035[] =
{
	{0xfe,0x80,BIT,0},
	{0xfe,0x80,BIT,0},
	{0xfe,0x80,BIT,0},
	{0xfc,0x06,BIT,0},
	{0xf9,0xfe,BIT,0},
	{0xf6,0x00,BIT,0},

	{0xf7,0x05,BIT,0},
	{0xf8,0x84,BIT,0},//0x85
	{0xfa,0x00,BIT,0},

	{0xfe,0x00,BIT,0},
	{0x82,0x00,BIT,0},
	{0xb3,0x60,BIT,0},
	{0xb4,0x40,BIT,0},
	{0xb5,0x60,BIT,0},
	{0x03,0x05,BIT,0},
	{0x04,0x08,BIT,0},
	{0xfe,0x00,BIT,0},
	{0xec,0x04,BIT,0},
	{0xed,0x04,BIT,0},
	{0xee,0x60,BIT,0},
	{0xef,0x90,BIT,0},
	{0x0a,0x00,BIT,0},
	{0x0c,0x00,BIT,0},
	{0x0d,0x04,BIT,0},
	{0x0e,0xc0,BIT,0},
	{0x0f,0x06,BIT,0},
	{0x10,0x58,BIT,0},
	{0x17,0x14,BIT,0},//0x14
	{0x18,0x0a,BIT,0},
	{0x19,0x0c,BIT,0},
	{0x1a,0x01,BIT,0},
	{0x1b,0x48,BIT,0},
	{0x1e,0x88,BIT,0},
	{0x1f,0x0f,BIT,0},
	{0x20,0x05,BIT,0},
	{0x21,0x0f,BIT,0},
	{0x22,0xf0,BIT,0},
	{0x23,0xc3,BIT,0},
	{0x24,0x16,BIT,0},
	{0xfe,0x01,BIT,0},
	{0x09,0x00,BIT,0},
	{0x0b,0x90,BIT,0},
	{0x13,0x74,BIT,0},
	{0xfe,0x00,BIT,0},
	{0xfe,0x00,BIT,0},
#if 1 //26
	{0x05,0x01,BIT,0},
	{0x06,0x05,BIT,0},
	{0x07,0x00,BIT,0},
	{0x08,0x72,BIT,0},
	{0xfe,0x01,BIT,0},
	{0x27,0x00,BIT,0},
	{0x28,0x92,BIT,0},
	{0x29,0x05,BIT,0},
	{0x2a,0x22,BIT,0},
	{0x2b,0x05,BIT,0},
	{0x2c,0xb4,BIT,0},
	{0x2d,0x07,BIT,0},
	{0x2e,0x6a,BIT,0},
	{0x2f,0x0a,BIT,0},
	{0x30,0x44,BIT,0},

	{0x3e,0x40,BIT,0},
#endif
#if 0 //24
	{0x05,0x01,BIT,0},
	{0x06,0x25,BIT,0},
	{0x07,0x00,BIT,0},
	{0x08,0x14,BIT,0},
	{0xfe,0x01,BIT,0},
	{0x27,0x00,BIT,0},
	{0x28,0x83,BIT,0},
	{0x29,0x04,BIT,0},
	{0x2a,0x9b,BIT,0},
	{0x2b,0x04,BIT,0},
	{0x2c,0x9b,BIT,0},
	{0x2d,0x05,BIT,0},
	{0x2e,0xa1,BIT,0},
	{0x2f,0x07,BIT,0},
	{0x30,0x2a,BIT,0},

	{0x3e,0x40,BIT,0},
#endif
	{0xfe,0x00,BIT,0},
	{0xb6,0x03,BIT,0},
	{0xfe,0x00,BIT,0},
	{0x3f,0x00,BIT,0},
	{0x40,0x77,BIT,0},
	{0x42,0x7f,BIT,0},
	{0x43,0x30,BIT,0},
	{0x5c,0x08,BIT,0},
	{0x5e,0x20,BIT,0},
	{0x5f,0x20,BIT,0},
	{0x60,0x20,BIT,0},
	{0x61,0x20,BIT,0},
	{0x62,0x20,BIT,0},
	{0x63,0x20,BIT,0},
	{0x64,0x20,BIT,0},
	{0x65,0x20,BIT,0},
	{0x66,0x20,BIT,0},
	{0x67,0x20,BIT,0},
	{0x68,0x20,BIT,0},
	{0x69,0x20,BIT,0},
	{0x90,0x01,BIT,0},
	{0x95,0x04,BIT,0},
	{0x96,0xb0,BIT,0},
	{0x97,0x06,BIT,0},
	{0x98,0x40,BIT,0},
	{0xfe,0x03,BIT,0},
	{0x42,0x80,BIT,0},
	{0x43,0x06,BIT,0},
	{0x41,0x00,BIT,0},
	{0x40,0x00,BIT,0},
	{0x17,0x01,BIT,0},
	{0xfe,0x00,BIT,0},
	{0x80,0xff,BIT,0},
	{0x81,0x26,BIT,0},
	{0x03,0x05,BIT,0},
	{0x04,0x2e,BIT,0},

	{0x84,0x02,BIT,0},
	{0x86,0x02,BIT,0},
	{0x87,0x80,BIT,0},
	{0x8b,0xbc,BIT,0},
	{0xa7,0x80,BIT,0},
	{0xa8,0x80,BIT,0},
	{0xb0,0x80,BIT,0},
	{0xc0,0x40,BIT,0},
	{0xfe,0x01,BIT,0},
	{0xc2,0x1e,BIT,0},
	{0xc3,0x10,BIT,0},
	{0xc4,0x09,BIT,0},
	{0xc8,0x16,BIT,0},
	{0xc9,0x0a,BIT,0},
	{0xca,0x00,BIT,0},
	{0xbc,0x33,BIT,0},
	{0xbd,0x12,BIT,0},
	{0xbe,0x0d,BIT,0},
	{0xb6,0x30,BIT,0},
	{0xb7,0x18,BIT,0},
	{0xb8,0x00,BIT,0},
	{0xc5,0x00,BIT,0},
	{0xc6,0x00,BIT,0},
	{0xc7,0x00,BIT,0},
	{0xcb,0x00,BIT,0},
	{0xcc,0x0b,BIT,0},
	{0xcd,0x16,BIT,0},
	{0xbf,0x00,BIT,0},
	{0xc0,0x00,BIT,0},
	{0xc1,0x00,BIT,0},
	{0xb9,0x0c,BIT,0},
	{0xba,0x00,BIT,0},
	{0xbb,0x12,BIT,0},
	{0xaa,0x00,BIT,0},
	{0xab,0x00,BIT,0},
	{0xac,0x00,BIT,0},
	{0xad,0x12,BIT,0},
	{0xae,0x00,BIT,0},
	{0xaf,0x00,BIT,0},
	{0xb0,0x00,BIT,0},
	{0xb1,0x00,BIT,0},
	{0xb2,0x00,BIT,0},
	{0xb3,0x00,BIT,0},
	{0xb4,0x00,BIT,0},
	{0xb5,0x06,BIT,0},
	{0xd0,0x00,BIT,0},
	{0xd2,0x0e,BIT,0},
	{0xd3,0x0b,BIT,0},
	{0xd8,0x21,BIT,0},
	{0xda,0x18,BIT,0},
	{0xdb,0x18,BIT,0},
	{0xdc,0x00,BIT,0},
	{0xde,0x09,BIT,0},
	{0xdf,0x00,BIT,0},
	{0xd4,0x08,BIT,0},
	{0xd6,0x0e,BIT,0},
	{0xd7,0x00,BIT,0},
	{0xa4,0x00,BIT,0},
	{0xa5,0x00,BIT,0},
	{0xa6,0x40,BIT,0},
	{0xa7,0x00,BIT,0},
	{0xa8,0x00,BIT,0},
	{0xa9,0x40,BIT,0},
	{0xa1,0x80,BIT,0},
	{0xa2,0x80,BIT,0},
	{0xfe,0x02,BIT,0},
	{0xa4,0x00,BIT,0},
	{0xfe,0x00,BIT,0},
	{0xfe,0x02,BIT,0},
	{0xc0,0x01,BIT,0},
	{0xc1,0x40,BIT,0},
	{0xc2,0xfc,BIT,0},
	{0xc3,0x05,BIT,0},
	{0xc4,0xec,BIT,0},
	{0xc5,0x42,BIT,0},
	{0xc6,0xf8,BIT,0},
	{0xc7,0x40,BIT,0},
	{0xc8,0xf8,BIT,0},
	{0xc9,0x06,BIT,0},
	{0xca,0xfd,BIT,0},
	{0xcb,0x3e,BIT,0},
	{0xcc,0xf3,BIT,0},
	{0xcd,0x36,BIT,0},
	{0xce,0xf6,BIT,0},
	{0xcf,0x04,BIT,0},
	{0xe3,0x0c,BIT,0},
	{0xe4,0x44,BIT,0},
	{0xe5,0xe5,BIT,0},
	{0xfe,0x00,BIT,0},
	{0xfe,0x01,BIT,0},
	{0x4f,0x00,BIT,0},
	{0x4d,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4d,0x10,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4d,0x20,BIT,0},///////////20
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4d,0x30,BIT,0},/////////////30
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x02,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4d,0x40,BIT,0},/////////////////40
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x04,BIT,0},
	{0x4e,0x02,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4d,0x50,BIT,0},////////////50
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x08,BIT,0},
	{0x4e,0x08,BIT,0},
	{0x4e,0x04,BIT,0},
	{0x4e,0x04,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4d,0x60,BIT,0},///////////60
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x20,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4d,0x70,BIT,0},/////////////70
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x20,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4d,0x80,BIT,0},//////////////80
	{0x4e,0x00,BIT,0},
	{0x4e,0x20,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4d,0x90,BIT,0},////////////////////90
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4d,0xa0,BIT,0},///////////////a0
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4d,0xb0,BIT,0},///////////////////////////////b0
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4d,0xc0,BIT,0},////////////////////////////////c0
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4d,0xd0,BIT,0},/////////////////////////d0
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4d,0xe0,BIT,0},//////////////////////////////e0
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4d,0xf0,BIT,0},////////////////////////////////f0
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4e,0x00,BIT,0},
	{0x4f,0x01,BIT,0},
	{0x50,0x80,BIT,0},
	{0x56,0x06,BIT,0},

//	{0x50,0x80,BIT,0},
	{0x52,0x40,BIT,0},
	{0x54,0x60,BIT,0},
//	{0x56,0x00,BIT,0},
	{0x57,0x20,BIT,0},
	{0x58,0x01,BIT,0},
	{0x5b,0x08,BIT,0},
	{0x61,0xaa,BIT,0},
	{0x62,0xaa,BIT,0},
	{0x71,0x00,BIT,0},
	{0x72,0x25,BIT,0},
	{0x74,0x10,BIT,0},
	{0x77,0x08,BIT,0},
	{0x78,0xfd,BIT,0},
	{0x86,0x30,BIT,0},
	{0x87,0x00,BIT,0},
	{0x88,0x04,BIT,0},
	{0x8a,0xc0,BIT,0},
	{0x89,0x71,BIT,0},
	{0x84,0x08,BIT,0},
	{0x8b,0x00,BIT,0},
	{0x8d,0x70,BIT,0},
	{0x8e,0x70,BIT,0},
	{0x8f,0xf4,BIT,0},
	{0xfe,0x00,BIT,0},
	{0x82,0x02,BIT,0},
	{0xfe,0x01,BIT,0},
	{0x21,0xbf,BIT,0},
	{0xfe,0x02,BIT,0},
	{0xa5,0x50,BIT,0},
	{0xa2,0xb0,BIT,0},
	{0xa6,0x50,BIT,0},
	{0xa7,0x30,BIT,0},
	{0xab,0x31,BIT,0},
	{0x88,0x15,BIT,0},
	{0xa9,0x6c,BIT,0},
	{0xb0,0x55,BIT,0},
	{0xb3,0x70,BIT,0},
	{0x8c,0xf6,BIT,0},
	{0x89,0x03,BIT,0},
	{0xde,0xb6,BIT,0},
	{0x38,0x08,BIT,0},
	{0x39,0x50,BIT,0},
	{0xfe,0x00,BIT,0},
	{0x81,0x26,BIT,0},
	{0x87,0x80,BIT,0},
	{0xfe,0x02,BIT,0},
	{0x83,0x00,BIT,0},
	{0x84,0x45,BIT,0},
	{0xd1,0x38,BIT,0},
	{0xd2,0x38,BIT,0},
	{0xdd,0x38,BIT,0},
	{0xfe,0x00,BIT,0},
	{0xfe,0x02,BIT,0},
	{0x2b,0x00,BIT,0},
	{0x2c,0x04,BIT,0},
	{0x2d,0x09,BIT,0},
	{0x2e,0x18,BIT,0},
	{0x2f,0x27,BIT,0},
	{0x30,0x37,BIT,0},
	{0x31,0x49,BIT,0},
	{0x32,0x5c,BIT,0},
	{0x33,0x7e,BIT,0},
	{0x34,0xa0,BIT,0},
	{0x35,0xc0,BIT,0},
	{0x36,0xe0,BIT,0},
	{0x37,0xff,BIT,0},
	{0xfe,0x00,BIT,0},

	//////////////////////////////////////////
	{0xfe,0x01,BIT,0},
	{0x21,0xbf,BIT,0},
	{0xfe,0x02,BIT,0},
	{0xa4,0x00,BIT,0},
	{0xa5,0x40,BIT,0},//lsc_th
	{0xa2,0xa0,BIT,0},//lsc_dec_slope
	{0xa6,0x80,BIT,0},//dd_th
	{0xa7,0x80,BIT,0},//ot_th
	{0xab,0x31,BIT,0},
	{0xa9,0x6f,BIT,0},
	{0xb0,0x99,BIT,0},//0x//edge effect slope low
	{0xb1,0x34,BIT,0},//edge effect slope low
	{0xb3,0x80,BIT,0},//saturation dec slope
	{0xde,0xb6,BIT,0},
	{0x38,0x0f,BIT,0},
	{0x39,0x60,BIT,0},
	{0xfe,0x00,BIT,0},
	{0x81,0x26,BIT,0},
	{0xfe,0x02,BIT,0},
	{0x83,0x00,BIT,0},
	{0x84,0x45,BIT,0},
	////////////YCP//////////
	{0xd1,0x30,BIT,0},//saturation_cb
	{0xd2,0x30,BIT,0},//saturation_Cr
	{0xd3,0x40,BIT,0},//contrast ?
	{0xd4,0x80,BIT,0},//contrast center
	{0xd5,0x00,BIT,0},//luma_offset
	{0xdc,0x30,BIT,0},
	{0xdd,0xb8,BIT,0},//edge_sa_g,b
	{0xfe,0x00,BIT,0},
	///////dndd///////////
	{0xfe,0x02,BIT,0},
	{0x88,0x15,BIT,0},//dn_b_base
	{0x8c,0xf6,BIT,0},//[2]b_in_dark_inc
	{0x89,0x03,BIT,0},//dn_c_weight
	////////EE///////////
	{0xfe,0x02,BIT,0},
	{0x90,0x6c,BIT,0},// EEINTP mode1
	{0x97,0x45,BIT,0},// edge effect

	{0xfe,0x00,BIT,0},
	{0xfe,0x01,BIT,0},
	{0x21,0xff,BIT,0},
	{0xfe,0x02,BIT,0},
	{0x8a,0x33,BIT,0},
	{0x8c,0x76,BIT,0},
	{0x8d,0x85,BIT,0},
	{0xa6,0xf0,BIT,0},
	{0xae,0x9f,BIT,0},
	{0xa2,0x90,BIT,0},
	{0xa5,0x40,BIT,0},
	{0xa7,0x30,BIT,0},
	{0xb0,0x88,BIT,0},
	{0x38,0x0b,BIT,0},
	{0x39,0x30,BIT,0},
	{0xfe,0x00,BIT,0},
	{0x87,0xb0,BIT,0},

	////dark sun/////
	{0xfe,0x02,BIT,0},
	{0x40,0x06,BIT,0},
	{0x41,0x23,BIT,0},
	{0x42,0x3f,BIT,0},
	{0x43,0x06,BIT,0},
	{0x44,0x00,BIT,0},
	{0x45,0x00,BIT,0},
	{0x46,0x14,BIT,0},
	{0x47,0x09,BIT,0},
	{0xfe,0x00,BIT,0},

	////////////////////////////////////////
	{0x82,0xfe,BIT,200},
	//MIPI
	{0xf2,0x00,BIT,0},
	{0xf3,0x00,BIT,0},
	{0xf4,0x00,BIT,0},
	{0xf5,0x00,BIT,0},
	{0xfe,0x01,BIT,0},
	{0x0b,0x90,BIT,0},
	{0x87,0x10,BIT,0},
	{0xfe,0x00,BIT,0},

	{0xfe,0x03,BIT,0},
	{0x01,0x03,BIT,0},//07//2 lanes
	{0x02,0x14,BIT,0},//05 0x14 [6:4] lane0 driver [2:0] clock lane driver
	{0x03,0x11,BIT,0},
	{0x06,0x90,BIT,0},//leo changed
	{0x11,0x1E,BIT,0},
	{0x12,0x80,BIT,0},
	{0x13,0x0c,BIT,0},
	{0x15,0x11,BIT,0},//{0x15, 0x10, 0},// clk_lane mode
	{0x04,0x20,BIT,0},
	{0x05,0x00,BIT,0},
	{0x17,0x00,BIT,0},

	{0x21,0x01,BIT,0},
	{0x22,0x02,BIT,0},//T_CLK_HS_PREPARE_set
	{0x23,0x01,BIT,0},//T_CLK_zero_set
	{0x29,0x02,BIT,0},//T_HS_PREPARE_SET 0x03 01 01,03 05
	{0x2a,0x01,BIT,0},//T_HS_Zero_set
//	{0x2b,0x06,BIT,0},
	{0x10,0x94,BIT,0},//95
	{0xfe,0x00,BIT,0},
};



// use this for 176x144 (QCIF) capture
static struct sensor_reg qcif_gc2035[] =
{
	{0xfe,0x00,BIT,0},
	{0xfa,0x00,BIT,0},
	{0xc8,0x00,BIT,0},

	{0x99,0x88,BIT,0},
	{0x9a,0x06,BIT,0},
	{0x9b,0x00,BIT,0},
	{0x9c,0x00,BIT,0},
	{0x9d,0x00,BIT,0},
	{0x9e,0x00,BIT,0},
	{0x9f,0x00,BIT,0},
	{0xa0,0x00,BIT,0},
	{0xa1,0x00,BIT,0},
	{0xa2,0x00,BIT,0},
	{0x0a,0x00,BIT,0},

	{0x90,0x01,BIT,0},
	{0x95,0x00,BIT,0},
	{0x96,0x90,BIT,0},
	{0x97,0x00,BIT,0},
	{0x98,0xb0,BIT,0},

	{0xfe,0x03,BIT,0},
	{0x13,0x01,BIT,0},
	{0x12,0x60,BIT,0},
	{0x05,0x00,BIT,0},
	{0x04,0x24,BIT,0},
	{0xfe,0x00,BIT,0},
};

// use this for 160x120 (QQVGA) capture
static struct sensor_reg qqvga_gc2035[] =
{
	{0xfe,0x00,BIT,0},
	{0xfa,0x00,BIT,0},
	{0xc8,0x00,BIT,0},

	{0x99,0x88,BIT,0},
	{0x9a,0x06,BIT,0},
	{0x9b,0x00,BIT,0},
	{0x9c,0x00,BIT,0},
	{0x9d,0x00,BIT,0},
	{0x9e,0x00,BIT,0},
	{0x9f,0x00,BIT,0},
	{0xa0,0x00,BIT,0},
	{0xa1,0x00,BIT,0},
	{0xa2,0x00,BIT,0},
	{0x0a,0x00,BIT,0},

	{0x90,0x01,BIT,0},
	{0x95,0x00,BIT,0},
	{0x96,0x78,BIT,0},
	{0x97,0x00,BIT,0},
	{0x98,0xa0,BIT,0},

	{0xfe,0x03,BIT,0},
	{0x13,0x01,BIT,0},
	{0x12,0x40,BIT,0},
	{0x05,0x00,BIT,0},
	{0x04,0x24,BIT,0},
	{0xfe,0x00,BIT,0},
};


// use this for 320x240 (QVGA) capture
static struct sensor_reg qvga_gc2035[] =
{
	{0xfe,0x00,BIT,0},
	{0xfa,0x00,BIT,0},
	{0xc8,0x00,BIT,0},

	{0x99,0x44,BIT,0},
	{0x9a,0x06,BIT,0},
	{0x9b,0x00,BIT,0},
	{0x9c,0x00,BIT,0},
	{0x9d,0x00,BIT,0},
	{0x9e,0x00,BIT,0},
	{0x9f,0x00,BIT,0},
	{0xa0,0x00,BIT,0},
	{0xa1,0x00,BIT,0},
	{0xa2,0x00,BIT,0},
	{0x0a,0x00,BIT,0},

	{0x90,0x01,BIT,0},
	{0x95,0x00,BIT,0},
	{0x96,0xf0,BIT,0},
	{0x97,0x01,BIT,0},
	{0x98,0x40,BIT,0},

	{0xfe,0x03,BIT,0},
	{0x13,0x02,BIT,0},
	{0x12,0x80,BIT,0},
	{0x05,0x00,BIT,0},
	{0x04,0x48,BIT,0},
	{0xfe,0x00,BIT,0},

};

// use this for 640x480 (VGA) capture
static struct sensor_reg vga_gc2035[] =
{
	{0xfe,0x00,BIT,0},
	{0xc8,0x00,BIT,0},
	{0xfa,0x00,BIT,0},

	{0x99,0x22,BIT,0},
	{0x9a,0x06,BIT,0},
	{0x9b,0x00,BIT,0},
	{0x9c,0x00,BIT,0},
	{0x9d,0x00,BIT,0},
	{0x9e,0x00,BIT,0},
	{0x9f,0x02,BIT,0},
	{0xa0,0x00,BIT,0},
	{0xa1,0x00,BIT,0},
	{0xa2,0x00,BIT,0},
	{0x0a,0x00,BIT,0},

	{0x90,0x01,BIT,0},
	{0x95,0x01,BIT,0},
	{0x96,0xe0,BIT,0},
	{0x97,0x02,BIT,0},
	{0x98,0x80,BIT,0},

	{0xfe,0x03,BIT,0},
	{0x13,0x05,BIT,0},
	{0x12,0x00,BIT,0},
	{0x05,0x00,BIT,0},
	{0x04,0x90,BIT,0},
	{0xfe,0x00,BIT,0},
};

// use this for 1600x1200 (UXGA) capture
static struct sensor_reg uxga_gc2035[] =
{
	{0xfe,0x00,BIT,0},
	{0xfa,0x11,BIT,0},
	{0xc8,0x00,BIT,0},

	{0x99,0x11,BIT,0},
	{0x9a,0x06,BIT,0},
	{0x9b,0x00,BIT,0},
	{0x9c,0x00,BIT,0},
	{0x9d,0x00,BIT,0},
	{0x9e,0x00,BIT,0},
	{0x9f,0x00,BIT,0},
	{0xa0,0x00,BIT,0},
	{0xa1,0x00,BIT,0},
	{0xa2,0x00,BIT,0},
	{0x0a,0x00,BIT,0},

	{0x90,0x01,BIT,0},
	{0x95,0x04,BIT,0}, //1200=4b0h
	{0x96,0xb0,BIT,0},
	{0x97,0x06,BIT,0}, //1600=640h
	{0x98,0x40,BIT,0},

	{0xfe,0x03,BIT,0},
	{0x13,0x0c,BIT,0},
	{0x12,0x80,BIT,0},
	{0x05,0x00,BIT,0},
	{0x04,0x20,BIT,0},
//	{0x10,0x94,BIT,0},
	{0xfe,0x00,BIT,0},
};

static struct sensor_reg exp_gc2035[][3] =
{
	{{0xfe,0x01,BIT,0},{0x13,0x50,BIT,0},{0xfe,0x00,BIT,0}},
	{{0xfe,0x01,BIT,0},{0x13,0x60,BIT,0},{0xfe,0x00,BIT,0}},
	{{0xfe,0x01,BIT,0},{0x13,0x70,BIT,0},{0xfe,0x00,BIT,0}},
	{{0xfe,0x01,BIT,0},{0x13,0x78,BIT,0},{0xfe,0x00,BIT,0}},
	{{0xfe,0x01,BIT,0},{0x13,0x80,BIT,0},{0xfe,0x00,BIT,0}},
	{{0xfe,0x01,BIT,0},{0x13,0x90,BIT,0},{0xfe,0x00,BIT,0}},
	{{0xfe,0x01,BIT,0},{0x13,0xa0,BIT,0},{0xfe,0x00,BIT,0}},
};

static struct sensor_reg awb_gc2035[][4] =
{
	{{0x82,0xfc,BIT,0},{0xb3,0x61,BIT,0},{0xb4,0x40,BIT,0},{0xb5,0x61,BIT,0}},//OFF
	{{0x82,0xfe,BIT,0},{0xb3,0x61,BIT,0},{0xb4,0x40,BIT,0},{0xb5,0x61,BIT,0}},//AUTO
	{{0x82,0xfc,BIT,0},{0xb3,0x50,BIT,0},{0xb4,0x40,BIT,0},{0xb5,0xa8,BIT,0}},//INCANDESCENT
	{{0x82,0xfc,BIT,0},{0xb3,0x4e,BIT,0},{0xb4,0x40,BIT,0},{0xb5,0x72,BIT,0}},//FLUORESCENT
	{{0x82,0xfc,BIT,0},{0xb3,0xa0,BIT,0},{0xb4,0x45,BIT,0},{0xb5,0x40,BIT,0}},//TUNGSTEN
	{{0x82,0xfc,BIT,0},{0xb3,0x70,BIT,0},{0xb4,0x40,BIT,0},{0xb5,0x50,BIT,0}},//DAYLIGHT
	{{0x82,0xfc,BIT,0},{0xb3,0x58,BIT,0},{0xb4,0x40,BIT,0},{0xb5,0x50,BIT,0}},//CLOUD
};

/*
 * Register settings for window size
 * The preamble, setup the internal DSP to input an UXGA (1600x1200) image.
 * Then the different zooming configurations will setup the output image size.
 */
/*static const struct sensor_reg gc2035_size_change_preamble_regs[] = {
	{ BANK_SEL, BANK_SEL_DSP ,BIT,0},
	{ RESET, RESET_DVP ,BIT,0},
	{ SIZEL, SIZEL_HSIZE8_11_SET(UXGA_WIDTH) |  SIZEL_HSIZE8_SET(UXGA_WIDTH) |  SIZEL_VSIZE8_SET(UXGA_HEIGHT) ,BIT,0},
	{ HSIZE8, HSIZE8_SET(UXGA_WIDTH) ,BIT,0},
	{ VSIZE8, VSIZE8_SET(UXGA_HEIGHT) ,BIT,0},
	{ CTRL2, CTRL2_DCW_EN | CTRL2_SDE_EN | CTRL2_UV_AVG_EN | CTRL2_CMX_EN | CTRL2_UV_ADJ_EN ,BIT,0},
	{ HSIZE, HSIZE_SET(UXGA_WIDTH) ,BIT,0},
	{ VSIZE, VSIZE_SET(UXGA_HEIGHT) ,BIT,0},
	{ XOFFL, XOFFL_SET(0) ,BIT,0},
	{ YOFFL, YOFFL_SET(0) ,BIT,0},
	{ VHYX, VHYX_HSIZE_SET(UXGA_WIDTH) | VHYX_VSIZE_SET(UXGA_HEIGHT) | VHYX_XOFF_SET(0) | VHYX_YOFF_SET(0) ,BIT,0},
	{ TEST, TEST_HSIZE_SET(UXGA_WIDTH) ,BIT,0},
	ENDMARKER,
};*/


#define gc2035_SIZE(n, w, h, r) \
	{.name = n, .width = w , .height = h, .regs = r }

static const struct gc2035_win_size gc2035_supported_win_sizes[] = {
	gc2035_SIZE("QCIF", QCIF_WIDTH, QCIF_HEIGHT, qcif_gc2035), // use this for 176x144 (QCIF) capture
	gc2035_SIZE("QQVGA", QQVGA_WIDTH, QQVGA_HEIGHT, qqvga_gc2035), // use this for 160x120 (QQVGA) capture
	gc2035_SIZE("QVGA", QVGA_WIDTH, QVGA_HEIGHT, qvga_gc2035), // use this for 320x240 (QVGA) capture
	gc2035_SIZE("VGA", VGA_WIDTH, VGA_HEIGHT, vga_gc2035), // use this for 640x480 (VGA) capture
	gc2035_SIZE("UXGA", UXGA_WIDTH, UXGA_HEIGHT, uxga_gc2035), // use this for 1600x1200 (UXGA) capture
};

/*
 * Register settings for pixel formats
 */


static u32 gc2035_codes[] = {
	MEDIA_BUS_FMT_YUYV8_2X8,
	MEDIA_BUS_FMT_YVYU8_2X8,
	MEDIA_BUS_FMT_UYVY8_2X8,
	MEDIA_BUS_FMT_VYUY8_2X8,
	MEDIA_BUS_FMT_SBGGR8_1X8,
};

/*
 * General functions
 */
/*static struct gc2035_priv *to_gc2035(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct gc2035_priv, subdev);
}

static int gc2035_write_array(struct i2c_client *client, const struct sensor_reg *vals)
{
	int ret;

	while ((vals->reg_num != 0xff) || (vals->value != 0xff)) {
		
		ret = i2c_smbus_write_byte_data(client, vals->reg_num, vals->value);
		
		dev_vdbg(&client->dev, "array: 0x%02x, 0x%02x",  vals->reg_num, vals->value);

		if (ret < 0)
			return ret;
		vals++;
	}
	return 0;
}*/

int sensor_i2c_read(struct i2c_client *client, const u8 addr, u16 * data)
{
	unsigned char DATA[2];
	int ret = 0;


	for (int i = 0; i < 3; i++)
	{
		ret = i2c_master_send(client, (char *)&addr, 1);
		if (ret >= 0) {
			break;
		}
	}

	if (ret < 0) {
		printk(KERN_INFO "***i2c_read_1_addr_2_data send:0x%X err:%d\n",
			addr, ret);
		return -1;
	}

	for (int i = 0; i < 3; i++)
	{
		ret = i2c_master_recv(client, DATA, 2);
		if (ret >= 0) {
			break;
		}
	}

	if (ret < 0) {
		printk(KERN_INFO "***i2c_read_1_addr_2_data send:0x%X err:%d\n",
			   addr, ret);
		return -1;
	}

	if (0) {
		*data = (DATA[0] << 8) | DATA[1];
	} else {
		*data = (DATA[1] << 8) | DATA[0];
	}
	return 0;
}


/*static int sensor_i2c_read(const struct i2c_client *client, const u16 addr, u8 *data, u8 bits)
{
	unsigned char tmp[2];
	int len, ret;

	if (!client)
		return -ENODEV;

	if (bits == 8) {
		tmp[0] = addr & 0xff;
		len = 1;
	} else if (bits == 16) {
		tmp[0] = addr >> 8;
		tmp[1] = addr & 0xff;
		len = 2;
	} else {
		printk(KERN_ERR "%s: bits %d not support\n",
				__func__, bits);
		return -EINVAL;
	}

	ret = i2c_master_send(client, tmp, len);
	if (ret < len) {
		printk(KERN_ERR "%s: i2c read error_1, reg: 0x%x\n",
				__func__, addr);
		return ret < 0 ? ret : -EIO;
	}

	ret = i2c_master_recv(client, data, 1);
	if (ret < 1) {
		printk(KERN_ERR "%s: i2c read error_2, reg: 0x%x\n",
				__func__, addr);
		return ret < 0 ? ret : -EIO;
	}

	return 0;
}*/

static int sensor_i2c_write(const struct i2c_client *client, const u16 addr, const u8 data, u8 bits)
{
	unsigned char tmp[3];
	int len, ret;

	if (!client)
		return -ENODEV;

	if (bits == 8) {
		tmp[0] = addr & 0xff;
		tmp[1] = data;
		len = 2;
	} else if (bits == 16) {
		tmp[0] = addr >> 8;
		tmp[1] = addr & 0xff;
		tmp[2] = data;
		len = 3;
	} else {
		printk(KERN_ERR "%s: bits %d not support\n",
				__func__, bits);
		return -EINVAL;
	}

	ret = i2c_master_send(client, tmp, len);
	if (ret < len) {
		printk(KERN_ERR "%s: i2c write error, reg: 0x%x\n", __func__, addr);
		return ret < 0 ? ret : -EIO;
	}

	return 0;
}

static int sensor_write_array(const struct i2c_client *client, struct sensor_reg_list *regs)
{
	int i, res;
	struct sensor_reg *tmp = NULL;

	for (i = 0; i < regs->size; i++) {
		tmp = regs->val + i;
		res = sensor_i2c_write(client, tmp->addr, tmp->data, tmp->bits);
		if (res != 0)
			return res;
		if (tmp->wait)
			mdelay(tmp->wait);
	}

	return 0;
}

static int gc2035_write_array_count(struct i2c_client *client, struct sensor_reg *vals, int count)
{
	int ret;
	struct sensor_reg *tmp = NULL;
	
	for (int i = 0; i < count; i++)
	{
		tmp = vals + i;
		//printk(KERN_ERR "addr = 0x%x data = 0x%x bit = 0x%x\n",  tmp->addr, tmp->data, tmp->bits);
		ret = sensor_i2c_write(client, tmp->addr, tmp->data, tmp->bits);
		
		//ret = i2c_smbus_write_byte_data(client, vals->reg_num, vals->value);
		
		if (ret < 0) return ret;
		
		vals++;
	}
	return 0;
}

/*static int gc2035_mask_set(struct i2c_client *client,  u8  reg, u8  mask, u8  set)
{
	s32 val = i2c_smbus_read_byte_data(client, reg);
	if (val < 0)
		return val;

	val &= ~mask;
	val |= set & mask;

	dev_vdbg(&client->dev, "masks: 0x%02x, 0x%02x", reg, val);

	return i2c_smbus_write_byte_data(client, reg, val);
}*/

/*static int gc2035_reset(struct i2c_client *client)
{
	int ret;
	static const struct regval_list reset_seq[] = {
		{BANK_SEL, BANK_SEL_SENS, BIT,0},
		{COM7, COM7_SRST, BIT,0},
		ENDMARKER,
	};

	ret = gc2035_write_array(client, reset_seq);
	if (ret)
		goto err;

	msleep(5);
err:
	dev_dbg(&client->dev, "%s: (ret %d)", __func__, ret);
	return ret;
}*/

static const char * const gc2035_test_pattern_menu[] = {
	"Disabled",
	"Eight Vertical Colour Bars",
};

/*
 * functions
 */

#define GC2035_FLIP_BASE	0x17
#define GC2035_H_FLIP_BIT	0
#define GC2035_V_FLIP_BIT	1
static void gc2035_set_flip(struct i2c_client *client, int hv, int flip)
{

	s32 tmp = i2c_smbus_read_byte_data(client, GC2035_FLIP_BASE);
	
	if (hv) {
		if (flip)
			tmp |= (0x1 << GC2035_V_FLIP_BIT);
		else
			tmp &= ~(0x1 << GC2035_V_FLIP_BIT);
	}
	else {
		if (flip)
			tmp |= (0x1 << GC2035_H_FLIP_BIT);
		else
			tmp &= ~(0x1 << GC2035_H_FLIP_BIT);
	}

	i2c_smbus_write_byte_data(client, GC2035_FLIP_BASE, tmp);
}

#define GC2035_EXP_ROW		ARRAY_ROW(exp_gc2035)
#define GC2035_EXP_COL		ARRAY_COL(exp_gc2035)
static void gc2035_set_exp(struct i2c_client *client, int exp)
{
	int key = exp + (GC2035_EXP_ROW / 2);
	if ((key < 0) || (key > (GC2035_EXP_ROW - 1))) return;
		
	gc2035_write_array_count(client, exp_gc2035[key], GC2035_EXP_COL);
}


#define GC2035_AWB_ROW		ARRAY_ROW(awb_gc2035)
#define GC2035_AWB_COL		ARRAY_COL(awb_gc2035)
static void gc2035_set_awb(struct i2c_client *client, int awb)
{
	if ((awb < 0) || (awb > (GC2035_AWB_ROW - awb))) return;
		
	gc2035_write_array_count(client, awb_gc2035[awb], GC2035_AWB_COL);
}

static int gc2035_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct v4l2_subdev *sd = &container_of(ctrl->handler, struct gc2035_priv, hdl)->subdev;
	struct i2c_client  *client = v4l2_get_subdevdata(sd);
	struct gc2035_priv *priv = container_of(i2c_get_clientdata(client), struct gc2035_priv, subdev);
	
	/*
	 * If the device is not powered up by the host driver, do not apply any
	 * controls to H/W at this time. Instead the controls will be restored
	 * when the streaming is started.
	 */
	if (!priv->power_count)
		return 0;
	

	switch (ctrl->id)
	{
	case V4L2_CID_HFLIP:
	
		gc2035_set_flip(client, 0, (int)ctrl->val);
		
		break;
	case V4L2_CID_VFLIP:
		
		gc2035_set_flip(client, 1, (int)ctrl->val);
			
		break;
	case V4L2_CID_EXPOSURE:
		
			gc2035_set_exp(client, (int)ctrl->val);
			priv->exp_def = (int)ctrl->val;
		
		break;
	case V4L2_CID_DO_WHITE_BALANCE:
		
			gc2035_set_awb(client, (int)ctrl->val);
			priv->awb_def = (int)ctrl->val;
		
		break;
	
	
	default:
		printk(KERN_ERR "%s: id[%d] not support\n", __func__, ctrl->id);
		break;
	}
	return 0;
}

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int gc2035_g_register(struct v4l2_subdev *sd,  struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	reg->size = 1;
	if (reg->reg > 0xff)
		return -EINVAL;

	ret = i2c_smbus_read_byte_data(client, reg->reg);
	if (ret < 0)
		return ret;

	reg->val = ret;

	return 0;
}

static int gc2035_s_register(struct v4l2_subdev *sd,  const struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->reg > 0xff ||
	    reg->val > 0xff)
		return -EINVAL;

	return i2c_smbus_write_byte_data(client, reg->reg, reg->val);
}
#endif




/*static int gc2035_s_power(struct v4l2_subdev *sd, int on)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct gc2035_priv *priv = container_of(i2c_get_clientdata(client), struct gc2035_priv, subdev);;


	/*
	 * If the power count is modified from 0 to != 0 or from != 0 to 0,
	 * update the power state.
	 */
/*	if (priv->power_count == !on)
	{
		if (priv->pwdn_gpio) gpiod_direction_output(priv->pwdn_gpio, !on);
		if (on && priv->resetb_gpio) {
			/* Active the resetb pin to perform a reset pulse */
/*			gpiod_direction_output(priv->resetb_gpio, 1);
			usleep_range(3000, 5000);
			gpiod_set_value(priv->resetb_gpio, 0);
		}
	}
	priv->power_count += on ? 1 : -1;
	WARN_ON(priv->power_count < 0);

	return 0;
}*/

/*static int gc2035_reset(struct v4l2_subdev *sd, u32 val)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct gc2035_priv *priv = container_of(i2c_get_clientdata(client), struct gc2035_priv, subdev);;
	
  switch(val)
  {
    case 0:
	
	  gpiod_direction_output(priv->resetb_gpio, 1);
      usleep_range(10000,12000);
      break;
    case 1:
      gpiod_direction_output(priv->resetb_gpio, 0);
      usleep_range(10000,12000);
      break;
    default:
      return -EINVAL;
  }
    
  return 0;
}*/

/* Select the nearest higher resolution for capture */
static const struct gc2035_win_size *gc2035_select_win(u32 width, u32 height)
{
	int i, default_size = ARRAY_SIZE(gc2035_supported_win_sizes) - 1;

	for (i = 0; i < ARRAY_SIZE(gc2035_supported_win_sizes); i++) {
		if (gc2035_supported_win_sizes[i].width  >= width &&
		    gc2035_supported_win_sizes[i].height >= height)
			return &gc2035_supported_win_sizes[i];
	}

	return &gc2035_supported_win_sizes[default_size];
}

/*static int gc2035_set_params(struct i2c_client *client,
			     const struct gc2035_win_size *win, u32 code)
{
	const struct regval_list *selected_cfmt_regs;
	u8 val;
	int ret;

	switch (code) {
	case MEDIA_BUS_FMT_RGB565_2X8_BE:
		dev_dbg(&client->dev, "%s: Selected cfmt RGB565 BE", __func__);
		selected_cfmt_regs = gc2035_rgb565_be_regs;
		break;
	case MEDIA_BUS_FMT_RGB565_2X8_LE:
		dev_dbg(&client->dev, "%s: Selected cfmt RGB565 LE", __func__);
		selected_cfmt_regs = gc2035_rgb565_le_regs;
		break;
	case MEDIA_BUS_FMT_YUYV8_2X8:
		dev_dbg(&client->dev, "%s: Selected cfmt YUYV (YUV422)", __func__);
		selected_cfmt_regs = gc2035_yuyv_regs;
		break;
	case MEDIA_BUS_FMT_UYVY8_2X8:
	default:
		dev_dbg(&client->dev, "%s: Selected cfmt UYVY", __func__);
		selected_cfmt_regs = gc2035_uyvy_regs;
		break;
	case MEDIA_BUS_FMT_YVYU8_2X8:
		dev_dbg(&client->dev, "%s: Selected cfmt YVYU", __func__);
		selected_cfmt_regs = gc2035_yuyv_regs;
		break;
	case MEDIA_BUS_FMT_VYUY8_2X8:
		dev_dbg(&client->dev, "%s: Selected cfmt VYUY", __func__);
		selected_cfmt_regs = gc2035_uyvy_regs;
		break;
	}*/

	/* reset hardware */
	//gc2035_reset(client);

	/* initialize the sensor with default data */
	//dev_dbg(&client->dev, "%s: Init default", __func__);
	//ret = gc2035_write_array(client, init_gc2035);
	//if (ret < 0)
		//goto err;

	/* select preamble */
	//dev_dbg(&client->dev, "%s: Set size to %s", __func__, win->name);
	//ret = gc2035_write_array(client, gc2035_size_change_preamble_regs);
	//if (ret < 0)
		//goto err;

	/* set size win */
	//ret = gc2035_write_array(client, win->regs);
	//if (ret < 0)
		//goto err;

	/* cfmt preamble */
	//dev_dbg(&client->dev, "%s: Set cfmt", __func__);
	//ret = gc2035_write_array(client, gc2035_format_change_preamble_regs);
	//if (ret < 0)
		//goto err;

	/* set cfmt */
	//ret = gc2035_write_array(client, selected_cfmt_regs);
	//if (ret < 0)
		//goto err;
	//val = (code == MEDIA_BUS_FMT_YVYU8_2X8)
	/*      || (code == MEDIA_BUS_FMT_VYUY8_2X8) ? CTRL0_VFIRST : 0x00;
	ret = gc2035_mask_set(client, CTRL0, CTRL0_VFIRST, val);
	if (ret < 0)
		goto err;

	return 0;

err:
	dev_err(&client->dev, "%s: Error %d", __func__, ret);
	gc2035_reset(client);

	return ret;
}*/

static int gc2035_get_fmt(struct v4l2_subdev *sd,
		struct v4l2_subdev_state *sd_state,
		struct v4l2_subdev_format *format)
{
	struct v4l2_mbus_framefmt *mf = &format->format;
	struct i2c_client  *client = v4l2_get_subdevdata(sd);
	struct gc2035_priv *priv = container_of(i2c_get_clientdata(client), struct gc2035_priv, subdev);;

	if (format->pad)
		return -EINVAL;

	if (format->which == V4L2_SUBDEV_FORMAT_TRY) {
#ifdef CONFIG_VIDEO_V4L2_SUBDEV_API
		mf = v4l2_subdev_get_try_format(sd, sd_state, 0);
		format->format = *mf;
		return 0;
#else
		return -EINVAL;
#endif
	}

	mf->width	= priv->win->width;
	mf->height	= priv->win->height;
	mf->code	= priv->cfmt_code;
	mf->colorspace	= V4L2_COLORSPACE_SRGB;
	mf->field	= V4L2_FIELD_NONE;
	mf->ycbcr_enc	= V4L2_YCBCR_ENC_DEFAULT;
	mf->quantization = V4L2_QUANTIZATION_DEFAULT;
	mf->xfer_func	= V4L2_XFER_FUNC_DEFAULT;

	return 0;
}

static int gc2035_set_fmt(struct v4l2_subdev *sd,
		struct v4l2_subdev_state *sd_state,
		struct v4l2_subdev_format *format)
{
	struct v4l2_mbus_framefmt *mf = &format->format;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct gc2035_priv *priv = container_of(i2c_get_clientdata(client), struct gc2035_priv, subdev);;
	const struct gc2035_win_size *win;
	int ret = 0;

	if (format->pad)
		return -EINVAL;

	mutex_lock(&priv->lock);

	/* select suitable win */
	win = gc2035_select_win(mf->width, mf->height);
	mf->width	= win->width;
	mf->height	= win->height;

	mf->field	= V4L2_FIELD_NONE;
	mf->colorspace	= V4L2_COLORSPACE_SRGB;
	mf->ycbcr_enc	= V4L2_YCBCR_ENC_DEFAULT;
	mf->quantization = V4L2_QUANTIZATION_DEFAULT;
	mf->xfer_func	= V4L2_XFER_FUNC_DEFAULT;

	switch (mf->code) {
	case MEDIA_BUS_FMT_RGB565_2X8_BE:
	case MEDIA_BUS_FMT_RGB565_2X8_LE:
	case MEDIA_BUS_FMT_YUYV8_2X8:
	case MEDIA_BUS_FMT_UYVY8_2X8:
	case MEDIA_BUS_FMT_YVYU8_2X8:
	case MEDIA_BUS_FMT_VYUY8_2X8:
		break;
	default:
		mf->code = MEDIA_BUS_FMT_UYVY8_2X8;
		break;
	}

	if (format->which == V4L2_SUBDEV_FORMAT_ACTIVE) {
		struct gc2035_priv *priv = container_of(i2c_get_clientdata(client), struct gc2035_priv, subdev);;

		if (priv->streaming) {
			ret = -EBUSY;
			goto out;
		}
		/* select win */
		priv->win = win;
		/* select format */
		priv->cfmt_code = mf->code;
	} else {
		sd_state->pads->try_fmt = *mf;
	}
out:
	mutex_unlock(&priv->lock);

	return ret;
}

static int gc2035_init_cfg(struct v4l2_subdev *sd,
			   struct v4l2_subdev_state *sd_state)
{
#ifdef CONFIG_VIDEO_V4L2_SUBDEV_API
	struct v4l2_mbus_framefmt *try_fmt =
		v4l2_subdev_get_try_format(sd, sd_state, 0);
	const struct gc2035_win_size *win =
		gc2035_select_win(VGA_WIDTH, VGA_HEIGHT);

	try_fmt->width = win->width;
	try_fmt->height = win->height;
	try_fmt->code = MEDIA_BUS_FMT_UYVY8_2X8;
	try_fmt->colorspace = V4L2_COLORSPACE_SRGB;
	try_fmt->field = V4L2_FIELD_NONE;
	try_fmt->ycbcr_enc = V4L2_YCBCR_ENC_DEFAULT;
	try_fmt->quantization = V4L2_QUANTIZATION_DEFAULT;
	try_fmt->xfer_func = V4L2_XFER_FUNC_DEFAULT;
#endif
	return 0;
}

static int gc2035_enum_mbus_code(struct v4l2_subdev *sd, struct v4l2_subdev_state *sd_state, struct v4l2_subdev_mbus_code_enum *code)
{
	if (code->pad || code->index >= ARRAY_SIZE(gc2035_codes))
		return -EINVAL;

	code->code = gc2035_codes[code->index];
	return 0;
}

static int gc2035_get_selection(struct v4l2_subdev *sd,
		struct v4l2_subdev_state *sd_state,
		struct v4l2_subdev_selection *sel)
{
	if (sel->which != V4L2_SUBDEV_FORMAT_ACTIVE)
		return -EINVAL;

	switch (sel->target) {
	case V4L2_SEL_TGT_CROP_BOUNDS:
	case V4L2_SEL_TGT_CROP:
		sel->r.left = 0;
		sel->r.top = 0;
		sel->r.width = UXGA_WIDTH;
		sel->r.height = UXGA_HEIGHT;
		return 0;
	default:
		return -EINVAL;
	}
}


static u32 gc2035_power(void)
{
	int mclk = 26;
	/* set state to power off */

	rcam_pdn(true);
	mdelay(1);
	rcam_rst(true);
	mdelay(1);

	/* power on sequence */
	rcam_clk(true, mclk);
	mdelay(1);
	rcam_pdn(false);
	mdelay(1);
	rcam_rst(false);
	mdelay(10);

	return 0;
}

static struct sensor_reg_list gc2035_init = {
	.size = ARRAY_ROW(init_gc2035),
	.val = init_gc2035
};

static int gc2035_s_power(struct v4l2_subdev *sd, int on)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct gc2035_priv *priv = container_of(i2c_get_clientdata(client), struct gc2035_priv, subdev);
	struct sensor_dev *sdev = NULL;
	int retry = 2;
	int ret = 0;
	

	printk("%s: power %d\n", __func__, on);


	while (on && retry--) 
	{

	
		/* Init sensor to default */
		
		gc2035_power();
		
		
		ret = sensor_write_array(client, &gc2035_init);
		mdelay(10);
		if (ret)
		{
			printk("%s: init sensor failed ret=%d\n", __func__, ret);
			ret = -EIO;
		} 
		else
		{
			printk("%s: init sensor ok\n", __func__);
			break;
		}
		/* power off the sensor */
		rcam_clk(false, 0);
		

		mdelay(10);
		
	}
	

	if (!on) 
	{
		/* power off the sensor */
		rcam_clk(false, 0);

	}


	return ret;
}

static int gc2035_s_stream(struct v4l2_subdev *sd, int on)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct gc2035_priv *priv = container_of(i2c_get_clientdata(client), struct gc2035_priv, subdev);;
	int ret = 0;
	
	printk("gc2035_s_stream");
	
	priv->exp_def = 0;
	priv->awb_def	= 1;
	
	if (priv->streaming != on)
	{
		if (on)
		{
			gc2035_power();
			ret = gc2035_write_array_count(client, init_gc2035, ARRAY_ROW(init_gc2035));
			
			printk("gc2035_s ret = %d", ret);
			
			/* Set exp & awb */
			gc2035_set_exp(client, priv->exp_def);

			gc2035_set_awb(client, priv->awb_def);
			
			if (!ret) ret = __v4l2_ctrl_handler_setup(&priv->hdl);
		} 
	}
	
	if (!ret) priv->streaming = on;

	return ret;
}

/*static int gc2035_s_stream(struct v4l2_subdev *sd, int on)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct gc2035_priv *priv = container_of(i2c_get_clientdata(client), struct gc2035_priv, subdev);;
	int ret = 0;

	mutex_lock(&priv->lock);
	if (priv->streaming == !on) {
		if (on) {
			ret = gc2035_set_params(client, priv->win, priv->cfmt_code);
			if (!ret)
				ret = __v4l2_ctrl_handler_setup(&priv->hdl);
		}
	}
	if (!ret)
		priv->streaming = on;
	mutex_unlock(&priv->lock);

	return ret;
}*/


static const struct v4l2_ctrl_ops gc2035_ctrl_ops = {
	.s_ctrl = gc2035_s_ctrl,
};

static const struct v4l2_subdev_core_ops gc2035_subdev_core_ops = {
	.log_status = v4l2_ctrl_subdev_log_status,
	.subscribe_event = v4l2_ctrl_subdev_subscribe_event,
	.unsubscribe_event = v4l2_event_subdev_unsubscribe,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.g_register	= gc2035_g_register,
	.s_register	= gc2035_s_register,
#endif
	.s_power	= gc2035_s_power,
};

static const struct v4l2_subdev_pad_ops gc2035_subdev_pad_ops = {
	.init_cfg	= gc2035_init_cfg,
	.enum_mbus_code = gc2035_enum_mbus_code,
	.get_selection	= gc2035_get_selection,
	.get_fmt	= gc2035_get_fmt,
	.set_fmt	= gc2035_set_fmt,
};

static const struct v4l2_subdev_video_ops gc2035_subdev_video_ops = {
	.s_stream = gc2035_s_stream,
};

static const struct v4l2_subdev_ops gc2035_subdev_ops = {
	.core	= &gc2035_subdev_core_ops,
	.pad	= &gc2035_subdev_pad_ops,
	.video	= &gc2035_subdev_video_ops,
};

static void ov2640_set_power(struct gc2035_priv *priv, int on)
{
	if (priv->pwdn_gpio) gpiod_direction_output(priv->pwdn_gpio, !on);
	if (on && priv->resetb_gpio) {
		/* Active the resetb pin to perform a reset pulse */
		gpiod_direction_output(priv->resetb_gpio, 1);
		usleep_range(3000, 5000);
		gpiod_set_value(priv->resetb_gpio, 1);
	}
}

/*static int gc2035_probe_dt(struct i2c_client *client, struct gc2035_priv *priv)
{
	int ret;

	/* Request the reset GPIO deasserted */
	/*priv->resetb_gpio = gpiod_get(&client->dev, "resetb", GPIOD_OUT_LOW);

	if (!priv->resetb_gpio) dev_err(&client->dev, "resetb gpio is not assigned!\n");

	ret = PTR_ERR_OR_ZERO(priv->resetb_gpio);
	if (ret && ret != -ENOSYS) {
		dev_err(&client->dev, "Error %d while getting resetb gpio\n", ret);
		return ret;
	}*/

	/* Request the power down GPIO asserted */
	/*priv->pwdn_gpio = gpiod_get(&client->dev, "pwdn", GPIOD_OUT_HIGH);

	if (!priv->pwdn_gpio) dev_err(&client->dev, "pwdn gpio is not assigned!\n");

	ret = PTR_ERR_OR_ZERO(priv->pwdn_gpio);
	if (ret && ret != -ENOSYS) {
		dev_err(&client->dev, "Error %d while getting pwdn gpio\n", ret);
		return ret;
	}

	return 0;
}*/

/*
 * i2c_driver functions
 */
 
 static u32 gc2035_get_chipid(struct i2c_client *client)
{
	u16 chip_id = 0;
	u16 tmp;

	sensor_i2c_read(client, 0xf0, &tmp);
	chip_id = (tmp << 8) & 0xff00;
	sensor_i2c_read(client, 0xf1, &tmp);
	chip_id |= (tmp & 0xff);

	return chip_id;
}
 
 int rda_sensor_adapt_n(struct gc2035_priv* priv, struct i2c_client *client)
{
	u32 cid = 0;

	printk("rda_sensor_adapt\n");
	regulator_enable(priv->cam_reg);

	//gc2035_s_power(&priv->subdev, 1);
	cid = gc2035_get_chipid(client);
	printk("%s:  chip_id=0x%x\n", __func__, cid);
	regulator_disable(priv->cam_reg);	
	if (!cid) {
		printk(KERN_ERR "%s: failed!\n", __func__);
		return -ENODEV;
	}
	return 0;
}

static struct sensor_csi_cfg gc2035_csi_cfg = {
	.csi_en = true,
	.d_term_en = 5,
	.c_term_en = 5,
	.dhs_settle = 5,
	.chs_settle = 5,
};

extern void rcam_config_csi(unsigned int d, unsigned int c, unsigned int line, unsigned int flag);
static int rda_sensor_s_mbus_config(struct gc2035_priv* priv)
{
	struct sensor_csi_cfg *csi = NULL;
	unsigned int d, c, line, flag = 0;


	csi = &gc2035_csi_cfg;
	if (csi->csi_en) {
		d = (csi->d_term_en << 16) | csi->dhs_settle;
		c = (csi->c_term_en << 16) | csi->chs_settle;
		line = 600;

#ifdef _TGT_AP_CAM0_CSI_CH_SEL
		if (!priv->dev_id) {
			flag |= _TGT_AP_CAM0_CSI_CH_SEL;
		}
#endif

#ifdef _TGT_AP_CAM1_CSI_CH_SEL
		if (priv->dev_id) {
			flag |= _TGT_AP_CAM1_CSI_CH_SEL;
		}
#endif

#ifdef _TGT_AP_CAM_CSI_AVDD
		flag |= _TGT_AP_CAM_CSI_AVDD << 1;
#endif

#ifdef _TGT_AP_CAM0_LANE2_ENABLE
		if (!priv->dev_id) {
			flag |= _TGT_AP_CAM0_LANE2_ENABLE << 2;
#if (_TGT_AP_CAM0_LANE2_ENABLE)
			flag |= (0x1 << 3); /* set clk_edge_sel. */
#endif
		}
#endif

#ifdef _TGT_AP_CAM1_LANE2_ENABLE
		if (priv->dev_id) {
			flag |= _TGT_AP_CAM1_LANE2_ENABLE << 2;
#if (_TGT_AP_CAM1_LANE2_ENABLE)
			flag |= (0x1 << 3); /* set clk_edge_sel. */
#endif
		}
#endif



		rcam_config_csi(d, c, line, flag);
	}

	return 0;
}

static int gc2035_probe(struct i2c_client *client)
{
	struct gc2035_priv	*priv;
	struct i2c_adapter	*adapter = client->adapter;
	int			ret;
	
	if(client)	{
		client->addr=0x78>>1;
	}

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(&client->dev, "gc2035: I2C-Adapter doesn't support SMBUS\n");
		return -EIO;
	}

	priv = devm_kzalloc(&client->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	/*priv->clk = devm_clk_get(&client->dev, NULL);
	if (IS_ERR(priv->clk)) 
	{
		dev_err(&adapter->dev, "could not get clock\n");
		return PTR_ERR(priv->clk);
	}
	ret = clk_prepare_enable(priv->clk);
	if (ret)
	{
		dev_err(&adapter->dev, "do not enable the specified clock\n");
		return ret;
	}*/
	
	priv->cam_reg = regulator_get(&adapter->dev, LDO_CAM);
		if (IS_ERR(priv->cam_reg))
		{
			dev_err(&client->dev, "could not find regulator devices\n");
			ret = PTR_ERR(priv->cam_reg);
			goto err_free_reg;
		}
	//ret = gc2035_probe_dt(client, priv);
	
	//if (ret) goto err_clk;
	
	priv->win = gc2035_select_win(VGA_WIDTH, VGA_HEIGHT);
	priv->cfmt_code = MEDIA_BUS_FMT_UYVY8_2X8;

	v4l2_i2c_subdev_init(&priv->subdev, client, &gc2035_subdev_ops);
	priv->subdev.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE |  V4L2_SUBDEV_FL_HAS_EVENTS;
	mutex_init(&priv->lock);
	v4l2_ctrl_handler_init(&priv->hdl, 3);
	priv->hdl.lock = &priv->lock;
	v4l2_ctrl_new_std(&priv->hdl, &gc2035_ctrl_ops, V4L2_CID_VFLIP, 0, 1, 1, 0);
	v4l2_ctrl_new_std(&priv->hdl, &gc2035_ctrl_ops, V4L2_CID_HFLIP, 0, 1, 1, 0);
	v4l2_ctrl_new_std_menu_items(&priv->hdl, &gc2035_ctrl_ops, V4L2_CID_TEST_PATTERN,
			ARRAY_SIZE(gc2035_test_pattern_menu) - 1, 0, 0,
			gc2035_test_pattern_menu);
	priv->subdev.ctrl_handler = &priv->hdl;
	if (priv->hdl.error) {
		ret = priv->hdl.error;
		goto err_hdl;
	}
#if defined(CONFIG_MEDIA_CONTROLLER)
	priv->pad.flags = MEDIA_PAD_FL_SOURCE;
	priv->subdev.entity.function = MEDIA_ENT_F_CAM_SENSOR;
	ret = media_entity_pads_init(&priv->subdev.entity, 1, &priv->pad);
	if (ret < 0)
		goto err_hdl;
#endif

	ret = v4l2_ctrl_handler_setup(&priv->hdl);

	if (ret < 0)
		goto err_videoprobe;

	ret = v4l2_async_register_subdev(&priv->subdev);
	if (ret < 0)
		goto err_videoprobe;

	
	regulator_enable(priv->cam_reg);
	
	
	gc2035_power();
	
	
	rda_sensor_s_mbus_config(priv);
	
	mdelay(10);
	mdelay(10);
	
	dev_info(&client->dev, "i2c_ADDR = %d\n", client->addr);
	
	dev_info(&adapter->dev, "gc2035 rda_sensor_s_mbus_config\n");
	u32 cid = 0;
	cid = gc2035_get_chipid(client);
	printk("%s:  chip_id=0x%x\n", __func__, cid);

	return 0;

err_videoprobe:
	media_entity_cleanup(&priv->subdev.entity);
err_hdl:
	v4l2_ctrl_handler_free(&priv->hdl);
	mutex_destroy(&priv->lock);
err_free_reg:	
	if (!IS_ERR(priv->cam_reg))
		{
			regulator_disable(priv->cam_reg);
			regulator_put(priv->cam_reg);
		}
err_clk:
	//clk_disable_unprepare(priv->clk);*/
	return ret;
}

static void gc2035_remove(struct i2c_client *client)
{
	struct gc2035_priv *priv = container_of(i2c_get_clientdata(client), struct gc2035_priv, subdev);;

	v4l2_async_unregister_subdev(&priv->subdev);
	v4l2_ctrl_handler_free(&priv->hdl);
	mutex_destroy(&priv->lock);
	media_entity_cleanup(&priv->subdev.entity);
	v4l2_device_unregister_subdev(&priv->subdev);
	//clk_disable_unprepare(priv->clk);
	if (!IS_ERR(priv->cam_reg))
	{
		regulator_disable(priv->cam_reg);
		regulator_put(priv->cam_reg);
	}
}

static const struct i2c_device_id gc2035_id[] = {
	{ "gc2035", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, gc2035_id);

static const struct of_device_id gc2035_of_match[] = {
	{.compatible = "op,gc2035", },
	{},
};
MODULE_DEVICE_TABLE(of, gc2035_of_match);

static struct i2c_driver gc2035_i2c_driver = {
	.driver = {
		.name = "gc2035",
		.of_match_table = of_match_ptr(gc2035_of_match),
	},
	.probe    = gc2035_probe,
	.remove   = gc2035_remove,
	.id_table = gc2035_id,
};

module_i2c_driver(gc2035_i2c_driver);

MODULE_DESCRIPTION("Driver for Omni Vision 2640 sensor");
MODULE_AUTHOR("Alberto Panizzo");
MODULE_LICENSE("GPL v2");
