/* linux/arch/arm/plat-rda/gpio.c
 *
 * Copyright (c) 2012-, RDA micro inc. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/bitops.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/irqchip/chained_irq.h>
#include <asm/mach/irq.h>
#include <linux/gpio/driver.h>
#include <rda/mach/gpio_id.h>
#include "gpio_hw.h"

#define RDA_GPIO_OEN_VAL		0x00
#define RDA_GPIO_OEN_SET_OUT		0x04
#define RDA_GPIO_OEN_SET_IN		0x08
#define RDA_GPIO_VAL			0x0c
#define RDA_GPIO_SET			0x10
#define RDA_GPIO_CLR			0x14
#define RDA_GPIO_INT_CTRL_SET		0x18
#define RDA_GPIO_INT_CTRL_CLR		0x1c
#define RDA_GPIO_INT_CLR		0x20
#define RDA_GPIO_INT_STATUS		0x24

#define RDA_GPIO_IRQ_MASK		0xff


#define RDA_GPIO_BANK_NR	32  // Each bank consists of 32 GPIOs 


#define	NR_GPIO_BANK_IRQS	8 // for every gpio bank, only 8 gpio pins can be input interrupt line

#define FIRST_GPIO_IRQ 32

/*
  GPIO as interrupt, for every gpio bank, only the gpio refer to register
  GPIO_OEN_VAL bits 0~7 can be interrupt input pins.
   ____________________________________________________________________________
  |      31~24       |      23~16       |     8~15      |     0~7              |
  |__________________|__________________|_______________|______________________|
  |                            normal gpios             |   input interrupt    |
  |_____________________________________________________|______________________|

 */

#define RISING_SHIFT	(0)
#define FALLING_SHIFT	(8)
#define DEBOUCE_SHIFT	(16)
#define LEVEL_MODE_SHIFT	(24)

enum {
	GPIO,
	GPO
};

struct rda_gpio_chip {
	spinlock_t		lock;
	struct gpio_chip	chip;
	int type;
	void __iomem *base;
	int irq;
	int gpio_base;
};

static int  __set_gpio_irq_type(struct rda_gpio_chip *rda_chip,
		unsigned offset, unsigned int flow_type);

static int rda_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	struct rda_gpio_chip *rda_chip;

	rda_chip = container_of(chip, struct rda_gpio_chip, chip);
	if (rda_chip->type == GPO) {
		printk(KERN_ERR "can not set gpo with input");
		return -EINVAL;
	}

//	pr_debug("set gpio %d input \n", chip->base + offset);
	writel(BIT(offset), rda_chip->base + RDA_GPIO_OEN_SET_IN);
	return 0;
}

static int
rda_gpio_direction_output(struct gpio_chip *chip, unsigned offset, int value)
{
	struct rda_gpio_chip *rda_chip;

	rda_chip = container_of(chip, struct rda_gpio_chip, chip);
	if (rda_chip->type == GPIO) {
		pr_debug("set gpio %d output value:%d\n",
				chip->base + offset,  value);
		writel(BIT(offset), rda_chip->base + RDA_GPIO_OEN_SET_OUT);
	}

	if (value) 
		writel(BIT(offset), rda_chip->base + RDA_GPIO_SET);
	else 
		writel(BIT(offset), rda_chip->base + RDA_GPIO_CLR);

	return 0;
}

static int rda_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	struct rda_gpio_chip *rda_chip;
	int val;

	pr_debug("get gpio %d value:\n", chip->base + offset);
	rda_chip = container_of(chip, struct rda_gpio_chip, chip);
	if (rda_chip->type == GPO) { //gpo 
		return (readl(rda_chip->base + RDA_GPIO_SET) & (BIT(offset))) ? 1 : 0;
	}

	if (readl(rda_chip->base + RDA_GPIO_OEN_VAL) & BIT(offset))   //gpio input
		val = (readl(rda_chip->base + RDA_GPIO_VAL) & BIT(offset)) ? 1 : 0;
	
	else 	//gpio output
		val = (readl(rda_chip->base + RDA_GPIO_SET) & BIT(offset)) ? 1 : 0;

	return val;
}

static void rda_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	struct rda_gpio_chip *rda_chip;

	rda_chip = container_of(chip, struct rda_gpio_chip, chip);
	//pr_debug("set gpio value:%d\n", value);
	//pr_debug("set gpio regs:%p to %lx\n",
	//			rda_chip->base + RDA_GPIO_SET, BIT(offset));
	if (value) 
		writel(BIT(offset), rda_chip->base + RDA_GPIO_SET);
	else 
		writel(BIT(offset), rda_chip->base + RDA_GPIO_CLR);
}





static int rda_gpio_request(struct gpio_chip *chip, unsigned offset)
{
	//return rda_gpiomux_get(chip->base + offset);
	return 0;
}

static void rda_gpio_free(struct gpio_chip *chip, unsigned offset)
{
	//rda_gpiomux_put(chip->base + offset);
}


static void rda_gpio_irq_ack(struct irq_data *d)
{
	struct gpio_chip *chip = irq_data_get_irq_chip_data(d);
	struct rda_gpio_chip *rda_gpio = gpiochip_get_data(chip);
	u32 offset = irqd_to_hwirq(d);

	writel(BIT(offset), rda_gpio->base + RDA_GPIO_INT_CLR);
}

static void rda_gpio_irq_mask(struct irq_data *d)
{
	struct gpio_chip *chip = irq_data_get_irq_chip_data(d);
	struct rda_gpio_chip *rda_gpio = gpiochip_get_data(chip);
	u32 offset = irqd_to_hwirq(d);
	unsigned value;

	//printk("%s, gpio %d\n", __func__, rda_gpio->chip.base + offset);
	value = BIT(offset) << RISING_SHIFT;
	value |= BIT(offset) << FALLING_SHIFT;
	writel(value, rda_gpio->base + RDA_GPIO_INT_CTRL_CLR);
}

static void rda_gpio_irq_unmask(struct irq_data *d)
{
	struct gpio_chip *chip = irq_data_get_irq_chip_data(d);
	struct rda_gpio_chip *rda_gpio = gpiochip_get_data(chip);
	u32 offset = irqd_to_hwirq(d);
	u32 trigger = irqd_get_trigger_type(d);

	//printk("%s, gpio %d\n", __func__, rda_gpio->chip.base + offset);
	__set_gpio_irq_type(rda_gpio, offset, trigger);
}

static int rda_gpio_irq_set_wake(struct irq_data *d, unsigned int on)
{
	return 0;
}

static int  __set_gpio_irq_type(struct rda_gpio_chip *rda_chip,
		unsigned offset, unsigned int flow_type)
{
	unsigned long irq_flags;
	u32 value;
	int ret = 0;

	spin_lock_irqsave(&rda_chip->lock, irq_flags);
	switch (flow_type & IRQ_TYPE_SENSE_MASK) {
	case IRQ_TYPE_EDGE_RISING:
		value = BIT(offset) << RISING_SHIFT;
		writel(value, rda_chip->base + RDA_GPIO_INT_CTRL_SET);
		value = BIT(offset) << LEVEL_MODE_SHIFT;
		writel(value, rda_chip->base + RDA_GPIO_INT_CTRL_CLR);
		break;

	case IRQ_TYPE_EDGE_FALLING:
		value = BIT(offset) << FALLING_SHIFT;
		writel(value, rda_chip->base + RDA_GPIO_INT_CTRL_SET);
		value = BIT(offset) << LEVEL_MODE_SHIFT;
		writel(value, rda_chip->base + RDA_GPIO_INT_CTRL_CLR);
		break;

	case IRQ_TYPE_EDGE_BOTH:
		value = BIT(offset) << RISING_SHIFT;
		value |= BIT(offset) << FALLING_SHIFT;
		writel(value, rda_chip->base + RDA_GPIO_INT_CTRL_SET);

		value = BIT(offset) << LEVEL_MODE_SHIFT;
		writel(value, rda_chip->base + RDA_GPIO_INT_CTRL_CLR);
		break;

	case IRQ_TYPE_LEVEL_HIGH:
		value = BIT(offset) << RISING_SHIFT;
		value |= BIT(offset) << LEVEL_MODE_SHIFT;
		writel(value, rda_chip->base + RDA_GPIO_INT_CTRL_SET);
		break;

	case IRQ_TYPE_LEVEL_LOW:
		value = BIT(offset) << FALLING_SHIFT;
		value |= BIT(offset) << LEVEL_MODE_SHIFT;
		writel(value, rda_chip->base + RDA_GPIO_INT_CTRL_SET);
		break;

	default:
		ret =  -EINVAL;
		break;
	}
	spin_unlock_irqrestore(&rda_chip->lock, irq_flags);
	return ret;
}

static int rda_gpio_irq_set_type(struct irq_data *d, unsigned int flow_type)
{
	struct gpio_chip *chip = irq_data_get_irq_chip_data(d);
	struct rda_gpio_chip *rda_gpio = gpiochip_get_data(chip);
	u32 offset = irqd_to_hwirq(d);
	int ret;
	
	if ((BIT(offset) & RDA_GPIO_IRQ_MASK) == 0) {
		printk(KERN_ERR "only bits %x in the gpio bank can be interrupt line\n", RDA_GPIO_IRQ_MASK);
		return -EINVAL;
	}
	
	//printk("%s, gpio %d\n",__func__, rda_gpio->chip.base + offset);

	ret = __set_gpio_irq_type(rda_gpio, offset, flow_type);
	if (ret) return ret;
	if (flow_type & (IRQ_TYPE_LEVEL_LOW | IRQ_TYPE_LEVEL_HIGH))
	{
		irq_set_handler_locked(d, handle_level_irq);
	}
	
	else if (flow_type & (IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING))
	{
		irq_set_handler_locked(d, handle_edge_irq);
	}
	return 0;
}



static void rda_gpio_irq_handler(struct irq_desc *desc)
{
	struct irq_chip *ic = irq_desc_get_chip(desc);
	struct rda_gpio_chip *rda_chip = irq_desc_get_handler_data(desc);
	u32 n;
	int gpio_irq;
	unsigned long status;
	
	chained_irq_enter(ic, desc);
	
	status = readl_relaxed(rda_chip->base + RDA_GPIO_INT_STATUS);
	/* Only lower 8 bits are capable of generating interrupts */
	status &= RDA_GPIO_IRQ_MASK;

	for_each_set_bit(n, &status, RDA_GPIO_BANK_NR) generic_handle_domain_irq(rda_chip->chip.irq.domain, n);
		
	chained_irq_exit(ic, desc);
}



static struct irq_chip rda_gpio_irq_chip = {
	.name          = "rdagpio",
	.irq_ack       = rda_gpio_irq_ack,
	.irq_mask      = rda_gpio_irq_mask,
	.irq_unmask    = rda_gpio_irq_unmask,
	.irq_set_wake  = rda_gpio_irq_set_wake,
	.irq_set_type  = rda_gpio_irq_set_type,
	.irq_disable = rda_gpio_irq_mask,
	.flags		= IRQCHIP_IMMUTABLE,
};


static int rda_gpio_probe_old(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	unsigned offset;
	struct rda_gpio_chip *rda_gpio;
	u32 id;
	u32 ngpios;
	int ret;
	int i;
	
	
	
	rda_gpio = devm_kzalloc(dev, sizeof(*rda_gpio), GFP_KERNEL);
	if (!rda_gpio)
		return -ENOMEM;

	ret = device_property_read_u32(dev, "ngpios", &ngpios);
	
	if (ret < 0) return ret;
		
	ret = device_property_read_u32(dev, "id", &id);
	
	if (ret < 0) return ret;
	
	
	rda_gpio->irq = platform_get_irq(pdev, 0);

	rda_gpio->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(rda_gpio->base))
		return PTR_ERR(rda_gpio->base);
	
	
	rda_gpio->chip.base = -1;
	dev_info(&pdev->dev, "rda_gpio_old rda_base = %08x, gpio_base = %d,   irq = %d\n", rda_gpio->base, rda_gpio->chip.base,  rda_gpio->irq);
	spin_lock_init(&rda_gpio->lock);
	
	
	
	rda_gpio->chip.label = dev_name(dev);
	rda_gpio->chip.ngpio = ngpios;		
	rda_gpio->chip.get = rda_gpio_get;				
	rda_gpio->chip.set = rda_gpio_set;				
	rda_gpio->chip.direction_input = rda_gpio_direction_input;	
	rda_gpio->chip.direction_output = rda_gpio_direction_output;			
	rda_gpio->chip.request = rda_gpio_request;			
	rda_gpio->chip.free = rda_gpio_free;
	rda_gpio->type = GPIO;
	rda_gpio->gpio_base = rda_gpio->chip.base;
	
	ret = bgpio_init(&rda_gpio->chip, dev, 4,
			 rda_gpio->base + RDA_GPIO_VAL,
			 rda_gpio->base + RDA_GPIO_SET,
			 rda_gpio->base + RDA_GPIO_CLR,
			 rda_gpio->base + RDA_GPIO_OEN_SET_OUT,
			 rda_gpio->base + RDA_GPIO_OEN_SET_IN,
			 BGPIOF_READ_OUTPUT_REG_SET);
			 
	if (ret)
	{
		dev_err(dev, "bgpio_init failed\n");
		return ret;
	}
	
	if (rda_gpio->irq >= 0)
	{
	
		for (i = FIRST_GPIO_IRQ + id * NR_GPIO_BANK_IRQS; i < (FIRST_GPIO_IRQ + id * NR_GPIO_BANK_IRQS) + NR_GPIO_BANK_IRQS; i++)
		{
			//dev_info(&pdev->dev,"map irq %d with bank %d\n", i, id);
			irq_set_chip_data(i, rda_gpio);
			irq_set_chip_and_handler(i, &rda_gpio_irq_chip, handle_level_irq);
			offset = i % NR_GPIO_BANK_IRQS;
			writel(BIT(offset), rda_gpio->base + RDA_GPIO_INT_CLR);
		}
		
		
		
		gpio_irq_chip_set_chip(&rda_gpio->chip.irq, &rda_gpio_irq_chip);
		rda_gpio->chip.irq.handler = handle_bad_irq;
		rda_gpio->chip.irq.default_type = IRQ_TYPE_NONE;
		rda_gpio->chip.irq.parent_handler = rda_gpio_irq_handler;
		rda_gpio->chip.irq.parent_handler_data = rda_gpio;
		rda_gpio->chip.irq.num_parents = 1;
		rda_gpio->chip.irq.parents = devm_kcalloc(dev, 1,  sizeof(rda_gpio->chip.irq.parents), GFP_KERNEL);
		if (!rda_gpio->chip.irq.parents)
			return -ENOMEM;
		rda_gpio->chip.irq.parents[0] = rda_gpio->irq;
	}
	
	platform_set_drvdata(pdev, rda_gpio);
	
	ret = devm_gpiochip_add_data(dev, &rda_gpio->chip, rda_gpio);
	
	if (ret) {
		dev_err(dev, "Failed to register GPIO lib\n");
		return ret;
	}

	return 0;
}

static const struct of_device_id rda_gpio_old_of_match[] = {
	{ .compatible = "rda,8810pl-gpio-old", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, rda_gpio_old_of_match);

static struct platform_driver rda_gpio_driver_old = {
	.probe = rda_gpio_probe_old,
	.driver = {
		.name = "rda-gpio-old",
		.of_match_table	= rda_gpio_old_of_match,
	},
};

module_platform_driver_probe(rda_gpio_driver_old, rda_gpio_probe_old);

MODULE_DESCRIPTION("RDA Micro GPIO driver");

