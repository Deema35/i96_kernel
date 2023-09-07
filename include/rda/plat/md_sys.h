/*
 * md_sys.h - A header file of definition of sys channel of modem of RDA
 *
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

#ifndef __MD_SYS_H__
#define __MD_SYS_H__

#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/notifier.h>

#include <rda/plat/devices.h>
#include <rda/plat/rda_md.h>

#include <dt-bindings/rda/md_sys_enums.h>

struct client_mesg {
	unsigned short mod_id;
	unsigned short mesg_id;

	int param_size;
	unsigned char param[PARAM_SIZE_MAX];
	unsigned int reserved0;
	unsigned int reserved1;
};

struct msys_async_item {
	struct list_head async_list;
	struct md_sys_hdr_ext *phdr;
};

struct msys_frame_slot {
	struct list_head list;
	struct md_sys_hdr_ext *phdr;
	unsigned int using;
};

#define MSYS_MAX_SLOTS 	32

struct msys_master {
	struct device dev;

	struct md_port *port;
	/* Hold the version of BP */
	unsigned int version;

	/* for rx */
	struct workqueue_struct *rx_wq;
	struct work_struct rx_work;

	struct list_head rx_pending_list;

	/* for tx */
	spinlock_t pending_lock;
	struct list_head pending_list;

	struct mutex tx_mutex;
	bool running;

	spinlock_t client_lock;
	struct list_head client_list;

	struct client_mesg cli_msg;
	struct client_mesg ts_msg;

	spinlock_t slot_lock;
	char *pslot_data;
	struct msys_frame_slot fr_slot[MSYS_MAX_SLOTS];
};

struct msys_device {
	struct msys_master *pmaster;

	struct list_head dev_entry;

	/* callback for notifier */
	struct notifier_block notifier;
	unsigned int module;

	/* Name of client device */
	const char	 *name;
	/* A pointer points to the private area */
	void *private;
};

struct msys_message {
	struct msys_device *pmsys_dev;

	unsigned short mod_id;
	unsigned short mesg_id;

	/* parameters from ap to bp. */
	const void *pdata;
	int data_size;
	/* parameters from bp to ap. */
	void *pbp_data;
	int bp_data_size;
	/* for pending list of tx */
	struct list_head list;

	/* completion is reported through a callback */
	void (*complete)(void *context);
	void *context;
	/* actual size of tx */
	int tx_len;
	/* received size from bp for debugging */
	int rx_len;
	unsigned int status;
};

struct client_cmd {
	struct msys_device *pmsys_dev;

	unsigned short mod_id;
	unsigned short mesg_id;

	/* parameters from ap to bp. */
	const void *pdata;
	int data_size;
	/* parameters from bp to ap. */
	void *pout_data;
	int out_size;
};


unsigned int rda_msys_send_cmd(struct client_cmd *pcmd);

unsigned int rda_msys_send_cmd_timeout(struct client_cmd *pcmd, int timeout);

struct msys_device *rda_msys_alloc_device(void);

void rda_msys_free_device(struct msys_device *pmsys_dev);

int rda_msys_register_device(struct msys_device *pmsys_dev);

int rda_msys_unregister_device(struct msys_device *pmsys_dev);

#endif /* __MD_SYS_H__ */

