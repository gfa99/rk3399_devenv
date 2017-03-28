#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/kthread.h>
#include <dt-bindings/gpio/gpio.h>

struct xgpio_device_t {
	int gpio;
	struct device * dev;
};

static ssize_t xgpio_state_show(struct device * dev, struct device_attribute * attr, char * buf)
{
	struct xgpio_device_t * xdev = dev_get_drvdata(dev);

	if(!strcmp(attr->attr.name, "state"))
	{
		if(gpio_direction_input(xdev->gpio) == 0)
			return strlcpy(buf, "0\n", 3);
		else
			return strlcpy(buf, "1\n", 3);
	}
	return strlcpy(buf, "0\n", 3);
}

static ssize_t xgpio_state_store(struct device * dev, struct device_attribute * attr, const char * buf, size_t count)
{
	struct xgpio_device_t * xdev = dev_get_drvdata(dev);
	unsigned long on = simple_strtoul(buf, NULL, 10);

	if(!strcmp(attr->attr.name, "state"))
	{
		if(on)
			gpio_direction_output(xdev->gpio, 1);
		else
			gpio_direction_output(xdev->gpio, 0);
	}
	return count;
}

static DEVICE_ATTR(state, 0664, xgpio_state_show, xgpio_state_store);
static struct attribute * xgpio_attrs[] = {
	&dev_attr_state.attr,
	NULL
};

static const struct attribute_group xgpio_group = {
	.attrs = xgpio_attrs,
};

static int xgpio_probe(struct platform_device * pdev)
{
	struct device_node * node = pdev->dev.of_node;
	struct xgpio_device_t * xdev;
	enum of_gpio_flags flags;
	int gpio;

	if(!node)
		return -ENODEV;
	
	gpio = of_get_named_gpio_flags(node, "gpio", 0, &flags);
	if(!gpio_is_valid(gpio))
	{
		printk("xgpio: invalid gpio %d\n", gpio);
		return -EINVAL;
	}
	
	if(devm_gpio_request(&pdev->dev, gpio, "xgpio-pin") != 0)
	{
		printk("xgpio: can not request gpio %d\n", gpio);
		return -EINVAL;
	}

	xdev = devm_kzalloc(&pdev->dev, sizeof(struct xgpio_device_t), GFP_KERNEL);
	if(!xdev)
		return -ENOMEM;

	xdev->gpio = gpio;
	xdev->dev = &pdev->dev;
	dev_set_drvdata(&pdev->dev, xdev);

	return sysfs_create_group(&pdev->dev.kobj, &xgpio_group);
}

static int xgpio_remove(struct platform_device *pdev)
{
	struct xgpio_device_t * xdev = dev_get_drvdata(&pdev->dev);

	devm_gpio_free(&pdev->dev, xdev->gpio);
	sysfs_remove_group(&pdev->dev.kobj, &xgpio_group);
	return 0;
}

#ifdef CONFIG_PM
static int xgpio_suspend(struct device *dev)
{
	return 0;
}

static int xgpio_resume(struct device *dev)
{
	return 0;
}
#else
#define xgpio_suspend NULL
#define xgpio_resume NULL
#endif

static const struct dev_pm_ops xgpio_pm_ops = {
	.suspend = xgpio_suspend,
	.resume = xgpio_resume,
};

static struct of_device_id xgpio_of_match[] = {
	{ .compatible = "9tripod,xgpio" },
	{},
};
MODULE_DEVICE_TABLE(of, xgpio_of_match);

static struct platform_driver xgpio_driver = {
	.driver		= {
		.name	= "xgpio",
		.owner	= THIS_MODULE,
		.pm	= &xgpio_pm_ops,
		.of_match_table	= of_match_ptr(xgpio_of_match),
	},
	.probe		= xgpio_probe,
	.remove		= xgpio_remove,
};
module_platform_driver(xgpio_driver);

MODULE_DESCRIPTION("9tripod xgpio driver");
MODULE_AUTHOR("Jianjun Jiang, 8192542@qq.com");
MODULE_LICENSE("GPL");
