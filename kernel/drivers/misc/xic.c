#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/sysfs.h>
#include <linux/ctype.h>
#include <linux/of.h>

enum {
	XIC_VERSION				= 0x00,
	XIC_POWER				= 0x01,
	XIC_WDOG_COUNT			= 0x02,
	XIC_WDOG_CTRL			= 0x03,

	XIC_UNIQUE_ID0			= 0x04,
	XIC_UNIQUE_ID1			= 0x05,
	XIC_UNIQUE_ID2			= 0x06,
	XIC_UNIQUE_ID3			= 0x07,
	XIC_UNIQUE_ID4			= 0x08,
	XIC_UNIQUE_ID5			= 0x09,
	XIC_UNIQUE_ID6			= 0x0a,
	XIC_UNIQUE_ID7			= 0x0b,
	XIC_UNIQUE_ID8			= 0x0c,
	XIC_UNIQUE_ID9			= 0x0d,
	XIC_UNIQUE_ID10			= 0x0e,
	XIC_UNIQUE_ID11			= 0x0f,

	XIC_TIME_GET_SECOND		= 0x10,
	XIC_TIME_GET_MINUTE		= 0x11,
	XIC_TIME_GET_HOUR		= 0x12,
	XIC_TIME_GET_DAY		= 0x13,
	XIC_TIME_GET_MONTH		= 0x14,
	XIC_TIME_GET_YEAR_LO	= 0x15,
	XIC_TIME_GET_YEAR_HI	= 0x16,

	XIC_TIME_SET_SECOND		= 0x20,
	XIC_TIME_SET_MINUTE		= 0x21,
	XIC_TIME_SET_HOUR		= 0x22,
	XIC_TIME_SET_DAY		= 0x23,
	XIC_TIME_SET_MONTH		= 0x24,
	XIC_TIME_SET_YEAR_LO	= 0x25,
	XIC_TIME_SET_YEAR_HI	= 0x26,
	XIC_TIME_SET_CTRL		= 0x27,

	XIC_TIME_ON_SECOND		= 0x30,
	XIC_TIME_ON_MINUTE		= 0x31,
	XIC_TIME_ON_HOUR		= 0x32,
	XIC_TIME_ON_DAY			= 0x33,
	XIC_TIME_ON_MONTH		= 0x34,
	XIC_TIME_ON_YEAR_LO		= 0x35,
	XIC_TIME_ON_YEAR_HI		= 0x36,
	XIC_TIME_ON_CTRL		= 0x37,

	XIC_TIME_OFF_SECOND		= 0x40,
	XIC_TIME_OFF_MINUTE		= 0x41,
	XIC_TIME_OFF_HOUR		= 0x42,
	XIC_TIME_OFF_DAY		= 0x43,
	XIC_TIME_OFF_MONTH		= 0x44,
	XIC_TIME_OFF_YEAR_LO	= 0x45,
	XIC_TIME_OFF_YEAR_HI	= 0x46,
	XIC_TIME_OFF_CTRL		= 0x47,
};

struct rtc_time_t {
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t day;
	uint8_t mon;
	uint16_t year;
};
static struct i2c_client * g_client = NULL;

static int xic_read(struct i2c_client * client, uint8_t reg, uint8_t * buf, int len)
{
	struct i2c_msg msgs[2];

    msgs[0].addr = client->addr;
    msgs[0].flags = 0;
    msgs[0].len = 1;
    msgs[0].buf = &reg;

    msgs[1].addr = client->addr;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len = len;
    msgs[1].buf = buf;

    if(i2c_transfer(client->adapter, msgs, 2) != 2)
    	return 0;
    return 1;
}

static int xic_write(struct i2c_client * client, uint8_t reg, uint8_t * buf, int len)
{
	struct i2c_msg msg;
	uint8_t mbuf[256];

	if(len > sizeof(mbuf) - 1)
		len = sizeof(mbuf) - 1;
	mbuf[0] = reg;
	memcpy(&mbuf[1], buf, len);

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = len + 1;
	msg.buf = &mbuf[0];

    if(i2c_transfer(client->adapter, &msg, 1) != 1)
    	return 0;
    return 1;
}

static int xic_read_version(struct i2c_client * client, uint8_t * version)
{
	uint8_t buf;

	if(xic_read(client, XIC_VERSION, &buf, 1))
	{
		if(version)
			*version = buf;
		return 1;
	}
	return 0;
}

static int xic_get_uniqueid(struct i2c_client * client, uint8_t * id)
{
	uint8_t buf[12];

	if(xic_read(client, XIC_UNIQUE_ID0, &buf[0], 12))
	{
		memcpy(id, buf, 12);
		return 1;
	}
	return 0;
}

static int xic_get_wdog_count(struct i2c_client * client, uint8_t * count)
{
	uint8_t buf;

	if(xic_read(client, XIC_WDOG_COUNT, &buf, 1))
	{
		*count = buf;
		return 1;
	}
	return 0;
}

static int xic_set_wdog_count(struct i2c_client * client, uint8_t count)
{
	uint8_t buf = count > 0 ? count : 60;

	if(xic_write(client, XIC_WDOG_COUNT, &buf, 1))
		return 1;
	return 0;
}

static int xic_get_wdog_status(struct i2c_client * client, uint8_t * status)
{
	uint8_t buf;

	if(xic_read(client, XIC_WDOG_CTRL, &buf, 1))
	{
		*status = buf > 0 ? 1 : 0;
		return 1;
	}
	return 0;
}

static int xic_set_wdog_status(struct i2c_client * client, uint8_t status)
{
	uint8_t buf = status > 0 ? 1 : 0;

	if(xic_write(client, XIC_WDOG_CTRL, &buf, 1))
		return 1;
	return 0;
}

static int xic_get_time(struct i2c_client * client, struct rtc_time_t * time)
{
	uint8_t buf[7];

	if(xic_read(client, XIC_TIME_GET_SECOND, &buf[0], 7))
	{
		time->sec = buf[0];
		time->min = buf[1];
		time->hour = buf[2];
		time->day = buf[3];
		time->mon = buf[4];
		time->year = ((uint16_t)buf[5] << 0) | ((uint16_t)buf[6] << 8);
		return 1;
	}
	return 0;
}

static int xic_set_time(struct i2c_client * client, struct rtc_time_t * time)
{
	uint8_t buf[8];

	buf[0] = time->sec;
	buf[1] = time->min;
	buf[2] = time->hour;
	buf[3] = time->day;
	buf[4] = time->mon;
	buf[5] = (time->year >> 0) & 0xff;
	buf[6] = (time->year >> 8) & 0xff;
	buf[7] = (0x1 << 7);

	if(xic_write(client, XIC_TIME_SET_SECOND, &buf[0], 8))
		return 1;
	return 0;
}

static int xic_get_time_on(struct i2c_client * client, struct rtc_time_t * time)
{
	uint8_t buf[8];

	if(xic_read(client, XIC_TIME_ON_SECOND, &buf[0], 8))
	{
		if(buf[7] & (0x1 << 0))
		{
			time->sec = buf[0];
			time->min = buf[1];
			time->hour = buf[2];
			time->day = buf[3];
			time->mon = buf[4];
			time->year = ((uint16_t)buf[5] << 0) | ((uint16_t)buf[6] << 8);
		}
		else
		{
			time->sec = 0;
			time->min = 0;
			time->hour = 0;
			time->day = 0;
			time->mon = 0;
			time->year = 0;
		}
		return 1;
	}
	return 0;
}

static int xic_set_time_on(struct i2c_client * client, struct rtc_time_t * time, int enable)
{
	uint8_t buf[8];

	buf[0] = time->sec;
	buf[1] = time->min;
	buf[2] = time->hour;
	buf[3] = time->day;
	buf[4] = time->mon;
	buf[5] = (time->year >> 0) & 0xff;
	buf[6] = (time->year >> 8) & 0xff;
	buf[7] = (0x1 << 7) | ((enable > 0 ? 1 : 0) << 0);

	if(xic_write(client, XIC_TIME_ON_SECOND, &buf[0], 8))
		return 1;
	return 0;
}

static int xic_get_time_off(struct i2c_client * client, struct rtc_time_t * time)
{
	uint8_t buf[8];

	if(xic_read(client, XIC_TIME_OFF_SECOND, &buf[0], 8))
	{
		if(buf[7] & (0x1 << 0))
		{
			time->sec = buf[0];
			time->min = buf[1];
			time->hour = buf[2];
			time->day = buf[3];
			time->mon = buf[4];
			time->year = ((uint16_t)buf[5] << 0) | ((uint16_t)buf[6] << 8);
		}
		else
		{
			time->sec = 0;
			time->min = 0;
			time->hour = 0;
			time->day = 0;
			time->mon = 0;
			time->year = 0;
		}
		return 1;
	}
	return 0;
}

static int xic_set_time_off(struct i2c_client * client, struct rtc_time_t * time, int enable)
{
	uint8_t buf[8];

	buf[0] = time->sec;
	buf[1] = time->min;
	buf[2] = time->hour;
	buf[3] = time->day;
	buf[4] = time->mon;
	buf[5] = (time->year >> 0) & 0xff;
	buf[6] = (time->year >> 8) & 0xff;
	buf[7] = (0x1 << 7) | ((enable > 0 ? 1 : 0) << 0);

	if(xic_write(client, XIC_TIME_OFF_SECOND, &buf[0], 8))
		return 1;
	return 0;
}

static int xic_power_off(struct i2c_client * client)
{
	uint8_t buf = 0x55;

	if(xic_write(client, XIC_POWER, &buf, 1))
		return 1;
	return 0;
}

static int xic_power_reboot(struct i2c_client * client)
{
	uint8_t buf = 0xaa;

	if(xic_write(client, XIC_POWER, &buf, 1))
		return 1;
	return 0;
}

void xic_poweroff(void)
{
	if(g_client)
		xic_power_off(g_client);
}
EXPORT_SYMBOL_GPL(xic_poweroff);

void xic_powerreboot(void)
{
	if(g_client)
		xic_power_reboot(g_client);
}
EXPORT_SYMBOL_GPL(xic_powerreboot);

static ssize_t xic_show(struct device * dev, struct device_attribute * attr, char * buf)
{
	struct i2c_client * client = to_i2c_client(dev);
	struct rtc_time_t time;
	uint8_t version;
	uint8_t id[12];

	if(!strcmp(attr->attr.name, "version"))
	{
		if(xic_read_version(client, &version))
			return sprintf(buf, "%02x\n", version);
	}
	else if(!strcmp(attr->attr.name, "uniqueid"))
	{
		if(xic_get_uniqueid(client, &id[0]))
			return  sprintf(buf, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n",
				id[0], id[1], id[2], id[3], id[4], id[5],
				id[6], id[7], id[8], id[9], id[10], id[11]);
	}
	else if(!strcmp(attr->attr.name, "wdogcount"))
	{
		if(xic_get_wdog_count(client, &id[0]))
			return sprintf(buf, "%d\n", id[0]);
	}
	else if(!strcmp(attr->attr.name, "wdogstatus"))
	{
		if(xic_get_wdog_status(client, &id[0]))
			return sprintf(buf, "%d\n", id[0]);
	}
	else if(!strcmp(attr->attr.name, "time"))
	{
		if(xic_get_time(client, &time))
			return sprintf(buf, "%04hu-%02hhu-%02hhu %02hhu:%02hhu:%02hhu\n", time.year, time.mon, time.day, time.hour, time.min, time.sec);
	}
	else if(!strcmp(attr->attr.name, "timeon"))
	{
		if(xic_get_time_on(client, &time))
			return sprintf(buf, "%04hu-%02hhu-%02hhu %02hhu:%02hhu:%02hhu\n", time.year, time.mon, time.day, time.hour, time.min, time.sec);
	}
	else if(!strcmp(attr->attr.name, "timeoff"))
	{
		if(xic_get_time_off(client, &time))
			return sprintf(buf, "%04hu-%02hhu-%02hhu %02hhu:%02hhu:%02hhu\n", time.year, time.mon, time.day, time.hour, time.min, time.sec);
	}
	return strlcpy(buf, "0\n", 3);
}

static ssize_t xic_store(struct device * dev, struct device_attribute * attr, const char * buf, size_t count)
{
	struct i2c_client * client = to_i2c_client(dev);
	struct rtc_time_t time;
	uint8_t enable = 0;
	uint8_t data;

	if(!strcmp(attr->attr.name, "wdogcount"))
	{
		data = simple_strtoul(buf, NULL, 10);
		if(xic_set_wdog_count(client, data))
			return count;
	}
	else if(!strcmp(attr->attr.name, "wdogstatus"))
	{
		data = simple_strtoul(buf, NULL, 10);
		if(xic_set_wdog_status(client, data))
			return count;
	}
	else if(!strcmp(attr->attr.name, "time"))
	{
		if(sscanf(buf, "%04hu-%02hhu-%02hhu %02hhu:%02hhu:%02hhu", &time.year, &time.mon, &time.day, &time.hour, &time.min, &time.sec) == 6)
		{
			if(xic_set_time(client, &time))
				return count;
		}
	}
	else if(!strcmp(attr->attr.name, "timeon"))
	{
		if(sscanf(buf, "%04hu-%02hhu-%02hhu %02hhu:%02hhu:%02hhu %02hhu", &time.year, &time.mon, &time.day, &time.hour, &time.min, &time.sec, &enable) == 7)
		{
			if(xic_set_time_on(client, &time, enable))
				return count;
		}
	}
	else if(!strcmp(attr->attr.name, "timeoff"))
	{
		if(sscanf(buf, "%04hu-%02hhu-%02hhu %02hhu:%02hhu:%02hhu %02hhu", &time.year, &time.mon, &time.day, &time.hour, &time.min, &time.sec, &enable) == 7)
		{
			if(xic_set_time_off(client, &time, enable))
				return count;
		}
	}
	else if(!strcmp(attr->attr.name, "poweroff"))
	{
		if(xic_power_off(client))
			return count;
	}
	else if(!strcmp(attr->attr.name, "powerreboot"))
	{
		if(xic_power_reboot(client))
			return count;
	}
	return 0;
}

static DEVICE_ATTR(version, 0440, xic_show, NULL);
static DEVICE_ATTR(uniqueid, 0440, xic_show, NULL);
static DEVICE_ATTR(wdogcount, 0660, xic_show, xic_store);
static DEVICE_ATTR(wdogstatus, 0660, xic_show, xic_store);
static DEVICE_ATTR(time, 0660, xic_show, xic_store);
static DEVICE_ATTR(timeon, 0660, xic_show, xic_store);
static DEVICE_ATTR(timeoff, 0660, xic_show, xic_store);
static DEVICE_ATTR(poweroff, 0220, NULL, xic_store);
static DEVICE_ATTR(powerreboot, 0220, NULL, xic_store);

static struct attribute * xic_attributes[] = {
	&dev_attr_version.attr,
	&dev_attr_uniqueid.attr,
	&dev_attr_wdogcount.attr,
	&dev_attr_wdogstatus.attr,
	&dev_attr_time.attr,
	&dev_attr_timeon.attr,
	&dev_attr_timeoff.attr,
	&dev_attr_poweroff.attr,
	&dev_attr_powerreboot.attr,
	NULL
};

static const struct attribute_group xic_attr_group = {
	.attrs = xic_attributes,
};

static int xic_probe(struct i2c_client * client, const struct i2c_device_id * id)
{
	uint8_t version = 0;

	if(!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
	{
		dev_err(&client->dev, "i2c bus does not support the xic\n");
		return -1;
	}

	if(!xic_read_version(client, &version) || (version != 0x10))
		return -1;
	g_client = client;

	printk("Probe the chip of xic, version = %02x\r\n", version);
	return sysfs_create_group(&client->dev.kobj, &xic_attr_group);
}

static int xic_remove(struct i2c_client * client)
{
	sysfs_remove_group(&client->dev.kobj, &xic_attr_group);
	return 0;
}

static const struct i2c_device_id xic_id_table[] = {
	{ "xic", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, xic_id_table);

#ifdef CONFIG_OF
static const struct of_device_id xic_of_match[] = {
	{ .compatible = "9tripod,xic", },
	{ }
};
MODULE_DEVICE_TABLE(of, xic_of_match);
#endif

static struct i2c_driver xic_driver = {
	.driver = {
		.name = "xic",
		.of_match_table = of_match_ptr(xic_of_match),
	},
	.probe = xic_probe,
	.remove = xic_remove,
	.id_table = xic_id_table,
};
module_i2c_driver(xic_driver);

MODULE_DESCRIPTION("9tripod xic driver");
MODULE_AUTHOR("Jianjun Jiang, 8192542@qq.com");
MODULE_LICENSE("GPL");
