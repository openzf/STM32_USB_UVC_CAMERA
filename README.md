# STM32_USB_UVC_CAMERA
cubemx stm32h743iit6,  usb device uvc camera

开发环境: cubemx 6.0 +keil5 + stm32h743iit6 

基于CDC串口更改

功能: 因为没摄像头,内部模拟了一张图片   USB插入到电脑中, 打开摄像头会显示这张图片



![img](https://img-blog.csdnimg.cn/20210414235447106.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2EyMjY3NTQyODQ4,size_16,color_FFFFFF,t_70)

![img](https://img-blog.csdnimg.cn/2021041423550417.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2EyMjY3NTQyODQ4,size_16,color_FFFFFF,t_70)

## 1 如何添加摄像头数据

在usdb_uvc_if.c中 返回图片缓冲地址

```c
uint8_t *Camera_GetFrame(uint32_t *pFrameLength)
{
	pFrameLength[0] = 160*120*2;
	int ix = 0;
	for(int i = 0; i<160;i++)
	{
			for(int j = 0; j<20;j++)
			{
				pic[i][j] = 0xffff;
			}
	}
	return (uint8_t *)&pic;
}
```

