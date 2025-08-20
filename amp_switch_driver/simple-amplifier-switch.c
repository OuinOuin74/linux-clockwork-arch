// SPDX-License-Identifier: GPL-2.0
/*
 * simple-amplifier-switch.c
 *
 * Drive one or more "amplifier enable" GPIOs based on a single
 * "jack detect" GPIO. Intended for uConsole, but generic enough.
 *
 * Behavior with typical wiring:
 *   - sw-gpios:  jack detect, active-low   (0 = plug inserted)
 *   - outputs:   amplifier enable, active-high (1 = speakers ON)
 *
 * Therefore:
 *   plug OUT  -> sw=1 -> outputs=1 -> speakers ON
 *   plug IN   -> sw=0 -> outputs=0 -> speakers OFF
 *
 * We use a THREADED IRQ so we can safely call *_cansleep() helpers.
 */

#include <linux/bitmap.h>
#include <linux/interrupt.h>
#include <linux/gpio/consumer.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/slab.h>

struct simple_amplifier_switch {
	struct gpio_desc *sw;        /* jack detect GPIO */
	struct gpio_descs *outputs;  /* array of amplifier enable GPIOs */
	u32 debounce_us;             /* optional debounce from DT */
	int irq;
};

static inline void sas_set_outputs(struct gpio_descs *outputs, int value)
{
	unsigned long *values;
	int n = outputs->ndescs;

	values = bitmap_alloc(n, GFP_KERNEL);
	if (!values)
		return;

	if (value)
		bitmap_fill(values, n);
	else
		bitmap_zero(values, n);

	/* Safe in threaded context */
	gpiod_set_array_value_cansleep(n, outputs->desc, outputs->info, values);
	bitmap_free(values);
}

/* Top-half: wake the threaded handler only */
static irqreturn_t sas_isr(int irq, void *data)
{
	return IRQ_WAKE_THREAD;
}

/* Bottom-half (thread): allowed to sleep */
static irqreturn_t sas_thread(int irq, void *data)
{
	struct simple_amplifier_switch *sas = data;
	int state = gpiod_get_value_cansleep(sas->sw);

	sas_set_outputs(sas->outputs, state);
	return IRQ_HANDLED;
}

static int simple_amplifier_switch_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct simple_amplifier_switch *sas;
	int ret, initial;

	sas = devm_kzalloc(dev, sizeof(*sas), GFP_KERNEL);
	if (!sas)
		return -ENOMEM;

	/* sw-gpios: input jack-detect (polarity handled by DT flag) */
	sas->sw = devm_gpiod_get(dev, "sw", GPIOD_IN);
	if (IS_ERR(sas->sw))
		return dev_err_probe(dev, PTR_ERR(sas->sw),
				     "failed to get sw-gpios\n");

	/* outputs-gpios: array of outputs, default low at request time */
	sas->outputs = devm_gpiod_get_array(dev, "outputs", GPIOD_OUT_LOW);
	if (IS_ERR(sas->outputs))
		return dev_err_probe(dev, PTR_ERR(sas->outputs),
				     "failed to get outputs-gpios\n");

	/* Optional debounce (in microseconds) via DT property "debounce-us" */
	if (!device_property_read_u32(dev, "debounce-us", &sas->debounce_us)) {
		ret = gpiod_set_debounce(sas->sw, sas->debounce_us);
		if (ret)
			dev_dbg(dev, "debounce not supported (%d), continuing\n", ret);
	}

	/* Apply initial state (this gives you speakers ON by default if no plug) */
	initial = gpiod_get_value_cansleep(sas->sw);
	sas_set_outputs(sas->outputs, initial);

	/* Request threaded IRQ on both edges */
	sas->irq = gpiod_to_irq(sas->sw);
	if (sas->irq < 0)
		return dev_err_probe(dev, sas->irq, "failed to map sw IRQ\n");

	ret = devm_request_threaded_irq(dev, sas->irq,
					sas_isr, sas_thread,
					IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
					dev_name(dev), sas);
	if (ret)
		return dev_err_probe(dev, ret, "failed to request threaded IRQ\n");

	platform_set_drvdata(pdev, sas);
	dev_info(dev, "simple-amplifier-switch ready (initial=%d)\n", initial);
	return 0;
}

static void simple_amplifier_switch_shutdown(struct platform_device *pdev)
{
	struct simple_amplifier_switch *sas = platform_get_drvdata(pdev);

	/* devm_* will free the IRQ; just ensure the amp is OFF on shutdown */
	if (sas && sas->outputs)
		sas_set_outputs(sas->outputs, 0);
}

#ifdef CONFIG_PM_SLEEP
static int sas_resume(struct device *dev)
{
	struct simple_amplifier_switch *sas = dev_get_drvdata(dev);
	int state;

	if (!sas)
		return 0;

	state = gpiod_get_value_cansleep(sas->sw);
	sas_set_outputs(sas->outputs, state);
	return 0;
}

static const struct dev_pm_ops sas_pm_ops = {
	.resume  = sas_resume,
	.restore = sas_resume,
};
#define SAS_PM_OPS (&sas_pm_ops)
#else
#define SAS_PM_OPS NULL
#endif

static const struct of_device_id sas_of_match[] = {
	{ .compatible = "simple-amplifier-switch" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, sas_of_match);

static struct platform_driver sas_driver = {
	.probe    = simple_amplifier_switch_probe,
	.shutdown = simple_amplifier_switch_shutdown,
	.driver   = {
		.name           = "simple-amplifier-switch",
		.of_match_table = sas_of_match,
		.pm             = SAS_PM_OPS,
	},
};
module_platform_driver(sas_driver);

MODULE_AUTHOR("PotatoMania; updates by ChatGPT");
MODULE_DESCRIPTION("GPIO-based amplifier switch (jack detect -> amp enable)");
MODULE_LICENSE("GPL v2");
