
#ifndef _DT_BINDINGS_RDA_CLK
#define _DT_BINDINGS_RDA_CLK


/* root clocks */

#define CLK_RDA_CPU			0
#define CLK_RDA_BUS			1
#define CLK_RDA_MEM			2
	
/* childs of BUS */

#define CLK_RDA_USB			3
#define CLK_RDA_AXI			4
#define CLK_RDA_AHB1		5
#define CLK_RDA_APB1		6
#define CLK_RDA_APB2		7
#define CLK_RDA_GCG			8
#define CLK_RDA_GPU			9
#define CLK_RDA_VPU			10
#define CLK_RDA_VOC			11

/* childs of APB2 */	

#define CLK_RDA_UART1		12
#define CLK_RDA_UART2		13
#define CLK_RDA_UART3		14
	
/* childs of AHB1 */

#define CLK_RDA_SPIFLASH	15

/* childs of GCG */

#define CLK_RDA_GOUDA		16
#define CLK_RDA_DPI			17
#define CLK_RDA_CAMERA		18


#define CLK_RDA_BCK			19
#define CLK_RDA_DSI			20
#define CLK_RDA_CSI			21
#define CLK_RDA_DEBUG		22
#define CLK_RDA_CLK_OUT		23
#define CLK_RDA_AUX_CLK		24
#define CLK_RDA_CLK_32K		25	
	
	
#endif /* _DT_BINDINGS_RDA_CLK */