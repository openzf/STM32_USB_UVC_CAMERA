#include "usbd_uvc_if.h"

uint16_t pic[160][120];

void Camera_On(void);
void Camera_Off(void);
uint8_t *Camera_GetFrame(uint32_t *pFrameLength);
void Camera_FreeFrame(uint8_t *frame);


USBD_UVC_CameraTypeDef USBD_UVC_Camera =
{
	Camera_On,
	Camera_Off,
	Camera_GetFrame,
	Camera_FreeFrame
};


void Camera_On(void)
{

}

void Camera_Off(void)
{

}

uint8_t reg,data,write,read;
void Camera_Loop(void)
{

}

uint32_t fps = 0;

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

void Camera_FreeFrame(uint8_t *frame)
{

}
