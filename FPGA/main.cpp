#include <math.h>

#include "xparameters.h"
#include "xgpio.h"
#include "xil_printf.h"

extern "C"
{
    #include "PmodNAV.h"
}

/*************************** Global Variables ******************************/

PmodNAV nav;
XGpio gpio;

/********************* Function Forward Declarations ***********************/

constexpr unsigned int ledChannel    = 1;
constexpr unsigned int buttonChannel = 2;

/***************************** Function Definitions ************************/

int main(void)
{
	unsigned char button;

    if (XGpio_Initialize(&gpio, XPAR_AXI_GPIO_0_DEVICE_ID) != XST_SUCCESS)
    	return -1;

    XGpio_SetDataDirection(&gpio, ledChannel, 0);
    XGpio_SetDataDirection(&gpio, buttonChannel, 1);

    NAV_begin (&nav, XPAR_PMODNAV_0_AXI_LITE_GPIO_BASEADDR, XPAR_PMODNAV_0_AXI_LITE_SPI_BASEADDR);
    NAV_InitAG(&nav, 1, NAV_ACL_MODE_INST_A);
    NAV_SetODR(&nav, NAV_ACL_MODE_INST_A, NAV_ACL_ODR_XL_119HZ);

    while(true)
    {
        button = static_cast<unsigned char>(XGpio_DiscreteRead(&gpio, buttonChannel));
        XGpio_DiscreteWrite(&gpio, ledChannel, button);

    	float x, y, z;
        NAV_ReadAccelG(&nav, &x, &y, &z);

        unsigned char data[10];

        data[0] = button | 0x80;

        long xInt = (static_cast<long>(fabs(x) * float(1 << 16))) & 0x0FFFFF | (x > 0 ? 0 : 0x100000);
        long yInt = (static_cast<long>(fabs(y) * float(1 << 16))) & 0x0FFFFF | (y > 0 ? 0 : 0x100000);
        long zInt = (static_cast<long>(fabs(z) * float(1 << 16))) & 0x0FFFFF | (z > 0 ? 0 : 0x100000);

        data[1] =  xInt        & 0x7F;
        data[2] = (xInt >>  7) & 0x7F;
        data[3] =  xInt >> 14;

        data[4] =  yInt        & 0x7F;
        data[5] = (yInt >>  7) & 0x7F;
        data[6] =  yInt >> 14;

        data[7] =  zInt        & 0x7F;
        data[8] = (zInt >>  7) & 0x7F;
        data[9] =  zInt >> 14;

        for (int i = 0; i < 10; i++)
            xil_printf("%c", data[i]);
    }
}
