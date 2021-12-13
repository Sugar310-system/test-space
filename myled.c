#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/delay.h>

MODULE_AUTHOR("usuisyun and Ryuichi Ueda");
MODULE_DESCRIPTION("driver for LED contorl");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.0.1");

static int i=24,i2=26,k;
static dev_t dev;
static struct cdev cdv;
static struct class *cls = NULL;
static volatile u32 *gpio_base = NULL;

static ssize_t led_write(struct file* filp, const char* buf, size_t count, loff_t* pos)
{
	char c;
	int led_chika_flag=0;
	if(copy_from_user(&c,buf,sizeof(char)))
		return -EFAULT;
	
	if(c == '0'){
		led_chika_flag=0;
		gpio_base[10] = 1 << i;
		gpio_base[10] = 1 << i2;
	}
	else if(c == '1'){
		led_chika_flag=0;
		gpio_base[7] = 1 << i;
		gpio_base[7] = 1 << i2;
	}
	else if(c == '2'){
		for(k=0;k<5;k++)
		{
			gpio_base[7] = 1 << i;
			gpio_base[7] = 1 << i2;
			__delay(10*1000*1000);
			gpio_base[10] = 1 << i;
			gpio_base[10] = 1 << i2;
			__delay(10*1000*1000);
		}
	}
	else if(c == '3'){
		for(k=0;k<5;k++)
		{
			gpio_base[10] = 1 << i;
			gpio_base[7] = 1 << i2;
			__delay(10*1000*1000);
			gpio_base[7] = 1 << i;
			gpio_base[10] = 1 << i2;
			__delay(10*1000*1000);
			gpio_base[10] = 1 << i;
		}
	}

	return 1;
}

static struct file_operations led_fops ={
	.owner = THIS_MODULE,
	.write = led_write
};

static int __init init_mod(void)
{
	int retval;

	gpio_base = ioremap_nocache(0xfe200000, 0xA0);
	const u32 led1 = i;
	const u32 index1 = led1/10;
	const u32 shift1 = (led1%10)*3;
	const u32 led2 = i2;
	const u32 index2 = led2/10;
	const u32 shift2 = (led2%10)*3;
	const u32 mask2 = ~(0x7 << shift1|0x7<<shift2);
	gpio_base[index1] = (gpio_base[index1] & mask2) | (0x1 << shift1)|(0x01<<shift2);
	retval = alloc_chrdev_region(&dev, 0, 1, "myled");
	if(retval < 0){
		printk(KERN_ERR "alloc_chrdev_region failed.\n");
		return retval;
	}
	printk(KERN_INFO "%s is loaded. major:%d\n",__FILE__,MAJOR(dev));

	cdev_init(&cdv, &led_fops);
	cdv.owner = THIS_MODULE;
	retval = cdev_add(&cdv, dev, 1);
	if(retval < 0){
		printk(KERN_ERR "cdev_add failed. major:%d, minor:%d",MAJOR(dev),MINOR(dev));
		return retval;
	}

	cls = class_create(THIS_MODULE,"myled");
	if(IS_ERR(cls)){
		printk(KERN_ERR "class_create failed.");
		return PTR_ERR(cls);
	}
	device_create(cls, NULL, dev, NULL,"myled%d",MINOR(dev));

	return 0;
}

static void __exit cleanup_mod(void)
{
	cdev_del(&cdv);
	device_destroy(cls, dev);
	class_destroy(cls);
	unregister_chrdev_region(dev, 1);
	printk(KERN_INFO "%s is unloaded. major:%d\n",__FILE__,MAJOR(dev));
}

module_init(init_mod);
module_exit(cleanup_mod);
