 /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
@file: am7xfb.c

@abstract: actions-mircro framebuffer main source file.

@notice: Copyright (c), 2010-2015 Actions-Mirco Communications, Inc.
 *
 *  This program is develop for Actions-Mirco Display Engine driver;
 *  include framebuffer,osd	
 *
 *
 *
 *  The initial developer of the original code is scopengl
 *
 *  scopengl@gmail.com
 *
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include <linux/module.h>
#include <linux/platform_device.h>

#include <linux/fb.h>

#include "am7xfb.h"


#define DE_SYS
#if 0
int de_set_input(struct am7xfb_inimage* in){return 0;}
int de_set_output(struct am7xfb_outimage* out){return 0;}
int de_enable(void){return 0;}
int de_disable(void){return 0;}
int de_set_display_mode(int mode){return 0;}
int de_set_framebuffer_addr(unsigned long addr){return 0;}


#endif 

#ifdef DE_SYS
#include "actions_regs.h"
#include "actions_io.h"
static ssize_t de_regs_show(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	int count  = 0;
	char* p = buf;
	for(count=0;count<64;count++,p+= 20)
		sprintf(p,"%8x:%8x\n",DE_Con+count*4,act_readl(DE_Con+count*4));
	return p-buf;	
}

static ssize_t de_ctl_show(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	return sprintf(buf,"%x\n",act_readl(DE_Con));
}

static ssize_t de_ctl_store(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf, size_t count)
{
	int i;

	if (count > 0 && sscanf(buf, "%d", &i) > 0)
		act_writel(i,DE_Con);

	return count;
}

static DEVICE_ATTR(de_regs,S_IRUGO, de_regs_show, NULL);
static DEVICE_ATTR(de_ctl,S_IRWXUGO,de_ctl_show, de_ctl_store);

static struct attribute *de_attrs[]={
	&dev_attr_de_regs.attr,
	&dev_attr_de_ctl.attr,
	NULL
};

static const struct attribute_group de_attr_group =
	{ .attrs = de_attrs };






#endif

static struct fb_fix_screeninfo __devinitdata am7xfb_fix = {
	.id		= AM7X_FBID,
	.type		= FB_TYPE_PACKED_PIXELS,
	.visual		= FB_VISUAL_PSEUDOCOLOR,
	.xpanstep	= 0,
	.ypanstep	= 0,
	.ywrapstep	= 0,
	.accel		= FB_ACCEL_NONE,
	.smem_start = __pa(0x80550000),
	.smem_len = 800*480*4,
};

static struct class *am7xfbclass;

static struct platform_device *am7xfb_device;

static int default_device = LCD;
static int default_format = 0;
module_param(default_device,int,0);
module_param(default_format,int,0);


static int mainfb_set_framebuffer_addr(struct fb_info *info,unsigned long arg)
{
	struct am7xfb_par *par = info->par;
	unsigned long addr ;

	copy_from_user((void*)&addr,(void*)arg,sizeof(unsigned long));
	info->fix.smem_start = par->in.framebuffer_addr = addr;
	de_set_framebuffer_addr(addr);
	printk("set fbaddr:%x\n",addr);
	return 0;
}

static int mainfb_set_input(struct fb_info* info,unsigned long arg)
{
	struct am7xfb_inimage in;

	copy_from_user((void*)&in,(void*)arg,sizeof(struct am7xfb_inimage));
	info->fix.smem_start = __pa(in.framebuffer_addr);
	switch(in.image_colorspace){
		case RGB_888:
		case YUV_444:	
		case ARGB_888:	
			info->var.bits_per_pixel = 32;
			break;
		case RGB_565:
			info->var.bits_per_pixel = 16;
			break;
		case YUV_422:
			break;
		default :
			break;	
	}
	info->fix.smem_len = 8*in.image_xres*in.image_yres* \
		info->var.bits_per_pixel/8;
	info->fix.line_length = in.image_xres*(info->var.bits_per_pixel/8);

	printk("framebuffer_addr*****:%x  info->fix.line_length:%d  \n",in.framebuffer_addr,info->fix.line_length);
	de_set_input(&in);
	de_enable();

	return 0;
}

static int mainfb_set_output(struct fb_info* info,unsigned long arg)
{
	struct am7xfb_outimage out;

	copy_from_user((void*)&out,(void*)arg,sizeof(struct am7xfb_outimage));
	de_set_output(&out);
	de_enable();

	return 0;	

}



static int set2_device(struct fb_info* info,unsigned long arg,int type)
{
	struct am7xfb_par * par = info->par;
	struct am7x_display_device* device = NULL;
	struct am7xfb_outimage* out = NULL;
	int format;
	
	copy_from_user((void*)&format,(void*)arg,sizeof(format));	
	hardware_close_display_device(par->device);
	config_device(type,format);

	destroy_display_device(par->device);
	device = init_device(device,type);
	out = &par->out;
	out->image_colorspace = device->colorspace;
	out->image_xres = device->x_res;
	out->image_yres = device->y_res;
	par->device = device;
	par->display_mode = cacl_display_mode(device);
	de_set_output(out);
	de_set_display_mode(par->display_mode);
	hardware_init_display_device(device);
			
	return 0;

}


static int mainfb_ioctl(struct fb_info *info, unsigned int cmd,unsigned long arg)
{
	int ret = 0;

	switch(cmd){
		case AM7XFBIO_SETADDR:
			ret = mainfb_set_framebuffer_addr(info,arg);
			break;
		case AM7XFBIO_GETADDR:
			copy_to_user((void*)arg, &((struct am7xfb_par*)(info->par))->in.framebuffer_addr,sizeof(unsigned long));
			break;
		case AM7XFBIO_SETINPUT:
			mainfb_set_input(info,arg);
			break;
		case AM7XFBIO_GETINPUT:
			copy_to_user((void*)arg,(void*)(&((struct am7xfb_par*)(info->par))->in),sizeof(struct am7xfb_inimage));
			break;
		case AM7XFBIO_SETOUPUT:
			mainfb_set_output(info,arg);
			break;
		case AM7XFBIO_GETOUPUT:
			copy_to_user((void*)arg,(void*)(&((struct am7xfb_par*)(info->par))->out),sizeof(struct am7xfb_outimage));
			break;
			break;
		case AM7XFBIO_SET2_HDMI:
			set2_device(info,arg,HDMI);			
			break;
		case AM7XFBIO_CTL_HDMI:
			
			break;
		case AM7XFBIO_SET2_LCD:
			set2_device(info,arg,LCD);
			break;
		case AM7XFBIO_CTL_LCD:
			break;
		default:
			printk("ioctl error\n");
			break;
	}

		
	return ret;
}

static int mainfb_set_par(struct fb_info *info)
{

	return 0;
}

/* set color register */
static int mainfb_setcolreg(unsigned regno, unsigned red, unsigned green,
			    unsigned blue, unsigned transp, struct fb_info *info)
{
	return 0;
}

/* set color registers in batch */
static int mainfb_setcmap(struct fb_cmap *cmap, struct fb_info *info)
{
	return 0;
}


static int osdfb_ioctl(struct fb_info *info, unsigned int cmd,
		unsigned long arg)
{
		
	return 0;
}

static int osdfb_set_par(struct fb_info *info)
{

	return 0;
}



static struct fb_ops am7x_mainfb_ops = {
	.fb_ioctl = mainfb_ioctl,
	.fb_set_par = mainfb_set_par,
	.fb_setcolreg = mainfb_setcolreg,
	.fb_setcmap = mainfb_setcmap,
};

static struct fb_ops am7x_osdfb_ops = {
	.fb_ioctl = osdfb_ioctl,
	.fb_set_par = osdfb_set_par,
};

static void init_default_main_fb_info(struct fb_info *pinfo)
{
	struct fb_var_screeninfo *var; 
	struct am7xfb_par* par ;
	
	pinfo->fbops = &am7x_mainfb_ops;
	pinfo->fix = am7xfb_fix;

	par = pinfo->par;
	var = &pinfo->var;
	var->xres = par->out.image_xres;
	var->yres = par->out.image_yres;
	var->xoffset = par->out.image_x_offset-1;
	var->yoffset = par->out.image_y_offset-1;  //
	var->xres_virtual = par->out.image_xres_virtual;
	var->yres_virtual = par->out.image_yres_virtual;


}

static void init_am7x_deafult_mainfb_par(struct am7xfb_par* par)
{
	struct am7x_display_device *device ;
	struct am7xfb_outimage *out;
	struct am7xfb_inimage *in;
	int defalut_device_type = default_device;	
	int format = default_format;
	

	config_device(defalut_device_type,format);
	device = init_device(par->device,defalut_device_type);
	if(device == NULL){
		printk(KERN_WARNING"no default display device\n");
		par->state = INACTIVE_STATE;
	}		
	else {
	
		par->state = ACTIVE_STATE;
		par->device = device;
		out = &par->out;
		in = &par->in;

		out->image_colorspace = device->colorspace;
		out->image_xres = device->x_res;
		out->image_yres = device->y_res;
		out->image_x_offset = 1;
		out->image_y_offset = 1;
		out->image_xres_virtual = out->image_xres;
		out->image_yres_virtual = out->image_yres;
		in->default_color_mode_enable = TRUE;
		in->background_default_color = 0xff0000;  //BLACK
//		in->image_xres =800;
//		in->image_yres = 480;
//		in->image_colorspace = RGB_888;
//		in->framebuffer_addr = 0x80550000;
		par->display_mode = cacl_display_mode(device);
	}	
}

static void init_am7x_default_osdfb_par(struct am7xosd_par* par)
{
		
}

static void init_default_osd_fb_info(struct fb_info *pinfo)
{	

	pinfo->fbops = &am7x_osdfb_ops;
	
	
}

static void init_am7x_hardware_mainfb(struct fb_info *pinfo)
{
	struct am7xfb_par* par = pinfo->par;

	de_set_input(&par->in);	
	de_set_output(&par->out);
	de_enable();
	de_set_display_mode(par->display_mode);
	
	hardware_init_display_device(par->device);
}

static void init_am7x_hardware_osd1(struct fb_info* pinfo)
{
	

}

static void init_am7x_hareware_osd2(struct fb_info* pinfo)
{
			
	
}

/**
**	Description:	Register 1 main framebuffer and 2 osd framebuffer
**/

static struct fb_info *am7xfb[MAX_FB_SUPPORTED];

static int am7xfb_remove(struct platform_device *pdev)
{
	struct fb_info **info_array = platform_get_drvdata(pdev);
	
	if(info_array){
		framebuffer_release(info_array[0]);
#ifdef	 ENABLE_OSDFB
		framebuffer_release(info_array[1]);
		framebuffer_release(info_array[2]);
#endif
	}	

	release_mem_region(pdev->resource[0].start,
			pdev->resource[0].end - pdev->resource[0].start +1);
	return 0;
}

static	int	am7xfb_probe(struct platform_device* pdev)
{
	struct 	fb_info **main_fb = &am7xfb[0];
	struct	am7xfb_par *defaultfb_par = NULL;
	
#ifdef ENABLE_OSDFB 
	struct	fb_info **osd_fb1 = &am7xfb[1];
	struct 	fb_info **osd_fb2 = &am7xfb[2];
	struct	am7xosd_par *defaultosd_par1 =NULL;
	struct	am7xosd_par *defaultosd_par2 = NULL;
#endif

	int ret = 0;
/*	if (pdev->num_resources != 1) {
		ret = -ENODEV;
		goto fail;
	}	

	if(pdev->resource[0].flags != IORESOURCE_MEM)
	{
		ret = -ENODEV;
		goto fail;
	}
*/


	*main_fb = framebuffer_alloc(sizeof(struct am7xfb_par),&pdev->dev);
	if (!(*main_fb)){
		ret = -ENOMEM;
		goto fail;
	}

#ifdef ENABLE_OSDFB 	
	*osd_fb1 = framebuffer_alloc(sizeof(struct am7xosd_par),&pdev->dev);
	if (!(*osd_fb1)){
		ret = -ENOMEM;
		goto fail;
	}

	*osd_fb2 = framebuffer_alloc(sizeof(struct am7xosd_par),&pdev->dev);
	if (!(*osd_fb2)){
		ret = -ENOMEM;
		goto fail;
	}	
#endif	
	
	platform_set_drvdata(pdev,am7xfb);
	
	defaultfb_par = kzalloc(sizeof(*defaultfb_par),GFP_KERNEL);
	if(defaultfb_par==NULL)
		goto fail;
	init_am7x_deafult_mainfb_par(defaultfb_par);
	(*main_fb)->par = defaultfb_par;


	init_default_main_fb_info(*main_fb);
	ret = register_framebuffer(*main_fb);
	if (ret < 0) {
		printk(KERN_ERR "Failed to register framebuffer device: %d\n",
			ret);
		goto fail;
	}

	init_am7x_hardware_mainfb(*main_fb);

#ifdef ENABLE_OSDFB	
	/***  init osd fb default value ***/
	
	init_am7x_default_osdfb_par(defaultosd_par1);
 	(*osd_fb1)->par = 	defaultosd_par1;
	init_default_osd_fb_info(*osd_fb1);
	
	init_am7x_default_osdfb_par(defaultosd_par2);
	(*osd_fb2)->par = defaultosd_par2;
	init_default_osd_fb_info(*osd_fb2);
	ret = register_framebuffer(*osd_fb2);
	if (ret < 0) {
		printk(KERN_ERR "Failed to register framebuffer device: %d\n",
			ret);
		goto fail;
	}
	init_am7x_hardware_osd1(*osd_fb1); 
	init_am7x_hareware_osd2(*osd_fb2);

#endif
#ifdef DE_SYS
	ret = sysfs_create_group(&pdev->dev.kobj,&de_attr_group);
	if(ret)
	{
		printk("fb register with sysfs error\n");
		goto fail;
	}
#endif	
	
	return 0;
fail:
	am7xfb_remove(pdev);
	return ret;
}



#ifdef CONFIG_PM
int am7xfb_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct fb_info *mainfb = ((struct fb_info**)platform_get_drvdata(pdev))[0];
	struct am7xfb_par *par = mainfb->par;
	struct am7x_display_device* device = par->device;

	
	hardware_close_display_device(device); //enable device
	de_disable(); //enable engine

	printk("am7xfb suspend\n");
	return 0;
}

int am7xfb_resume(struct platform_device* pdev)
{
	struct fb_info *mainfb = ((struct fb_info**)platform_get_drvdata(pdev))[0];
	struct am7xfb_par *par = mainfb->par;
	struct am7x_display_device* device = par->device;

	de_enable();  //enable engine 
	hardware_init_display_device(device); //enable device 

	printk("am7xfb resume\n");
	return 0;
}

#endif

static struct platform_driver am7xfb_driver = {
	.probe		= am7xfb_probe,
	.remove 	= am7xfb_remove,
#ifdef CONFIG_PM
	.suspend	= am7xfb_suspend,
	.resume 	= am7xfb_resume,
#endif
	.driver 	= {
		.name	= AM7XFB_DEVICENAME,
	},
};

static int __devinit	am7xfb_init(void)
{
	int ret ;
	ret = platform_driver_register(&am7xfb_driver);
	if (!ret) {
		am7xfb_device = platform_device_register_simple(AM7XFB_DEVICENAME, 0,
								NULL, 0);
		if (IS_ERR(am7xfb_device)) {
			platform_driver_unregister(&am7xfb_driver);
			ret = PTR_ERR(am7xfb_device);
			goto exit;
		}
	}
	am7xfbclass = class_create(THIS_MODULE,"am7xfb");
	
	
exit:	
	return ret;
}

static void __exit	am7xfb_exit(void)
{
	class_destroy(am7xfbclass);
	platform_driver_unregister(&am7xfb_driver); 
}

module_init(am7xfb_init);
module_exit(am7xfb_exit);
MODULE_LICENSE("GPL");


