/*
 * V4L2 driver for RDA camera host
 *
 * Copyright (C) 2014 Rda electronics, Inc.
 *
 * Contact: Xing Wei <xingwei@rdamicro.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#include <linux/clk.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/of_graph.h>
#include <linux/of.h>
#include <linux/pm_runtime.h>

#include <linux/videodev2.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fwnode.h>
#include <media/videobuf2-dma-contig.h>
#include <media/v4l2-image-sizes.h>


#include <rda/mach/rda_clk_name.h>
#include <rda/plat/devices.h>
#include <rda/plat/reg_cam_8810.h>
#include <rda/plat/pm_ddr.h>

#include <rda/tgt_ap_board_config.h>
#include <rda/tgt_ap_clock_config.h>

/* Macros */
#define MAX_BUFFER_NUM		32
#define VID_LIMIT_BYTES		(16 * 1024 * 1024)
#define MAX_SUPPORT_WIDTH	2048U
#define MAX_SUPPORT_HEIGHT	2048U

#define RDA_CAM_MBUS_PARA	(V4L2_MBUS_MASTER		|\
				V4L2_MBUS_HSYNC_ACTIVE_HIGH	|\
				V4L2_MBUS_HSYNC_ACTIVE_LOW	|\
				V4L2_MBUS_VSYNC_ACTIVE_HIGH	|\
				V4L2_MBUS_VSYNC_ACTIVE_LOW	|\
				V4L2_MBUS_PCLK_SAMPLE_RISING	|\
				V4L2_MBUS_PCLK_SAMPLE_FALLING	|\
				V4L2_MBUS_DATA_ACTIVE_HIGH)

#define RDA_CAM_MBUS_CSI2	V4L2_MBUS_CSI2_LANES		|\
				V4L2_MBUS_CSI2_CONTINUOUS_CLOCK

#define CAM_OUT_MCLK		(_TGT_AP_PLL_BUS_FREQ >> 3)

/* Global Var */
static void __iomem *cam_regs = NULL;
/* Structure */
//static struct tasklet_struct rcam_tasklet;

/*
 * struct rda_format - Rda media bus format information
 * @fourcc:		Fourcc code for this format
 * @mbus_code:		V4L2 media bus format code.
 * @bpp:		Bytes per pixel (when stored in memory)
 * @support:		Indicates format supported by subdev
 * @skip:		Skip duplicate format supported by subdev
 */
struct rda_format {
	u32	fourcc;
	u32	mbus_code;
	u8	bpp;
};

struct rda_platform_data {
	u8 has_emb_sync;
	u8 hsync_act_low;
	u8 vsync_act_low;
	u8 pclk_act_falling;
	u8 full_mode;
	u32 data_width_flags;

};

struct rda_graph_entity {
	struct device_node *node;

	struct v4l2_subdev *subdev;
};

/* Capture Buffer */
struct cap_buffer {
	struct vb2_buffer vb;
	struct list_head list;
	unsigned int dma_addr;
};

/* RDA camera device */
struct rda_camera_dev {

	
	struct v4l2_device		v4l2_dev;
	struct video_device		*vdev;
	struct device			*dev;
	
	struct rda_graph_entity entity;
	struct v4l2_async_notifier	notifier;

	struct list_head cap_buffer_list;
	struct cap_buffer *active;
	struct cap_buffer *next;

	struct delayed_work isr_work;
	
	const struct rda_format		**user_formats;
	unsigned int			num_user_formats;
	const struct rda_format		*current_fmt;
	
	struct rda_platform_data	pdata;
	
	struct v4l2_format		fmt;

	int state;
	int sequence;
	wait_queue_head_t vsync_wq;
	struct mutex lock;
	spinlock_t			irqlock;
	
	struct vb2_queue		queue;

	struct clk *pclk;
	void __iomem *regs;
	unsigned int irq;
};


/*static const struct soc_mbus_pixelfmt rda_camera_formats[] = {
	{
		.fourcc = V4L2_PIX_FMT_YUYV,
		.name = "Packed YUV422 16 bit",
		.bits_per_sample = 8,
		.packing = SOC_MBUS_PACKING_2X8_PADHI,
		.order = SOC_MBUS_ORDER_LE,
		.layout = SOC_MBUS_LAYOUT_PACKED,
	},
};*/

/* camera states */
enum {
	CAM_STATE_IDLE = 0,
	CAM_STATE_ONESHOT,
	CAM_STATE_SUCCESS,
};

/* -----------------------------------------------------------------
 * Public functions for sensor
 * -----------------------------------------------------------------*/

void rcam_pdn(bool pdn)
{
	HWP_CAMERA_T *hwp_cam = (HWP_CAMERA_T*)cam_regs;
	
	bool acth = true;

	if (acth)
		hwp_cam->CTRL &= ~CAMERA_PWDN_POL_INVERT;
	else
		hwp_cam->CTRL |= CAMERA_PWDN_POL_INVERT;

	if (pdn)
		hwp_cam->CMD_SET = CAMERA_PWDN;
	else
		hwp_cam->CMD_CLR = CAMERA_PWDN;
}

void rcam_rst(bool rst)
{
	HWP_CAMERA_T *hwp_cam = (HWP_CAMERA_T*)cam_regs;
	
	bool acth = false;

	if (acth)
		hwp_cam->CTRL &= ~CAMERA_RESET_POL_INVERT;
	else
		hwp_cam->CTRL |= CAMERA_RESET_POL_INVERT;

	if (rst)
		hwp_cam->CMD_SET = CAMERA_RESET;
	else
		hwp_cam->CMD_CLR = CAMERA_RESET;
}

void rcam_clk(bool out, int freq)
{
	HWP_CAMERA_T *hwp_cam = (HWP_CAMERA_T*)cam_regs;
	unsigned int val = 0x1;
	int tmp;

	if (out) {
		if (freq == 13)
			val |= 0x1 << 4;
		else if (freq == 26)
			val |= 0x2 << 12;
		else {
			tmp = CAM_OUT_MCLK / freq;
			if (tmp < 2)
				tmp = 2;
			else if (tmp > 17)
				tmp = 17;
			val |= (tmp - 2) << 8;
		}
		hwp_cam->CLK_OUT = val;
	} else {
		hwp_cam->CLK_OUT = 0x3f00;
	}
}
#define CAMERA_CSI_CHANNEL_SEL     (1<<20)
#define CAMERA_AVDD1V8_2V8_SEL_REG (1<<11)
void rcam_config_csi(unsigned int d, unsigned int c, unsigned int line, unsigned int flag)
{
	HWP_CAMERA_T *hwp_cam = (HWP_CAMERA_T*)cam_regs;
	unsigned char d_term_en = (d >> 16) & 0xff;
	unsigned char d_hs_setl = d & 0xff;
	unsigned short c_term_en = c >> 16;
	unsigned short c_hs_setl = c & 0xffff;
	unsigned int frame_line = (line >> 1) & 0x3ff;
	unsigned char ch_sel = flag & 0x1;
	unsigned char avdd = (flag >> 1) & 0x1;

	hwp_cam->CAM_CSI_REG_0 = 0xA0000000 | (frame_line << 8) | d_term_en;
	hwp_cam->CAM_CSI_REG_1 = 0x00020000 | d_hs_setl;
	hwp_cam->CAM_CSI_REG_2 = (c_term_en << 16) | c_hs_setl;
	hwp_cam->CAM_CSI_REG_3 = 0x9E0A0800 | (ch_sel << 20) | (avdd << 11);
	hwp_cam->CAM_CSI_REG_4 = 0xffffffff;
	hwp_cam->CAM_CSI_REG_5 = 0x40dc0200;
	hwp_cam->CAM_CSI_REG_6 = 0x800420ea;
	hwp_cam->CAM_CSI_ENABLE = 1;
}

/* -----------------------------------------------------------------
 * Private functions
 * -----------------------------------------------------------------*/
static void start_dma(struct rda_camera_dev *rcam)
{
	HWP_CAMERA_T *hwp_cam = (HWP_CAMERA_T*)rcam->regs;
	unsigned int dma_addr = rcam->active->dma_addr;
	unsigned int size = rcam->fmt.fmt.pix.sizeimage;

	pm_ddr_get(PM_DDR_CAMERA_DMA);
	/* config address & size for next frame */
	hwp_cam->CAM_FRAME_START_ADDR = dma_addr;
	hwp_cam->CAM_FRAME_SIZE = size;

	/* config Camera controller & AXI */
	hwp_cam->CMD_SET = CAMERA_FIFO_RESET;
	hwp_cam->CAM_AXI_CONFIG = CAMERA_AXI_BURST(0xf);//0~15
	/* enable Camera controller & AXI */
	hwp_cam->CTRL |= CAMERA_ENABLE;
	hwp_cam->CAM_AXI_CONFIG |= CAMERA_AXI_START;
}

static void stop_dma(struct rda_camera_dev *rcam)
{
	HWP_CAMERA_T *hwp_cam = (HWP_CAMERA_T*)rcam->regs;

	/* disable Camera controller & AXI */
	hwp_cam->CTRL &= ~CAMERA_ENABLE;
	hwp_cam->CAM_AXI_CONFIG &= ~CAMERA_AXI_START;
	pm_ddr_put(PM_DDR_CAMERA_DMA);
}

static int configure_geometry(struct rda_camera_dev *rcam, u32 code)
{
	HWP_CAMERA_T *hwp_cam = (HWP_CAMERA_T*)rcam->regs;
	switch (code) {
	case MEDIA_BUS_FMT_YUYV8_2X8:
		hwp_cam->CTRL |= CAMERA_REORDER_YUYV;
		break;
	case MEDIA_BUS_FMT_YVYU8_2X8:
		hwp_cam->CTRL |= CAMERA_REORDER_YVYU;
		break;
	case MEDIA_BUS_FMT_UYVY8_2X8:
		hwp_cam->CTRL |= CAMERA_REORDER_UYVY;
		break;
	case MEDIA_BUS_FMT_VYUY8_2X8:
		hwp_cam->CTRL |= CAMERA_REORDER_VYUY;
		break;
	default:
		dev_err(rcam->dev,  "%s: pixelcode: %x not support\n", __func__, code);
		return -EINVAL;
	}
	hwp_cam->CTRL |= CAMERA_DATAFORMAT_YUV422;

	return 0;
}

static void handle_vsync(struct work_struct *wk)
{
	struct delayed_work *dwk = to_delayed_work(wk);
	struct rda_camera_dev *p = container_of(dwk, struct rda_camera_dev, isr_work);
	HWP_CAMERA_T *hwp_cam = (HWP_CAMERA_T*)p->regs;
	unsigned int tc = hwp_cam->CAM_TC_COUNT;

	if (!tc)
		schedule_delayed_work(dwk, msecs_to_jiffies(5));
	else if (p->next) {
		hwp_cam->CAM_FRAME_START_ADDR = p->next->dma_addr;
		dev_dbg(p->dev,  "%s: tc: %d, next dma_addr: 0x%x\n", __func__, tc, p->next->dma_addr);
	} else
		dev_dbg(p->dev,  "%s: p->next is NULL\n", __func__);
}
/*
static void handle_vsync(unsigned long pcam)
{
	struct rda_camera_dev *p = (struct rda_camera_dev*)pcam;
	HWP_CAMERA_T *hwp_cam = (HWP_CAMERA_T*)p->regs;
	unsigned int tc = hwp_cam->CAM_TC_COUNT;

	if (!tc) {
		tasklet_schedule(&rcam_tasklet);
	} else if (p->next) {
		hwp_cam->CAM_FRAME_START_ADDR = p->next->dma_addr;
		rda_dbg_camera("%s: tc: %d, next dma_addr: 0x%x\n",
				__func__, tc, p->next->dma_addr);
	} else
		rda_dbg_camera("%s: p->next is NULL\n", __func__);
}
*/

static irqreturn_t handle_streaming(struct rda_camera_dev *rcam)
{
	struct cap_buffer *buf = rcam->active;
	struct vb2_buffer *vb= &buf->vb;
	struct vb2_v4l2_buffer *vbuf = to_vb2_v4l2_buffer(vb);
	HWP_CAMERA_T *hwp_cam = (HWP_CAMERA_T*)rcam->regs;
	unsigned int tc = hwp_cam->CAM_TC_COUNT;

	if (!buf)
		return IRQ_HANDLED;

	if (tc)
		dev_err(rcam->dev,  "%s: frame size=%d, tc=%d\n", __func__, rcam->fmt.fmt.pix.sizeimage, tc);

	list_del_init(&buf->list);
	if (list_empty(&rcam->cap_buffer_list)) {
		rcam->active = NULL;
		rcam->next = NULL;
	} else {
		rcam->active = list_entry(rcam->cap_buffer_list.next,
				struct cap_buffer, list);
		if (list_is_last(&rcam->active->list, &rcam->cap_buffer_list))
			rcam->next = NULL;
		else
			rcam->next = list_entry(rcam->active->list.next,
					struct cap_buffer, list);
	}

	//vb = &buf->vb;
	vb->timestamp = ktime_get_ns();
	vbuf->sequence = rcam->sequence++;
	vb2_buffer_done(vb, VB2_BUF_STATE_DONE);

	return IRQ_HANDLED;
}

static irqreturn_t rda_camera_isr(int irq, void *dev)
{
	struct rda_camera_dev *rcam = dev;
	HWP_CAMERA_T *hwp_cam = (HWP_CAMERA_T*)rcam->regs;
	unsigned int irq_cause = 0;
	unsigned int state = 0;
	unsigned int addr = 0;
	irqreturn_t ret = IRQ_NONE;
	unsigned long flags = 0;

	spin_lock_irqsave(&rcam->irqlock, flags);
	irq_cause = hwp_cam->IRQ_CAUSE;
	hwp_cam->IRQ_CLEAR |= irq_cause;
	addr = hwp_cam->CAM_FRAME_START_ADDR;
	state = rcam->state;
	if (irq_cause & IRQ_VSYNC_R) {
		if (rcam->next) {
			schedule_delayed_work(&rcam->isr_work, 0);
//			tasklet_schedule(&rcam_tasklet);
			rcam->state = CAM_STATE_SUCCESS;
		} else
			rcam->state = CAM_STATE_ONESHOT;
		if (state == CAM_STATE_IDLE) {
			wake_up_interruptible(&rcam->vsync_wq);
		}
		ret = IRQ_HANDLED;
	} else if (irq_cause & IRQ_OVFL) {
		rcam->state = CAM_STATE_ONESHOT;
		dev_err(rcam->dev,  "%s: overflow!\n", __func__);
		ret = IRQ_HANDLED;
	} else if (irq_cause & IRQ_VSYNC_F) {
		if (!rcam->active) {
			ret = IRQ_HANDLED;
		} else if ((state == CAM_STATE_ONESHOT) ||
				(addr == rcam->active->dma_addr)) {
			stop_dma(rcam);
			ret = handle_streaming(rcam);
			if (rcam->active)
				start_dma(rcam);
			rcam->state = CAM_STATE_ONESHOT;
			cancel_delayed_work(&rcam->isr_work);
		} else if (state == CAM_STATE_SUCCESS) {
			ret = handle_streaming(rcam);
		}
	}

	spin_unlock_irqrestore(&rcam->irqlock, flags);

	dev_dbg(rcam->dev, "%s: cause: %x, addr: 0x%x, state: %d, new state %d\n", __func__, irq_cause, addr, state, rcam->state);
	return ret;
}

/* -----------------------------------------------------------------
 * Videobuf operations
 * -----------------------------------------------------------------*/
static int queue_setup(struct vb2_queue *vq, unsigned int *nbuffers, unsigned int* nplanes, unsigned int sizes[], struct device *alloc_devs[])
{
	struct rda_camera_dev *rcam = vb2_get_drv_priv(vq);
	unsigned int size;
	dev_info(rcam->dev, "queue_setup\n");
	/* May need reset camera host */
	/* TODO: do hardware reset here */
	size = rcam->fmt.fmt.pix.sizeimage;

	if (!*nbuffers || *nbuffers > MAX_BUFFER_NUM)
		*nbuffers = MAX_BUFFER_NUM;

	if (size * *nbuffers > VID_LIMIT_BYTES)
		*nbuffers = VID_LIMIT_BYTES / size;

	*nplanes = 1;
	sizes[0] = size;

	rcam->sequence = 0;
	rcam->active = NULL;
	rcam->next = NULL;
	dev_info(rcam->dev, "%s: count=%d, size=%d\n", __func__, *nbuffers, size);

	return 0;
}

static int buffer_init(struct vb2_buffer *vb)
{
	struct cap_buffer *buf = container_of(vb, struct cap_buffer, vb);
	struct rda_camera_dev *rcam = vb2_get_drv_priv(vb->vb2_queue);
	dev_info(rcam->dev, "buffer_init\n");

	INIT_LIST_HEAD(&buf->list);

	return 0;
}

static int buffer_prepare(struct vb2_buffer *vb)
{
	struct cap_buffer *buf = container_of(vb, struct cap_buffer, vb);
	struct rda_camera_dev *rcam = vb2_get_drv_priv(vb->vb2_queue);
	dev_info(rcam->dev, "buffer_prepare\n");
	
	unsigned long size;

	size = rcam->fmt.fmt.pix.sizeimage;

	if (vb2_plane_size(vb, 0) < size) {
		dev_dbg(rcam->dev, "%s: data will not fit into plane(%lu < %lu)\n",
				__func__, vb2_plane_size(vb, 0), size);
		return -EINVAL;
	}

	vb2_set_plane_payload(&buf->vb, 0, size);

	return 0;
}

static void buffer_cleanup(struct vb2_buffer *vb)
{
	struct rda_camera_dev *rcam = vb2_get_drv_priv(vb->vb2_queue);
	dev_info(rcam->dev, "buffer_cleanup\n");
}

static void buffer_queue(struct vb2_buffer *vb)
{
	struct rda_camera_dev *rcam = vb2_get_drv_priv(vb->vb2_queue);
	struct cap_buffer *buf = container_of(vb, struct cap_buffer, vb);
	unsigned long flags = 0;
	
	dev_info(rcam->dev, "buffer_queue\n");

	buf->dma_addr = vb2_dma_contig_plane_dma_addr(vb, 0);
	spin_lock_irqsave(&rcam->irqlock, flags);
	list_add_tail(&buf->list, &rcam->cap_buffer_list);

	if (rcam->active == NULL)
	{
		rcam->active = buf;
		dev_dbg(rcam->dev, "%s: rcam->active->dma_addr: 0x%x\n",
				__func__, buf->dma_addr);
		if (vb2_is_streaming(vb->vb2_queue)) {
			start_dma(rcam);
		}
	} else if (rcam->next == NULL)
	{
		rcam->next = buf;
		dev_dbg(rcam->dev, "%s: rcam->next->dma_addr: 0x%x\n",
				__func__, buf->dma_addr);
	}
	spin_unlock_irqrestore(&rcam->irqlock, flags);
}

static int start_streaming(struct vb2_queue *vq, unsigned int count)
{
	struct rda_camera_dev *rcam = vb2_get_drv_priv(vq);

	
	dev_info(rcam->dev, "start_streaming\n");
	
	dev_info(rcam->dev, "num_user_formats = %d\n",rcam->num_user_formats);
	
	if (list_empty(&vq->done_list))
	{
		dev_info(rcam->dev, "done_list is empty\n");
	}
	
	HWP_CAMERA_T *hwp_cam = (HWP_CAMERA_T*)rcam->regs;
	struct cap_buffer *buf, *node;
	unsigned long flags = 0;
	int ret;

	spin_lock_irqsave(&rcam->irqlock, flags);
	rcam->state = CAM_STATE_IDLE;
	hwp_cam->IRQ_CLEAR |= IRQ_MASKALL;
	hwp_cam->IRQ_MASK |= IRQ_VSYNC_R | IRQ_VSYNC_F | IRQ_OVFL;
	/* enable camera before wait vsync */
	if (count)
		start_dma(rcam);
	spin_unlock_irqrestore(&rcam->irqlock, flags);
	
	/* Enable stream on the sub device */
	ret = v4l2_subdev_call(rcam->entity.subdev, video, s_stream, 1);
	if (ret && ret != -ENOIOCTLCMD) {
		dev_err(rcam->dev, "stream on failed in subdev ret =%d\n", ret);
		goto err;
	}

	dev_info(rcam->dev, "%s: Waiting for VSYNC\n", __func__);
	ret = wait_event_interruptible_timeout(rcam->vsync_wq,
			rcam->state != CAM_STATE_IDLE,
			msecs_to_jiffies(500));
	if (ret == 0) {
		dev_info(rcam->dev, "%s: timed out\n", __func__);
		ret = -ETIMEDOUT;
		goto err;
	} else if (ret == -ERESTARTSYS) {
		dev_info(rcam->dev, "%s: Interrupted by a signal\n", __func__);
		goto err;
	}

	return 0;
err:
	/* Clear & Disable interrupt */
	hwp_cam->IRQ_CLEAR |= IRQ_MASKALL;
	hwp_cam->IRQ_MASK &= ~IRQ_MASKALL;
	stop_dma(rcam);
	rcam->active = NULL;
	rcam->next = NULL;
	list_for_each_entry_safe(buf, node, &rcam->cap_buffer_list, list) {
		list_del_init(&buf->list);
		vb2_buffer_done(&buf->vb, VB2_BUF_STATE_ERROR);
	}
	INIT_LIST_HEAD(&rcam->cap_buffer_list);
	return ret;
}

static void stop_streaming(struct vb2_queue *vq)
{
	struct rda_camera_dev *rcam = vb2_get_drv_priv(vq);
	
	dev_info(rcam->dev, "stop_streaming\n");
	
	HWP_CAMERA_T *hwp_cam = (HWP_CAMERA_T*)rcam->regs;
	struct cap_buffer *buf, *node;
	unsigned long flags = 0;

	cancel_delayed_work(&rcam->isr_work);
	flush_scheduled_work();
	spin_lock_irqsave(&rcam->irqlock, flags);
	/* Clear & Disable interrupt */
	hwp_cam->IRQ_CLEAR |= IRQ_MASKALL;
	hwp_cam->IRQ_MASK &= ~IRQ_MASKALL;
	stop_dma(rcam);
	/* Release all active buffers */
	rcam->active = NULL;
	rcam->next = NULL;
	list_for_each_entry_safe(buf, node, &rcam->cap_buffer_list, list) {
		list_del_init(&buf->list);
		vb2_buffer_done(&buf->vb, VB2_BUF_STATE_ERROR);
	}
	INIT_LIST_HEAD(&rcam->cap_buffer_list);
	spin_unlock_irqrestore(&rcam->irqlock, flags);

}

static const struct vb2_ops rda_video_qops = {
	.queue_setup = queue_setup,
	.buf_init = buffer_init,
	.buf_prepare = buffer_prepare,
	.buf_cleanup = buffer_cleanup,
	.buf_queue = buffer_queue,
	.start_streaming = start_streaming,
	.stop_streaming = stop_streaming,
	.wait_prepare = vb2_ops_wait_prepare,
	.wait_finish = vb2_ops_wait_finish,
};

/* -----------------------------------------------------------------
 * SoC camera operation for the device
 * -----------------------------------------------------------------*/

static const struct rda_format *find_format_by_fourcc(struct rda_camera_dev *rcam, unsigned int fourcc)
{
	unsigned int num_formats = rcam->num_user_formats;
	const struct rda_format *fmt;
	unsigned int i;

	for (i = 0; i < num_formats; i++) {
		fmt = rcam->user_formats[i];
		if (fmt->fourcc == fourcc)
			return fmt;
	}

	return NULL;
}

static void rda_try_fse(struct rda_camera_dev *rcam, const struct rda_format *rda_fmt, struct v4l2_subdev_state *sd_state)
{
	int ret;
	struct v4l2_subdev_frame_size_enum fse = {
		.code = rda_fmt->mbus_code,
		.which = V4L2_SUBDEV_FORMAT_TRY,
	};

	ret = v4l2_subdev_call(rcam->entity.subdev, pad, enum_frame_size,  sd_state, &fse);
	/*
	 * Attempt to obtain format size from subdev. If not available,
	 * just use the maximum RDA can receive.
	 */
	if (ret) {
		sd_state->pads->try_crop.width = MAX_SUPPORT_WIDTH;
		sd_state->pads->try_crop.height = MAX_SUPPORT_HEIGHT;
	} else {
		sd_state->pads->try_crop.width = fse.max_width;
		sd_state->pads->try_crop.height = fse.max_height;
	}
}

static int rda_camera_try_fmt(struct rda_camera_dev *rcam, struct v4l2_format *f, const struct rda_format **current_fmt)
{
	const struct rda_format *rda_fmt;
	struct v4l2_pix_format *pixfmt = &f->fmt.pix;
	struct v4l2_subdev_pad_config pad_cfg = {};
	struct v4l2_subdev_state pad_state = {
		.pads = &pad_cfg,
	};
	struct v4l2_subdev_format format = {
		.which = V4L2_SUBDEV_FORMAT_TRY,
	};
	int ret;
	
	dev_info(rcam->dev, "rda_camera_set_fmt");

	rda_fmt = find_format_by_fourcc(rcam, pixfmt->pixelformat);
	if (!rda_fmt) {
		rda_fmt = rcam->user_formats[rcam->num_user_formats - 1];
		pixfmt->pixelformat = rda_fmt->fourcc;
	}

	/* Limit to Atmel RDA hardware capabilities */
	pixfmt->width = clamp(pixfmt->width, 0U, MAX_SUPPORT_WIDTH);
	pixfmt->height = clamp(pixfmt->height, 0U, MAX_SUPPORT_HEIGHT);

	v4l2_fill_mbus_format(&format.format, pixfmt, rda_fmt->mbus_code);

	rda_try_fse(rcam, rda_fmt, &pad_state);

	ret = v4l2_subdev_call(rcam->entity.subdev, pad, set_fmt, &pad_state, &format);
	if (ret < 0)
		return ret;

	v4l2_fill_pix_format(pixfmt, &format.format);

	pixfmt->field = V4L2_FIELD_NONE;
	pixfmt->bytesperline = pixfmt->width * rda_fmt->bpp;
	pixfmt->sizeimage = pixfmt->bytesperline * pixfmt->height;

	if (current_fmt)
		*current_fmt = rda_fmt;

	return 0;
}


static int rda_camera_set_fmt(struct rda_camera_dev *rcam, struct v4l2_format *f)
{
	struct v4l2_subdev_format format = {
		.which = V4L2_SUBDEV_FORMAT_ACTIVE,
	};
	const struct rda_format *current_fmt;
	int ret;
	dev_info(rcam->dev, "rda_camera_set_fmt");
	ret = rda_camera_try_fmt(rcam, f, &current_fmt);
	if (ret)
		return ret;

	v4l2_fill_mbus_format(&format.format, &f->fmt.pix,  current_fmt->mbus_code);
	ret = v4l2_subdev_call(rcam->entity.subdev, pad, set_fmt, NULL, &format);
	if (ret < 0)
		return ret;
	
	ret = configure_geometry(rcam, current_fmt->mbus_code);
	if (ret < 0)
		return ret;

	rcam->fmt = *f;
	rcam->current_fmt = current_fmt;
	

	return 0;
}


/*static int rda_camera_try_bus_param(struct soc_camera_device *icd,
		unsigned char buswidth)
{
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct v4l2_mbus_config cfg = {.type = V4L2_MBUS_PARALLEL,};
	unsigned int common_flags = RDA_CAM_MBUS_PARA;
	int ret;

	ret = v4l2_subdev_call(sd, video, g_mbus_config, &cfg);
	if (!ret) {
		if (cfg.type == V4L2_MBUS_CSI2)
			common_flags = RDA_CAM_MBUS_CSI2;
		common_flags = soc_mbus_config_compatible(&cfg,
				common_flags);
		if (!common_flags) {
			rda_dbg_camera("%s: Flags incompatible camera 0x%x, host 0x%x\n",
					__func__, cfg.flags, common_flags);
			return -EINVAL;
		}
	} else if (ret != -ENOIOCTLCMD) {
		return ret;
	}

	return 0;
}*/

/* This will be corrected as we get more formats */
/*static bool rda_camera_packing_supported(const struct soc_mbus_pixelfmt *fmt)
{
	return fmt->packing == SOC_MBUS_PACKING_NONE ||
		(fmt->bits_per_sample == 8 &&
		 fmt->packing == SOC_MBUS_PACKING_2X8_PADHI) ||
		(fmt->bits_per_sample > 8 &&
		 fmt->packing == SOC_MBUS_PACKING_EXTEND16);
}*/

/*static int rda_camera_get_formats(struct soc_camera_device *icd,
		unsigned int idx,
		struct soc_camera_format_xlate *xlate)
{
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	int formats = 0, ret;*/
	/* sensor format */
//	enum v4l2_mbus_pixelcode code;
	/* soc camera host format */
/*	const struct soc_mbus_pixelfmt *fmt;

	ret = v4l2_subdev_call(sd, video, enum_mbus_fmt, idx, &code);
	if (ret < 0)*/
		/* No more formats */
/*		return 0;

	fmt = soc_mbus_get_fmtdesc(code);
	if (!fmt) {
		rda_dbg_camera("%s: Invalid format code #%u: %d\n",
				__func__, idx, code);
		return 0;
	}*/

	/* This also checks support for the requested bits-per-sample */
//	ret = rda_camera_try_bus_param(icd, fmt->bits_per_sample);
/*	if (ret < 0) {
		rda_dbg_camera("%s: Fail to try the bus parameters.\n",
				__func__);
		return 0;
	}

	switch (code) {
	case V4L2_MBUS_FMT_UYVY8_2X8:
	case V4L2_MBUS_FMT_VYUY8_2X8:
	case V4L2_MBUS_FMT_YUYV8_2X8:
	case V4L2_MBUS_FMT_YVYU8_2X8:
		formats++;
		if (xlate) {
			xlate->host_fmt = &rda_camera_formats[0];
			xlate->code = code;
			xlate++;
			rda_dbg_camera("%s: Providing format %s using code %d\n",
					__func__, rda_camera_formats[0].name, code);
		}
		break;
	default:
		if (!rda_camera_packing_supported(fmt))
			return 0;
		if (xlate)
			rda_dbg_camera("%s: Providing format %s in pass-through mode\n",
					__func__, fmt->name);
	}*/

	/* Generic pass-through */
/*	formats++;
	if (xlate) {
		xlate->host_fmt = fmt;
		xlate->code = code;
		xlate++;
	}

	return formats;
}*/

/* Called with .host_lock held */
/*static int rda_camera_add_device(struct soc_camera_device *icd)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct rda_camera_dev *rcam = ici->priv;
	int ret;

	if (rcam->icd)
		return -EBUSY;

	ret = clk_enable(rcam->pclk);
	if (ret)
		return ret;

	rcam->icd = icd;
	rda_dbg_camera("%s: Camera driver attached to camera %d\n",
			__func__, icd->devnum);
	return 0;
}*/

/* Called with .host_lock held */
/*static void rda_camera_remove_device(struct soc_camera_device *icd)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct rda_camera_dev *rcam = ici->priv;

	BUG_ON(icd != rcam->icd);

	clk_disable(rcam->pclk);
	rcam->icd = NULL;

	rda_dbg_camera("%s: Camera driver detached from camera %d\n",
			__func__, icd->devnum);
}*/

/*static unsigned int rda_camera_poll(struct file *file, poll_table *pt)
{
	struct soc_camera_device *icd = file->private_data;

	return vb2_poll(&icd->vb2_vidq, file, pt);
}
*/
static int rda_querycap(struct file *file, void *priv, struct v4l2_capability *cap)
{
	strscpy(cap->driver, RDA_CAMERA_DRV_NAME, sizeof(cap->driver));
	strscpy(cap->card, "RDA Camera Sensor Interface", sizeof(cap->card));
	strscpy(cap->bus_info, "platform:rda", sizeof(cap->bus_info));
	cap->capabilities = (V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING);
	return 0;
}





/*static int rda_querycap(struct file *file, void *priv, struct v4l2_capability *cap)
{
	strscpy(cap->driver, "RDA", sizeof(cap->driver));
	strscpy(cap->card, "RDA Image Sensor Interface", sizeof(cap->card));
	strscpy(cap->bus_info, "platform:rda", sizeof(cap->bus_info));
	return 0;
}*/

static int rda_try_fmt_vid_cap(struct file *file, void *priv, struct v4l2_format *f)
{
	struct rda_camera_dev *rcam = video_drvdata(file);

	return rda_camera_try_fmt(rcam, f, NULL);
}

static int rda_g_fmt_vid_cap(struct file *file, void *priv, struct v4l2_format *fmt)
{
	struct rda_camera_dev *rcam = video_drvdata(file);

	*fmt = rcam->fmt;

	return 0;
}

static int rda_s_fmt_vid_cap(struct file *file, void *priv,   struct v4l2_format *f)
{
	struct rda_camera_dev *rcam = video_drvdata(file);

	if (vb2_is_streaming(&rcam->queue))
		return -EBUSY;

	return rda_camera_set_fmt(rcam, f);
}

static int rda_enum_fmt_vid_cap(struct file *file, void  *priv, struct v4l2_fmtdesc *f)
{
	struct rda_camera_dev *rcam = video_drvdata(file);

	if (f->index >= rcam->num_user_formats)
		return -EINVAL;

	f->pixelformat = rcam->user_formats[f->index]->fourcc;
	return 0;
}

static int rda_enum_input(struct file *file, void *priv, struct v4l2_input *i)
{
	if (i->index != 0)
		return -EINVAL;

	i->type = V4L2_INPUT_TYPE_CAMERA;
	strscpy(i->name, "Camera", sizeof(i->name));
	return 0;
}

static int rda_g_input(struct file *file, void *priv, unsigned int *i)
{
	*i = 0;
	return 0;
}

static int rda_s_input(struct file *file, void *priv, unsigned int i)
{
	if (i > 0) return -EINVAL;
	return 0;
}

static int rda_g_parm(struct file *file, void *fh, struct v4l2_streamparm *a)
{
	struct rda_camera_dev *rcam = video_drvdata(file);
	printk("rda_g_parm\n");
	return v4l2_g_parm_cap(video_devdata(file), rcam->entity.subdev, a);
}

static int rda_s_parm(struct file *file, void *fh, struct v4l2_streamparm *a)
{
	struct rda_camera_dev *rcam = video_drvdata(file);
	printk("rda_s_parm\n");
	
	

	return v4l2_s_parm_cap(video_devdata(file), rcam->entity.subdev, a);
}

static int rda_enum_framesizes(struct file *file, void *fh,  struct v4l2_frmsizeenum *fsize)
{
	struct rda_camera_dev *rcam = video_drvdata(file);
	const struct rda_format *rda_fmt;
	struct v4l2_subdev_frame_size_enum fse = {
		.index = fsize->index,
		.which = V4L2_SUBDEV_FORMAT_ACTIVE,
	};
	int ret;

	rda_fmt = find_format_by_fourcc(rcam, fsize->pixel_format);
	if (!rda_fmt) return -EINVAL;

	fse.code = rda_fmt->mbus_code;

	ret = v4l2_subdev_call(rcam->entity.subdev, pad, enum_frame_size,  NULL, &fse);
	
	if (ret) return ret;

	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->discrete.width = fse.max_width;
	fsize->discrete.height = fse.max_height;

	return 0;
}

static int rda_enum_frameintervals(struct file *file, void *fh, struct v4l2_frmivalenum *fival)
{
	struct rda_camera_dev *rcam = video_drvdata(file);
	const struct rda_format *rda_fmt;
	struct v4l2_subdev_frame_interval_enum fie = {
		.index = fival->index,
		.width = fival->width,
		.height = fival->height,
		.which = V4L2_SUBDEV_FORMAT_ACTIVE,
	};
	int ret;

	rda_fmt = find_format_by_fourcc(rcam, fival->pixel_format);
	if (!rda_fmt)
		return -EINVAL;

	fie.code = rda_fmt->mbus_code;

	ret = v4l2_subdev_call(rcam->entity.subdev, pad, enum_frame_interval, NULL, &fie);
	if (ret)
		return ret;

	fival->type = V4L2_FRMIVAL_TYPE_DISCRETE;
	fival->discrete = fie.interval;

	return 0;
}

static const struct v4l2_ioctl_ops rda_ioctl_ops = 
{
	.vidioc_querycap		= rda_querycap,

	.vidioc_try_fmt_vid_cap		= rda_try_fmt_vid_cap,
	.vidioc_g_fmt_vid_cap		= rda_g_fmt_vid_cap,
	.vidioc_s_fmt_vid_cap		= rda_s_fmt_vid_cap,
	.vidioc_enum_fmt_vid_cap	= rda_enum_fmt_vid_cap,

	.vidioc_enum_input		= rda_enum_input,
	.vidioc_g_input			= rda_g_input,
	.vidioc_s_input			= rda_s_input,

	.vidioc_g_parm			= rda_g_parm,
	.vidioc_s_parm			= rda_s_parm,
	.vidioc_enum_framesizes		= rda_enum_framesizes,
	.vidioc_enum_frameintervals	= rda_enum_frameintervals,

	.vidioc_reqbufs			= vb2_ioctl_reqbufs,
	.vidioc_create_bufs		= vb2_ioctl_create_bufs,
	.vidioc_querybuf		= vb2_ioctl_querybuf,
	.vidioc_qbuf			= vb2_ioctl_qbuf,
	.vidioc_dqbuf			= vb2_ioctl_dqbuf,
	.vidioc_expbuf			= vb2_ioctl_expbuf,
	.vidioc_prepare_buf		= vb2_ioctl_prepare_buf,
	.vidioc_streamon		= vb2_ioctl_streamon,
	.vidioc_streamoff		= vb2_ioctl_streamoff,

	.vidioc_log_status		= v4l2_ctrl_log_status,
	.vidioc_subscribe_event		= v4l2_ctrl_subscribe_event,
	.vidioc_unsubscribe_event	= v4l2_event_unsubscribe,
};

static int rda_file_open(struct file *file)
{
	struct rda_camera_dev *rcam = video_drvdata(file);
	struct v4l2_subdev *sd = rcam->entity.subdev;
	int ret;

	ret = v4l2_fh_open(file);
	if (ret < 0)
		return ret;

	if (!v4l2_fh_is_singular_file(file))
		goto fh_rel;

	ret = v4l2_subdev_call(sd, core, s_power, 1);
	if (ret < 0 && ret != -ENOIOCTLCMD)
		goto fh_rel;

	ret = rda_camera_set_fmt(rcam, &rcam->fmt);
	if (ret)
		v4l2_subdev_call(sd, core, s_power, 0);
fh_rel:
	if (ret)
		v4l2_fh_release(file);

	return ret;
}

static int rda_file_release(struct file *file)
{
	struct rda_camera_dev *rcam = video_drvdata(file);
	struct v4l2_subdev *sd = rcam->entity.subdev;
	bool fh_singular;
	int ret;


	fh_singular = v4l2_fh_is_singular_file(file);

	ret = _vb2_fop_release(file, NULL);

	if (fh_singular) v4l2_subdev_call(sd, core, s_power, 0);


	return ret;
}

static const struct v4l2_file_operations rda_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= video_ioctl2,
	.open		= rda_file_open,
	.release	= rda_file_release,
	.poll		= vb2_fop_poll,
	.mmap		= vb2_fop_mmap,
	.read		= vb2_fop_read,
};



static int rda_graph_notify_bound(struct v4l2_async_notifier *notifier, struct v4l2_subdev *subdev, struct v4l2_async_subdev *asd)
{
	struct rda_camera_dev *rcam = container_of(notifier, struct rda_camera_dev, notifier);

	dev_dbg(rcam->dev, "subdev %s bound\n", subdev->name);

	rcam->entity.subdev = subdev;

	return 0;
}

static void rda_graph_notify_unbind(struct v4l2_async_notifier *notifier,  struct v4l2_subdev *sd, struct v4l2_async_subdev *asd)
{
	
	struct rda_camera_dev *rcam = container_of(notifier, struct rda_camera_dev, notifier);

	dev_dbg(rcam->dev, "Removing %s\n", video_device_node_name(rcam->vdev));

	/* Checks internally if vdev have been init or not */
	video_unregister_device(rcam->vdev);
}

static int rda_set_default_fmt(struct rda_camera_dev *rcam)
{
	struct v4l2_format f = {
		.type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.fmt.pix = {
			.width		= VGA_WIDTH,
			.height		= VGA_HEIGHT,
			.field		= V4L2_FIELD_NONE,
			.pixelformat	= rcam->user_formats[0]->fourcc,
		},
	};
	int ret;

	ret = rda_camera_try_fmt(rcam, &f, NULL);
	if (ret)
		return ret;
	rcam->current_fmt = rcam->user_formats[0];
	rcam->fmt = f;
	return 0;
}

static const struct rda_format rda_formats[] = {
	{
		.fourcc = V4L2_PIX_FMT_YUYV,
		.mbus_code = MEDIA_BUS_FMT_YUYV8_2X8,
		.bpp = 8,
	},
};

static int rda_formats_init(struct rda_camera_dev *rcam)
{
	const struct rda_format *rda_fmts[ARRAY_SIZE(rda_formats)];
	unsigned int num_fmts = 0, i, j;
	struct v4l2_subdev *subdev = rcam->entity.subdev;
	struct v4l2_subdev_mbus_code_enum mbus_code = {
		.which = V4L2_SUBDEV_FORMAT_ACTIVE,
	};

	while (!v4l2_subdev_call(subdev, pad, enum_mbus_code,
				 NULL, &mbus_code)) {
		for (i = 0; i < ARRAY_SIZE(rda_formats); i++) {
			if (rda_formats[i].mbus_code != mbus_code.code)
				continue;

			/* Code supported, have we got this fourcc yet? */
			for (j = 0; j < num_fmts; j++)
				if (rda_fmts[j]->fourcc == rda_formats[i].fourcc)
					/* Already available */
					break;
			if (j == num_fmts)
				/* new */
				rda_fmts[num_fmts++] = rda_formats + i;
		}
		mbus_code.index++;
	}

	if (!num_fmts)
		return -ENXIO;

	rcam->num_user_formats = num_fmts;
	rcam->user_formats = devm_kcalloc(rcam->dev,
					 num_fmts, sizeof(struct rda_format *),
					 GFP_KERNEL);
	if (!rcam->user_formats)
		return -ENOMEM;

	memcpy(rcam->user_formats, rda_fmts,
	       num_fmts * sizeof(struct rda_format *));
	rcam->current_fmt = rcam->user_formats[0];

	return 0;
}

/*static int rda_camera_set_bus_param(struct rda_camera_dev *rcam)
{
	u32 cfg1 = 0;
	int ret;

	if (rcam->pdata.hsync_act_low)
		cfg1 |= ISI_CFG1_HSYNC_POL_ACTIVE_LOW;
	if (rcam->pdata.vsync_act_low)
		cfg1 |= ISI_CFG1_VSYNC_POL_ACTIVE_LOW;
	if (rcam->pdata.pclk_act_falling)
		cfg1 |= ISI_CFG1_PIXCLK_POL_ACTIVE_FALLING;
	if (rcam->pdata.has_emb_sync)
		cfg1 |= ISI_CFG1_EMB_SYNC;
	if (rcam->pdata.full_mode)
		cfg1 |= ISI_CFG1_FULL_MODE;

	cfg1 |= ISI_CFG1_THMASK_BEATS_16;*/

	/* Enable PM and peripheral clock before operate rda registers */
/*	ret = pm_runtime_resume_and_get(rcam->dev);
	if (ret < 0)
		return ret;

	writel(CTRL_DIS, rcam->regs +ISI_CTRL);
	writel(cfg1, rcam->regs + ISI_CFG1);
	pm_runtime_put(rcam->dev);

	return 0;
}*/


/*static int rda_camera_set_bus_param(struct rda_camera_dev *rcam)
{
	struct v4l2_subdev *sd = rcam->entity.subdev;

	HWP_CAMERA_T *hwp_cam = (HWP_CAMERA_T*)rcam->regs;
	unsigned int ctrl = hwp_cam->CTRL;
	struct v4l2_mbus_config cfg = {.type = V4L2_MBUS_PARALLEL,};
	unsigned int common_flags = RDA_CAM_MBUS_PARA;
	int ret;

	ret = v4l2_subdev_call(sd, video, g_mbus_config, &cfg);
	if (!ret) {
		if (cfg.type == V4L2_MBUS_CSI2)
			common_flags = RDA_CAM_MBUS_CSI2;
		common_flags = soc_mbus_config_compatible(&cfg,
				common_flags);
		if (!common_flags) {
			rda_dbg_camera("%s: Flags incompatible camera 0x%x, host 0x%x\n",
					__func__, cfg.flags, common_flags);
			return -EINVAL;
		}
	} else if (ret != -ENOIOCTLCMD) {
		return ret;
	}
	rda_dbg_camera("%s: Flags cam: 0x%x common: 0x%x\n",
			__func__, cfg.flags, common_flags);

	if (cfg.type == V4L2_MBUS_PARALLEL) {*/
		/* Make choises, based on platform preferences */
/*		if ((common_flags & V4L2_MBUS_HSYNC_ACTIVE_HIGH) &&
				(common_flags & V4L2_MBUS_HSYNC_ACTIVE_LOW)) {
			if (rcam->pdata->hsync_act_low)
				common_flags &= ~V4L2_MBUS_HSYNC_ACTIVE_HIGH;
			else
				common_flags &= ~V4L2_MBUS_HSYNC_ACTIVE_LOW;
		}

		if ((common_flags & V4L2_MBUS_VSYNC_ACTIVE_HIGH) &&
				(common_flags & V4L2_MBUS_VSYNC_ACTIVE_LOW)) {
			if (rcam->pdata->vsync_act_low)
				common_flags &= ~V4L2_MBUS_VSYNC_ACTIVE_HIGH;
			else
				common_flags &= ~V4L2_MBUS_VSYNC_ACTIVE_LOW;
		}

		if ((common_flags & V4L2_MBUS_PCLK_SAMPLE_RISING) &&
				(common_flags & V4L2_MBUS_PCLK_SAMPLE_FALLING)) {
			if (rcam->pdata->pclk_act_falling)
				common_flags &= ~V4L2_MBUS_PCLK_SAMPLE_RISING;
			else
				common_flags &= ~V4L2_MBUS_PCLK_SAMPLE_FALLING;
		}*/
		/* set bus param for host */
/*	if (common_flags & V4L2_MBUS_HSYNC_ACTIVE_HIGH)
			ctrl &= ~CAMERA_HREF_POL_INVERT;
		else
			ctrl |= CAMERA_HREF_POL_INVERT;
		if (common_flags & V4L2_MBUS_VSYNC_ACTIVE_HIGH)
			ctrl &= ~CAMERA_VSYNC_POL_INVERT;
		else
			ctrl |= CAMERA_VSYNC_POL_INVERT;
		if (common_flags & V4L2_MBUS_PCLK_SAMPLE_RISING)
			ctrl &= ~CAMERA_PIXCLK_POL_INVERT;
		else
			ctrl |= CAMERA_PIXCLK_POL_INVERT;
		hwp_cam->CTRL = ctrl;
	}

	cfg.flags = common_flags;
	ret = v4l2_subdev_call(sd, video, s_mbus_config, &cfg);
	if (ret < 0 && ret != -ENOIOCTLCMD) {
		rda_dbg_camera("%s: camera s_mbus_config(0x%x) returned %d\n",
				__func__, common_flags, ret);
		return ret;
	}

	return 0;
}*/

static int rda_graph_notify_complete(struct v4l2_async_notifier *notifier)
{
	struct rda_camera_dev *rcam = container_of(notifier, struct rda_camera_dev, notifier);
	int ret;

	rcam->vdev->ctrl_handler = rcam->entity.subdev->ctrl_handler;
	ret = rda_formats_init(rcam);
	if (ret) {
		dev_err(rcam->dev, "No supported mediabus format found\n");
		return ret;
	}
	/*ret = rda_camera_set_bus_param(rcam);
	if (ret) {
		dev_err(rcam->dev, "Can't wake up device\n");
		return ret;
	}*/

	ret = rda_set_default_fmt(rcam);
	if (ret) {
		dev_err(rcam->dev, "Could not set default format\n");
		return ret;
	}

	ret = video_register_device(rcam->vdev, VFL_TYPE_VIDEO, -1);
	if (ret) {
		dev_err(rcam->dev, "Failed to register video device\n");
		return ret;
	}

	dev_dbg(rcam->dev, "Device registered as %s\n", video_device_node_name(rcam->vdev));
	return 0;
}

static const struct v4l2_async_notifier_operations rda_graph_notify_ops = {
	.bound = rda_graph_notify_bound,
	.unbind = rda_graph_notify_unbind,
	.complete = rda_graph_notify_complete,
};

/* Definition for isi_platform_data */
#define RDA_DATAWIDTH_8				0x01
#define RDA_DATAWIDTH_10			0x02

static int rda_parse_dt(struct rda_camera_dev *rcam, struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct v4l2_fwnode_endpoint ep = { .bus_type = 0 };
	int err;

	/* Default settings for rcam */
	rcam->pdata.full_mode = 1;

	np = of_graph_get_next_endpoint(np, NULL);
	if (!np) {
		dev_err(&pdev->dev, "Could not find the endpoint\n");
		return -EINVAL;
	}

	err = v4l2_fwnode_endpoint_parse(of_fwnode_handle(np), &ep);
	of_node_put(np);
	if (err)
	{
		dev_err(&pdev->dev, "Could not parse the endpoint\n");
		return err;
	}

	switch (ep.bus.parallel.bus_width) {
	case 8:
		rcam->pdata.data_width_flags = RDA_DATAWIDTH_8;
		break;
	case 10:
		rcam->pdata.data_width_flags = RDA_DATAWIDTH_8 | RDA_DATAWIDTH_10;
		break;
	default:
		dev_err(&pdev->dev, "Unsupported bus width: %d\n",
				ep.bus.parallel.bus_width);
		return -EINVAL;
	}

	if (ep.bus.parallel.flags & V4L2_MBUS_HSYNC_ACTIVE_LOW)
		rcam->pdata.hsync_act_low = true;
	if (ep.bus.parallel.flags & V4L2_MBUS_VSYNC_ACTIVE_LOW)
		rcam->pdata.vsync_act_low = true;
	if (ep.bus.parallel.flags & V4L2_MBUS_PCLK_SAMPLE_FALLING)
		rcam->pdata.pclk_act_falling = true;

	if (ep.bus_type == V4L2_MBUS_BT656)
		rcam->pdata.has_emb_sync = true;

	return 0;
}

static int rda_camera_probe(struct platform_device *pdev)
{
	unsigned int irq;
	struct rda_camera_dev *rcam;
	struct clk *pclk;
	struct resource *regs;
	struct vb2_queue *q;
	struct v4l2_async_subdev *asd;
	struct device_node *ep;
	int ret;
	
	dev_info(&pdev->dev, "rda_camera_probe\n");

	regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	
	if (!regs) return -ENXIO;

	pclk = clk_get(&pdev->dev, NULL);
	
	if (IS_ERR(pclk)) 
	{
		dev_err(&pdev->dev, "could not get clock\n");
		return PTR_ERR(pclk);
	}
	
	
	if (ret) 
	{
		dev_err(&pdev->dev, "do not enable the specified clock\n");
		goto err_clk_prepare;
	}

	rcam = kzalloc(sizeof(struct rda_camera_dev), GFP_KERNEL);
	
	if (!rcam) {
		ret = -ENOMEM;
		dev_err(&pdev->dev, "%s: Can't allocate interface!\n", __func__);
		goto err_alloc_rcam;
	}
	
	ret = rda_parse_dt(rcam, pdev);
	if (ret)
		return ret;

	rcam->pclk = pclk;
	rcam->active = NULL;
	rcam->next = NULL;
	mutex_init(&rcam->lock);
	spin_lock_init(&rcam->irqlock);
	INIT_DELAYED_WORK(&rcam->isr_work, handle_vsync);
//	tasklet_init(&rcam_tasklet, handle_vsync, (unsigned long)rcam);
	init_waitqueue_head(&rcam->vsync_wq);
	INIT_LIST_HEAD(&rcam->cap_buffer_list);


	rcam->regs = ioremap(regs->start, resource_size(regs));
	
	if (!rcam->regs) {
		ret = -ENOMEM;
		goto err_ioremap;
	}
	cam_regs = rcam->regs;

	irq = platform_get_irq(pdev, 0);
	if (IS_ERR_VALUE(irq)) {
		ret = irq;
		goto err_req_irq;
	}
	ret = request_irq(irq, rda_camera_isr, IRQF_SHARED, pdev->name, rcam);
	if (ret) {
		dev_err(&pdev->dev,  "%s: Unable to request  irq %d\n", __func__, irq);
		goto err_req_irq;
	}
	rcam->irq = irq;
	
	rcam->vdev = video_device_alloc();
	rcam->dev = &pdev->dev;
	if (!rcam->vdev) {
		ret = -ENOMEM;
		goto err_vdev_alloc;
	}
	
		/* Initialize the top-level structure */
	ret = v4l2_device_register(&pdev->dev, &rcam->v4l2_dev);
	if (ret)
		return ret;

	rcam->vdev = video_device_alloc();
	if (!rcam->vdev) {
		ret = -ENOMEM;
		goto err_vdev_alloc;
	}

	/* video node */
	rcam->vdev->fops = &rda_fops;
	rcam->vdev->v4l2_dev = &rcam->v4l2_dev;
	rcam->vdev->queue = &rcam->queue;
	strscpy(rcam->vdev->name, KBUILD_MODNAME, sizeof(rcam->vdev->name));
	rcam->vdev->release = video_device_release;
	rcam->vdev->ioctl_ops = &rda_ioctl_ops;
	rcam->vdev->device_caps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING | V4L2_CAP_READWRITE;
	video_set_drvdata(rcam->vdev, rcam);
	
	/* buffer queue */
	q = &rcam->queue;
	
	q->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	q->io_modes = VB2_MMAP | VB2_USERPTR;
	q->drv_priv = rcam;
	q->buf_struct_size = sizeof(struct cap_buffer);
	q->ops = &rda_video_qops;
	q->mem_ops = &vb2_dma_contig_memops;
	q->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;
	q->min_buffers_needed = 0;
	q->dev = &pdev->dev;
	q->lock = &rcam->lock;
	
	ret = vb2_queue_init(q);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to initialize VB2 queue\n");
		goto err_vdev_alloc;
	}
	
	ep = of_graph_get_next_endpoint(pdev->dev.of_node, NULL);
	if (!ep) return -EINVAL;

	v4l2_async_nf_init(&rcam->notifier);

	asd = v4l2_async_nf_add_fwnode_remote(&rcam->notifier,  of_fwnode_handle(ep), struct v4l2_async_subdev);
	of_node_put(ep);

	if (IS_ERR(asd)) return PTR_ERR(asd);

	rcam->notifier.ops = &rda_graph_notify_ops;

	ret = v4l2_async_nf_register(&rcam->v4l2_dev, &rcam->notifier);
	
	if (ret < 0)
	{
		dev_err(&pdev->dev, "Notifier registration failed\n");
		v4l2_async_nf_cleanup(&rcam->notifier);
		goto err_vdev_alloc;
	}
	
	//rcam_clk(true, 26);
	
	dev_info(&pdev->dev, "Probe complite\n");
	
	//int d,c;
	
	//d = (5 << 16) | 5;
	//c = (5 << 16) | 5;
	
	//rcam_config_csi(d, c, rcam->fmt.fmt.pix.height, 0);
	
	
	return 0;

err_vdev_alloc:
	free_irq(rcam->irq, rcam);
err_req_irq:
	cam_regs = NULL;
	iounmap(rcam->regs);
err_ioremap:
//	tasklet_kill(&rcam_tasklet);
	kfree(rcam);
err_alloc_rcam:
err_clk_prepare:
	clk_put(pclk);

	return ret;
}

static int rda_camera_remove(struct platform_device *pdev)
{
	struct rda_camera_dev *rcam = platform_get_drvdata(pdev);

	free_irq(rcam->irq, rcam);
	cam_regs = NULL;
	iounmap(rcam->regs);
	clk_put(rcam->pclk);
	kfree(rcam);
	
	v4l2_async_nf_unregister(&rcam->notifier);
	v4l2_async_nf_cleanup(&rcam->notifier);
	v4l2_device_unregister(&rcam->v4l2_dev);
	
	

	return 0;
}

#ifdef CONFIG_PM
static int rda_camera_runtime_suspend(struct device *dev)
{
	struct rda_camera_dev *rcam = dev_get_drvdata(dev);

	clk_disable_unprepare(rcam->pclk);

	return 0;
}
static int rda_camera_runtime_resume(struct device *dev)
{
	struct rda_camera_dev *rcam = dev_get_drvdata(dev);

	return clk_prepare_enable(rcam->pclk);
}
#endif /* CONFIG_PM */

static const struct dev_pm_ops rda_camera_dev_pm_ops = {
	SET_RUNTIME_PM_OPS(rda_camera_runtime_suspend,
				rda_camera_runtime_resume, NULL)
};

static const struct of_device_id rda_mmc_dt_matches[] = {
	{ .compatible = "rda,8810pl-csi" },
	{ }
};
MODULE_DEVICE_TABLE(of, rda_mmc_dt_matches);

static struct platform_driver rda_camera_driver = {
	.probe = rda_camera_probe,
	.remove = rda_camera_remove,
	.driver = {
		.name = RDA_CAMERA_DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = rda_mmc_dt_matches,
		.pm	= &rda_camera_dev_pm_ops,
	},
};

static int __init rda_camera_init_module(void)
{
	return platform_driver_probe(&rda_camera_driver, &rda_camera_probe);
}

static void __exit rda_camera_exit(void)
{
	platform_driver_unregister(&rda_camera_driver);
}

module_init(rda_camera_init_module);
module_exit(rda_camera_exit);

EXPORT_SYMBOL(rcam_pdn);
EXPORT_SYMBOL(rcam_rst);
EXPORT_SYMBOL(rcam_clk);
EXPORT_SYMBOL(rcam_config_csi);

MODULE_AUTHOR("Wei Xing <xingwei@rdamicro.com>");
MODULE_DESCRIPTION("The V4L2 driver for RDA camera");
MODULE_LICENSE("GPL");

