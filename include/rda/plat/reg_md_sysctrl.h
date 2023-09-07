#ifndef _REG_MD_SYSCTRL_H_
#define _REG_MD_SYSCTRL_H_

#include <rda/mach/hardware.h>
#include <rda/mach/iomap.h>

// =============================================================================
//  MACROS
// =============================================================================

// ============================================================================
// CPU_ID_T
// -----------------------------------------------------------------------------
/// 
// =============================================================================
typedef enum
{
/// CPU IDs
    XCPU                                        = 0x00000000,
    BCPU                                        = 0x00000001
} CPU_ID_T;


// ============================================================================
// SYS_CLKS_T
// -----------------------------------------------------------------------------
/// 
// =============================================================================
typedef enum
{
/// System side System clocks
    SYS_XCPU                                    = 0x00000000,
    SYS_XCPU_INT                                = 0x00000001,
    SYS_PCLK_CONF                               = 0x00000002,
    SYS_PCLK_DATA                               = 0x00000003,
    SYS_AMBA                                    = 0x00000004,
    SYS_DMA                                     = 0x00000005,
    SYS_EBC                                     = 0x00000006,
    SYS_IFC_CH0                                 = 0x00000007,
    SYS_IFC_CH1                                 = 0x00000008,
    SYS_IFC_CH2                                 = 0x00000009,
    SYS_IFC_CH3                                 = 0x0000000A,
    SYS_IFC_DBG                                 = 0x0000000B,
    SYS_A2A                                     = 0x0000000C,
    SYS_AXI2AHB                                 = 0x0000000D,
    SYS_AHB2AXI                                 = 0x0000000E,
    SYS_EXT_AHB                                 = 0x0000000F,
    SYS_DEBUG_UART                              = 0x00000010,
    SYS_DBGHST                                  = 0x00000011,
/// System side divided clock (either divided by module or by sys_ctrl)
    SYSD_SCI1                                   = 0x00000012,
    SYSD_SCI2                                   = 0x00000013,
    SYSD_SCI3                                   = 0x00000014,
    SYSD_RF_SPI                                 = 0x00000015,
    SYSD_OSC                                    = 0x00000016,
/// the following don't have an auto enable
    SYS_GPIO                                    = 0x00000017,
    SYS_IRQ                                     = 0x00000018,
    SYS_TCU                                     = 0x00000019,
    SYS_TIMER                                   = 0x0000001A,
    SYS_COM_REGS                                = 0x0000001B,
    SYS_DP_AP                                   = 0x0000001C,
/// the following are sharing their enable
    SYS_SCI1                                    = 0x0000001D,
    SYS_SCI2                                    = 0x0000001E,
    SYS_SCI3                                    = 0x0000001F,
/// keep last
    SYS_NOGATE                                  = 0x00000020
} SYS_CLKS_T;

#define NB_SYS_CLK_XCPU                          (2)
#define NB_SYS_CLK_AEN                           (23)
#define NB_SYS_CLK_EN                            (29)
#define NB_SYS_CLK                               (33)

// ============================================================================
// PER_CLKS_T
// -----------------------------------------------------------------------------
/// 
// =============================================================================
typedef enum
{
/// System side divided clock (either divided by module or by sys_ctrl)
    PERD_SPI1                                   = 0x00000000,
/// System side divided clock (either divided by module or by sys_ctrl)
    PERD_SPI2                                   = 0x00000001,
    PER_SPY                                     = 0x00000002,
    PER_TEST                                    = 0x00000003
} PER_CLKS_T;

#define NB_PER_CLK_AEN                           (2)
#define NB_PER_CLK_EN                            (4)
#define NB_PER_CLK                               (4)

// ============================================================================
// BB_CLKS_T
// -----------------------------------------------------------------------------
/// 
// =============================================================================
typedef enum
{
/// Baseband side System clocks
    BB_BCPU                                     = 0x00000000,
    BB_BCPU_INT                                 = 0x00000001,
    BB_AMBA                                     = 0x00000002,
    BB_PCLK_CONF                                = 0x00000003,
    BB_PCLK_DATA                                = 0x00000004,
    BB_EXCOR                                    = 0x00000005,
    BB_IFC_CH2                                  = 0x00000006,
    BB_IFC_CH3                                  = 0x00000007,
    BB_SRAM                                     = 0x00000008,
    BB_A2A                                      = 0x00000009,
    BB_ITLV                                     = 0x0000000A,
    BB_VITERBI                                  = 0x0000000B,
    BB_CIPHER                                   = 0x0000000C,
    BB_RF_IF                                    = 0x0000000D,
    BB_COPRO                                    = 0x0000000E,
    BB_CP2_REG                                  = 0x0000000F,
    BB_XCOR                                     = 0x00000010,
    BB_EVITAC                                   = 0x00000011,
/// the following don't have an auto enable
    BB_IRQ                                      = 0x00000012,
    BB_COM_REGS                                 = 0x00000013,
    BB_CORDIC                                   = 0x00000014
} BB_CLKS_T;

#define NB_BB_CLK_AEN                            (18)
#define NB_BB_CLK_EN                             (21)
#define NB_BB_CLK                                (21)
/// Other clocks
/// clocks with auto enble
/// the debug host clock auto enable is not used in host mode, only in uart mode
#define OC_HOST_UART                             (0)

// ============================================================================
// OTHER_CLKS_T
// -----------------------------------------------------------------------------
/// 
// =============================================================================
typedef enum
{
    OC_DEBUG_UART                               = 0x00000000,
    OC_RF_RX                                    = 0x00000001,
    OC_RF_TX                                    = 0x00000002,
    OC_MEM_BRIDGE                               = 0x00000003,
/// the following don't have an auto enable
    OC_LPS                                      = 0x00000004,
    OC_GPIO                                     = 0x00000005,
    OC_CLK_OUT                                  = 0x00000006,
    OC_MEM_CLK_OUT                              = 0x00000007,
    OC_TCU                                      = 0x00000008
} OTHER_CLKS_T;

#define NB_OTHER_CLK_AEN                         (4)
#define NB_OTHER_CLK_EN                          (9)
#define NB_OTHER_CLK                             (9)

// ============================================================================
// RESETS_T
// -----------------------------------------------------------------------------
/// 
// =============================================================================
typedef enum
{
/// System side resets
    RST_XCPU                                    = 0x00000000,
    RST_SYS_IRQ                                 = 0x00000001,
    RST_SYS_A2A                                 = 0x00000002,
    RST_SYS_AHB2AXI                             = 0x00000003,
    RST_SYS_AXI2AHB                             = 0x00000004,
    RST_DMA                                     = 0x00000005,
    RST_TIMER                                   = 0x00000006,
    RST_TCU                                     = 0x00000007,
    RST_GPIO                                    = 0x00000008,
    RST_CFG                                     = 0x00000009,
    RST_SPI1                                    = 0x0000000A,
    RST_SPI2                                    = 0x0000000B,
    RST_RF_SPI                                  = 0x0000000C,
    RST_SCI1                                    = 0x0000000D,
    RST_SCI2                                    = 0x0000000E,
    RST_SCI3                                    = 0x0000000F,
    RST_SPY                                     = 0x00000010,
    RST_MEM_BRIDGE                              = 0x00000011,
    RST_EXT_AHB                                 = 0x00000012,
    RST_DP_AP                                   = 0x00000013,
    RST_COMREGS                                 = 0x00000014,
    RST_COMREGS_AP                              = 0x00000015,
    RST_AP_CLKEN                                = 0x00000016,
    RST_AP_RST                                  = 0x00000017,
    RST_MEM_CHK                                 = 0x00000018,
    RST_BCPU                                    = 0x00000019,
    RST_BB_IRQ                                  = 0x0000001A,
    RST_BB_A2A                                  = 0x0000001B,
    RST_BB_IFC                                  = 0x0000001C,
    RST_BB_SRAM                                 = 0x0000001D,
    RST_ITLV                                    = 0x0000001E,
    RST_VITERBI                                 = 0x0000001F,
    RST_CIPHER                                  = 0x00000020,
    RST_XCOR                                    = 0x00000021,
    RST_COPRO                                   = 0x00000022,
    RST_RF_IF                                   = 0x00000023,
    RST_EXCOR                                   = 0x00000024,
    RST_EVITAC                                  = 0x00000025,
    RST_CORDIC                                  = 0x00000026,
    RST_TCU_BB                                  = 0x00000027,
    RST_BB_FULL                                 = 0x00000028,
    RST_SYS_FULL                                = 0x00000029
} RESETS_T;

#define NB_SRST                                  (25)
/// Baseband side resets
#define BOUND_BRST_FIRST                         (25)
#define BOUND_BRST_AFTER                         (40)
/// The following reset does not have register
#define NR_RST_REG                               (41)
#define NB_RST                                   (42)
#define NB_BRST                                  (BOUND_BRST_AFTER-BOUND_BRST_FIRST)

// ============================================================================
// RESET_OTHERS_T
// -----------------------------------------------------------------------------
/// 
// =============================================================================
typedef enum
{
/// Reset Other : resync on corresponding clock other
    RSTO_DBG_HOST                               = 0x00000000,
    RSTO_RF_RX                                  = 0x00000001,
    RSTO_RF_TX                                  = 0x00000002,
    RSTO_MEM_BRIDGE                             = 0x00000003,
    RSTO_LPS                                    = 0x00000004,
    RSTO_GPIO                                   = 0x00000005,
    RSTO_WDTIMER                                = 0x00000006,
    RSTO_TCU                                    = 0x00000007
} RESET_OTHERS_T;

#define BOUND_RSTO_RF_FIRST                      (1)
#define BOUND_RSTO_RF_AFTER                      (3)
#define NB_RSTO                                  (8)
/// For REG_DBG protect lock/unlock value
#define SYS_CTRL_PROTECT_LOCK                    (0XA50000)
#define SYS_CTRL_PROTECT_UNLOCK                  (0XA50001)

// =============================================================================
//  TYPES
// =============================================================================

// ============================================================================
// SYS_CTRL_T
// -----------------------------------------------------------------------------
/// 
// =============================================================================
typedef volatile struct
{
    /// <strong>This register is used to Lock and Unlock the protected registers.</strong>
    REG32                          REG_DBG;                      //0x00000000
    /// This register is protected.
    REG32                          Sys_Rst_Set;                  //0x00000004
    REG32                          Sys_Rst_Clr;                  //0x00000008
    /// This register is protected.
    REG32                          BB_Rst_Set;                   //0x0000000C
    REG32                          BB_Rst_Clr;                   //0x00000010
    REG32                          Clk_Sys_Mode;                 //0x00000014
    REG32                          Clk_Sys_Enable;               //0x00000018
    /// This register is protected.
    REG32                          Clk_Sys_Disable;              //0x0000001C
    REG32                          Clk_Per_Mode;                 //0x00000020
    REG32                          Clk_Per_Enable;               //0x00000024
    /// This register is protected.
    REG32                          Clk_Per_Disable;              //0x00000028
    REG32                          Clk_BB_Mode;                  //0x0000002C
    REG32                          Clk_BB_Enable;                //0x00000030
    /// This register is protected.
    REG32                          Clk_BB_Disable;               //0x00000034
    REG32                          Clk_Other_Mode;               //0x00000038
    REG32                          Clk_Other_Enable;             //0x0000003C
    /// This register is protected.
    REG32                          Clk_Other_Disable;            //0x00000040
    /// Register protected by Write_Unlocked_H.
    REG32                          Pll_Ctrl;                     //0x00000044
    /// This register is protected.
    REG32                          Sel_Clock;                    //0x00000048
    REG32                          Cfg_Clk_Sys;                  //0x0000004C
    REG32                          Cfg_Clk_Mem_Bridge;           //0x00000050
    /// This register is protected.
    REG32                          Cfg_Clk_Out;                  //0x00000054
    REG32                          Cfg_Clk_Host_Uart;            //0x00000058
    REG32                          Cfg_Clk_Auxclk;               //0x0000005C
    /// This register is protected.
    REG32                          Cfg_AHB;                      //0x00000060
    /// This register is protected. Used to unsplit masters manualy.
    REG32                          Ctrl_AHB;                     //0x00000064
    REG32                          XCpu_Dbg_BKP;                 //0x00000068
    REG32                          XCpu_Dbg_Addr;                //0x0000006C
    REG32                          BCpu_Dbg_BKP;                 //0x00000070
    REG32                          BCpu_Dbg_Addr;                //0x00000074
    REG32                          Cfg_Cpus_Cache_Ram_Disable;   //0x00000078
    REG32                          Reset_Cause;                  //0x0000007C
    /// This register is protected.
    REG32                          WakeUp;                       //0x00000080
    REG32                          AP_Ctrl;                      //0x00000084
    /// This register is protected.
    REG32                          Ignore_Charger;               //0x00000088
    REG32 Reserved_0000008C[28];                //0x0000008C
    /// This register is reserved.
    REG32                          Cfg_Reserve;                  //0x000000FC
} HWP_SYS_CTRL_T;


//REG_DBG
#define SYS_CTRL_SCRATCH(n)         (((n)&0xFFFF)<<0)
#define SYS_CTRL_WRITE_UNLOCK_STATUS (1<<30)
#define SYS_CTRL_WRITE_UNLOCK       (1<<31)

//Sys_Rst_Set
#define SYS_CTRL_SET_RST_XCPU       (1<<0)
#define SYS_CTRL_SET_RST_SYS_IRQ    (1<<1)
#define SYS_CTRL_SET_RST_SYS_A2A    (1<<2)
#define SYS_CTRL_SET_RST_SYS_AHB2AXI (1<<3)
#define SYS_CTRL_SET_RST_SYS_AXI2AHB (1<<4)
#define SYS_CTRL_SET_RST_DMA        (1<<5)
#define SYS_CTRL_SET_RST_TIMER      (1<<6)
#define SYS_CTRL_SET_RST_TCU        (1<<7)
#define SYS_CTRL_SET_RST_GPIO       (1<<8)
#define SYS_CTRL_SET_RST_CFG        (1<<9)
#define SYS_CTRL_SET_RST_SPI1       (1<<10)
#define SYS_CTRL_SET_RST_SPI2       (1<<11)
#define SYS_CTRL_SET_RST_RF_SPI     (1<<12)
#define SYS_CTRL_SET_RST_SCI1       (1<<13)
#define SYS_CTRL_SET_RST_SCI2       (1<<14)
#define SYS_CTRL_SET_RST_SCI3       (1<<15)
#define SYS_CTRL_SET_RST_SPY        (1<<16)
#define SYS_CTRL_SET_RST_MEM_BRIDGE (1<<17)
#define SYS_CTRL_SET_RST_EXT_AHB    (1<<18)
#define SYS_CTRL_SET_RST_DP_AP      (1<<19)
#define SYS_CTRL_SET_RST_COMREGS    (1<<20)
#define SYS_CTRL_SET_RST_COMREGS_AP (1<<21)
#define SYS_CTRL_SET_RST_AP_CLKEN   (1<<22)
#define SYS_CTRL_SET_RST_AP_RST     (1<<23)
#define SYS_CTRL_SET_RST_MEM_CHK    (1<<24)
#define SYS_CTRL_SET_RST_OUT        (1<<30)
#define SYS_CTRL_SOFT_RST           (1<<31)
#define SYS_CTRL_SET_SYS_RST(n)     (((n)&0x1FFFFFF)<<0)
#define SYS_CTRL_SET_SYS_RST_MASK   (0x1FFFFFF<<0)
#define SYS_CTRL_SET_SYS_RST_SHIFT  (0)

//Sys_Rst_Clr
#define SYS_CTRL_CLR_RST_XCPU       (1<<0)
#define SYS_CTRL_CLR_RST_SYS_IRQ    (1<<1)
#define SYS_CTRL_CLR_RST_SYS_A2A    (1<<2)
#define SYS_CTRL_CLR_RST_SYS_AHB2AXI (1<<3)
#define SYS_CTRL_CLR_RST_SYS_AXI2AHB (1<<4)
#define SYS_CTRL_CLR_RST_DMA        (1<<5)
#define SYS_CTRL_CLR_RST_TIMER      (1<<6)
#define SYS_CTRL_CLR_RST_TCU        (1<<7)
#define SYS_CTRL_CLR_RST_GPIO       (1<<8)
#define SYS_CTRL_CLR_RST_CFG        (1<<9)
#define SYS_CTRL_CLR_RST_SPI1       (1<<10)
#define SYS_CTRL_CLR_RST_SPI2       (1<<11)
#define SYS_CTRL_CLR_RST_RF_SPI     (1<<12)
#define SYS_CTRL_CLR_RST_SCI1       (1<<13)
#define SYS_CTRL_CLR_RST_SCI2       (1<<14)
#define SYS_CTRL_CLR_RST_SCI3       (1<<15)
#define SYS_CTRL_CLR_RST_SPY        (1<<16)
#define SYS_CTRL_CLR_RST_MEM_BRIDGE (1<<17)
#define SYS_CTRL_CLR_RST_EXT_AHB    (1<<18)
#define SYS_CTRL_CLR_RST_DP_AP      (1<<19)
#define SYS_CTRL_CLR_RST_COMREGS    (1<<20)
#define SYS_CTRL_CLR_RST_COMREGS_AP (1<<21)
#define SYS_CTRL_CLR_RST_AP_CLKEN   (1<<22)
#define SYS_CTRL_CLR_RST_AP_RST     (1<<23)
#define SYS_CTRL_CLR_RST_MEM_CHK    (1<<24)
#define SYS_CTRL_CLR_RST_OUT        (1<<30)
#define SYS_CTRL_CLR_SYS_RST(n)     (((n)&0x1FFFFFF)<<0)
#define SYS_CTRL_CLR_SYS_RST_MASK   (0x1FFFFFF<<0)
#define SYS_CTRL_CLR_SYS_RST_SHIFT  (0)

//BB_Rst_Set
#define SYS_CTRL_SET_RST_BCPU       (1<<0)
#define SYS_CTRL_SET_RST_BB_IRQ     (1<<1)
#define SYS_CTRL_SET_RST_BB_A2A     (1<<2)
#define SYS_CTRL_SET_RST_BB_IFC     (1<<3)
#define SYS_CTRL_SET_RST_BB_SRAM    (1<<4)
#define SYS_CTRL_SET_RST_ITLV       (1<<5)
#define SYS_CTRL_SET_RST_VITERBI    (1<<6)
#define SYS_CTRL_SET_RST_CIPHER     (1<<7)
#define SYS_CTRL_SET_RST_XCOR       (1<<8)
#define SYS_CTRL_SET_RST_COPRO      (1<<9)
#define SYS_CTRL_SET_RST_RF_IF      (1<<10)
#define SYS_CTRL_SET_RST_EXCOR      (1<<11)
#define SYS_CTRL_SET_RST_EVITAC     (1<<12)
#define SYS_CTRL_SET_RST_CORDIC     (1<<13)
#define SYS_CTRL_SET_RST_TCU_BB     (1<<14)
#define SYS_CTRL_SET_RST_BB_FULL    (1<<31)
#define SYS_CTRL_SET_BB_RST(n)      (((n)&0x7FFF)<<0)
#define SYS_CTRL_SET_BB_RST_MASK    (0x7FFF<<0)
#define SYS_CTRL_SET_BB_RST_SHIFT   (0)

//BB_Rst_Clr
#define SYS_CTRL_CLR_RST_BCPU       (1<<0)
#define SYS_CTRL_CLR_RST_BB_IRQ     (1<<1)
#define SYS_CTRL_CLR_RST_BB_A2A     (1<<2)
#define SYS_CTRL_CLR_RST_BB_IFC     (1<<3)
#define SYS_CTRL_CLR_RST_BB_SRAM    (1<<4)
#define SYS_CTRL_CLR_RST_ITLV       (1<<5)
#define SYS_CTRL_CLR_RST_VITERBI    (1<<6)
#define SYS_CTRL_CLR_RST_CIPHER     (1<<7)
#define SYS_CTRL_CLR_RST_XCOR       (1<<8)
#define SYS_CTRL_CLR_RST_COPRO      (1<<9)
#define SYS_CTRL_CLR_RST_RF_IF      (1<<10)
#define SYS_CTRL_CLR_RST_EXCOR      (1<<11)
#define SYS_CTRL_CLR_RST_EVITAC     (1<<12)
#define SYS_CTRL_CLR_RST_CORDIC     (1<<13)
#define SYS_CTRL_CLR_RST_TCU_BB     (1<<14)
#define SYS_CTRL_CLR_RST_BB_FULL    (1<<31)
#define SYS_CTRL_CLR_BB_RST(n)      (((n)&0x7FFF)<<0)
#define SYS_CTRL_CLR_BB_RST_MASK    (0x7FFF<<0)
#define SYS_CTRL_CLR_BB_RST_SHIFT   (0)

//Clk_Sys_Mode
#define SYS_CTRL_MODE_SYS_XCPU      (1<<0)
#define SYS_CTRL_MODE_SYS_XCPU_INT_AUTOMATIC (0<<1)
#define SYS_CTRL_MODE_SYS_XCPU_INT_MANUAL (1<<1)
#define SYS_CTRL_MODE_SYS_PCLK_CONF_AUTOMATIC (0<<2)
#define SYS_CTRL_MODE_SYS_PCLK_CONF_MANUAL (1<<2)
#define SYS_CTRL_MODE_SYS_PCLK_DATA_AUTOMATIC (0<<3)
#define SYS_CTRL_MODE_SYS_PCLK_DATA_MANUAL (1<<3)
#define SYS_CTRL_MODE_SYS_AMBA_AUTOMATIC (0<<4)
#define SYS_CTRL_MODE_SYS_AMBA_MANUAL (1<<4)
#define SYS_CTRL_MODE_SYS_DMA_AUTOMATIC (0<<5)
#define SYS_CTRL_MODE_SYS_DMA_MANUAL (1<<5)
#define SYS_CTRL_MODE_SYS_EBC_AUTOMATIC (0<<6)
#define SYS_CTRL_MODE_SYS_EBC_MANUAL (1<<6)
#define SYS_CTRL_MODE_SYS_IFC_CH0_AUTOMATIC (0<<7)
#define SYS_CTRL_MODE_SYS_IFC_CH0_MANUAL (1<<7)
#define SYS_CTRL_MODE_SYS_IFC_CH1_AUTOMATIC (0<<8)
#define SYS_CTRL_MODE_SYS_IFC_CH1_MANUAL (1<<8)
#define SYS_CTRL_MODE_SYS_IFC_CH2_AUTOMATIC (0<<9)
#define SYS_CTRL_MODE_SYS_IFC_CH2_MANUAL (1<<9)
#define SYS_CTRL_MODE_SYS_IFC_CH3_AUTOMATIC (0<<10)
#define SYS_CTRL_MODE_SYS_IFC_CH3_MANUAL (1<<10)
#define SYS_CTRL_MODE_SYS_IFC_DBG_AUTOMATIC (0<<11)
#define SYS_CTRL_MODE_SYS_IFC_DBG_MANUAL (1<<11)
#define SYS_CTRL_MODE_SYS_A2A_AUTOMATIC (0<<12)
#define SYS_CTRL_MODE_SYS_A2A_MANUAL (1<<12)
#define SYS_CTRL_MODE_SYS_AXI2AHB_AUTOMATIC (0<<13)
#define SYS_CTRL_MODE_SYS_AXI2AHB_MANUAL (1<<13)
#define SYS_CTRL_MODE_SYS_AHB2AXI_AUTOMATIC (0<<14)
#define SYS_CTRL_MODE_SYS_AHB2AXI_MANUAL (1<<14)
#define SYS_CTRL_MODE_SYS_EXT_AHB_AUTOMATIC (0<<15)
#define SYS_CTRL_MODE_SYS_EXT_AHB_MANUAL (1<<15)
#define SYS_CTRL_MODE_SYS_DEBUG_UART_AUTOMATIC (0<<16)
#define SYS_CTRL_MODE_SYS_DEBUG_UART_MANUAL (1<<16)
#define SYS_CTRL_MODE_SYS_DBGHST_AUTOMATIC (0<<17)
#define SYS_CTRL_MODE_SYS_DBGHST_MANUAL (1<<17)
#define SYS_CTRL_MODE_SYSD_SCI1_AUTOMATIC (0<<18)
#define SYS_CTRL_MODE_SYSD_SCI1_MANUAL (1<<18)
#define SYS_CTRL_MODE_SYSD_SCI2_AUTOMATIC (0<<19)
#define SYS_CTRL_MODE_SYSD_SCI2_MANUAL (1<<19)
#define SYS_CTRL_MODE_SYSD_SCI3_AUTOMATIC (0<<20)
#define SYS_CTRL_MODE_SYSD_SCI3_MANUAL (1<<20)
#define SYS_CTRL_MODE_SYSD_RF_SPI_AUTOMATIC (0<<21)
#define SYS_CTRL_MODE_SYSD_RF_SPI_MANUAL (1<<21)
#define SYS_CTRL_MODE_SYSD_OSC_AUTOMATIC (0<<22)
#define SYS_CTRL_MODE_SYSD_OSC_MANUAL (1<<22)
#define SYS_CTRL_MODE_CLK_SYS(n)    (((n)&0x3FFFFF)<<1)
#define SYS_CTRL_MODE_CLK_SYS_MASK  (0x3FFFFF<<1)
#define SYS_CTRL_MODE_CLK_SYS_SHIFT (1)

//Clk_Sys_Enable
#define SYS_CTRL_ENABLE_SYS_XCPU    (1<<0)
#define SYS_CTRL_ENABLE_SYS_XCPU_INT (1<<1)
#define SYS_CTRL_ENABLE_SYS_PCLK_CONF (1<<2)
#define SYS_CTRL_ENABLE_SYS_PCLK_DATA (1<<3)
#define SYS_CTRL_ENABLE_SYS_AMBA    (1<<4)
#define SYS_CTRL_ENABLE_SYS_DMA     (1<<5)
#define SYS_CTRL_ENABLE_SYS_EBC     (1<<6)
#define SYS_CTRL_ENABLE_SYS_IFC_CH0 (1<<7)
#define SYS_CTRL_ENABLE_SYS_IFC_CH1 (1<<8)
#define SYS_CTRL_ENABLE_SYS_IFC_CH2 (1<<9)
#define SYS_CTRL_ENABLE_SYS_IFC_CH3 (1<<10)
#define SYS_CTRL_ENABLE_SYS_IFC_DBG (1<<11)
#define SYS_CTRL_ENABLE_SYS_A2A     (1<<12)
#define SYS_CTRL_ENABLE_SYS_AXI2AHB (1<<13)
#define SYS_CTRL_ENABLE_SYS_AHB2AXI (1<<14)
#define SYS_CTRL_ENABLE_SYS_EXT_AHB (1<<15)
#define SYS_CTRL_ENABLE_SYS_DEBUG_UART (1<<16)
#define SYS_CTRL_ENABLE_SYS_DBGHST  (1<<17)
#define SYS_CTRL_ENABLE_SYSD_SCI1   (1<<18)
#define SYS_CTRL_ENABLE_SYSD_SCI2   (1<<19)
#define SYS_CTRL_ENABLE_SYSD_SCI3   (1<<20)
#define SYS_CTRL_ENABLE_SYSD_RF_SPI (1<<21)
#define SYS_CTRL_ENABLE_SYSD_OSC    (1<<22)
#define SYS_CTRL_ENABLE_SYS_GPIO    (1<<23)
#define SYS_CTRL_ENABLE_SYS_IRQ     (1<<24)
#define SYS_CTRL_ENABLE_SYS_TCU     (1<<25)
#define SYS_CTRL_ENABLE_SYS_TIMER   (1<<26)
#define SYS_CTRL_ENABLE_SYS_COM_REGS (1<<27)
#define SYS_CTRL_ENABLE_SYS_DP_AP   (1<<28)
#define SYS_CTRL_ENABLE_CLK_SYS(n)  (((n)&0x1FFFFFFF)<<0)
#define SYS_CTRL_ENABLE_CLK_SYS_MASK (0x1FFFFFFF<<0)
#define SYS_CTRL_ENABLE_CLK_SYS_SHIFT (0)

//Clk_Sys_Disable
#define SYS_CTRL_DISABLE_SYS_XCPU   (1<<0)
#define SYS_CTRL_DISABLE_SYS_XCPU_INT (1<<1)
#define SYS_CTRL_DISABLE_SYS_PCLK_CONF (1<<2)
#define SYS_CTRL_DISABLE_SYS_PCLK_DATA (1<<3)
#define SYS_CTRL_DISABLE_SYS_AMBA   (1<<4)
#define SYS_CTRL_DISABLE_SYS_DMA    (1<<5)
#define SYS_CTRL_DISABLE_SYS_EBC    (1<<6)
#define SYS_CTRL_DISABLE_SYS_IFC_CH0 (1<<7)
#define SYS_CTRL_DISABLE_SYS_IFC_CH1 (1<<8)
#define SYS_CTRL_DISABLE_SYS_IFC_CH2 (1<<9)
#define SYS_CTRL_DISABLE_SYS_IFC_CH3 (1<<10)
#define SYS_CTRL_DISABLE_SYS_IFC_DBG (1<<11)
#define SYS_CTRL_DISABLE_SYS_A2A    (1<<12)
#define SYS_CTRL_DISABLE_SYS_AXI2AHB (1<<13)
#define SYS_CTRL_DISABLE_SYS_AHB2AXI (1<<14)
#define SYS_CTRL_DISABLE_SYS_EXT_AHB (1<<15)
#define SYS_CTRL_DISABLE_SYS_DEBUG_UART (1<<16)
#define SYS_CTRL_DISABLE_SYS_DBGHST (1<<17)
#define SYS_CTRL_DISABLE_SYSD_SCI1  (1<<18)
#define SYS_CTRL_DISABLE_SYSD_SCI2  (1<<19)
#define SYS_CTRL_DISABLE_SYSD_SCI3  (1<<20)
#define SYS_CTRL_DISABLE_SYSD_RF_SPI (1<<21)
#define SYS_CTRL_DISABLE_SYSD_OSC   (1<<22)
#define SYS_CTRL_DISABLE_SYS_GPIO   (1<<23)
#define SYS_CTRL_DISABLE_SYS_IRQ    (1<<24)
#define SYS_CTRL_DISABLE_SYS_TCU    (1<<25)
#define SYS_CTRL_DISABLE_SYS_TIMER  (1<<26)
#define SYS_CTRL_DISABLE_SYS_COM_REGS (1<<27)
#define SYS_CTRL_DISABLE_SYS_DP_AP  (1<<28)
#define SYS_CTRL_DISABLE_CLK_SYS(n) (((n)&0x1FFFFFFF)<<0)
#define SYS_CTRL_DISABLE_CLK_SYS_MASK (0x1FFFFFFF<<0)
#define SYS_CTRL_DISABLE_CLK_SYS_SHIFT (0)

//Clk_Per_Mode
#define SYS_CTRL_MODE_PERD_SPI1_AUTOMATIC (0<<0)
#define SYS_CTRL_MODE_PERD_SPI1_MANUAL (1<<0)
#define SYS_CTRL_MODE_PERD_SPI2_AUTOMATIC (0<<1)
#define SYS_CTRL_MODE_PERD_SPI2_MANUAL (1<<1)
#define SYS_CTRL_MODE_CLK_PER(n)    (((n)&3)<<0)
#define SYS_CTRL_MODE_CLK_PER_MASK  (3<<0)
#define SYS_CTRL_MODE_CLK_PER_SHIFT (0)

//Clk_Per_Enable
#define SYS_CTRL_ENABLE_PERD_SPI1   (1<<0)
#define SYS_CTRL_ENABLE_PERD_SPI2   (1<<1)
#define SYS_CTRL_ENABLE_PER_SPY     (1<<2)
#define SYS_CTRL_ENABLE_PER_TEST    (1<<3)
#define SYS_CTRL_ENABLE_CLK_PER(n)  (((n)&15)<<0)
#define SYS_CTRL_ENABLE_CLK_PER_MASK (15<<0)
#define SYS_CTRL_ENABLE_CLK_PER_SHIFT (0)

//Clk_Per_Disable
#define SYS_CTRL_DISABLE_PERD_SPI1  (1<<0)
#define SYS_CTRL_DISABLE_PERD_SPI2  (1<<1)
#define SYS_CTRL_DISABLE_PER_SPY    (1<<2)
#define SYS_CTRL_DISABLE_PER_TEST   (1<<3)
#define SYS_CTRL_DISABLE_CLK_PER(n) (((n)&15)<<0)
#define SYS_CTRL_DISABLE_CLK_PER_MASK (15<<0)
#define SYS_CTRL_DISABLE_CLK_PER_SHIFT (0)

//Clk_BB_Mode
#define SYS_CTRL_MODE_BB_BCPU       (1<<0)
#define SYS_CTRL_MODE_BB_BCPU_INT_AUTOMATIC (0<<1)
#define SYS_CTRL_MODE_BB_BCPU_INT_MANUAL (1<<1)
#define SYS_CTRL_MODE_BB_AMBA_AUTOMATIC (0<<2)
#define SYS_CTRL_MODE_BB_AMBA_MANUAL (1<<2)
#define SYS_CTRL_MODE_BB_PCLK_CONF_AUTOMATIC (0<<3)
#define SYS_CTRL_MODE_BB_PCLK_CONF_MANUAL (1<<3)
#define SYS_CTRL_MODE_BB_PCLK_DATA_AUTOMATIC (0<<4)
#define SYS_CTRL_MODE_BB_PCLK_DATA_MANUAL (1<<4)
#define SYS_CTRL_MODE_BB_EXCOR_AUTOMATIC (0<<5)
#define SYS_CTRL_MODE_BB_EXCOR_MANUAL (1<<5)
#define SYS_CTRL_MODE_BB_IFC_CH2_AUTOMATIC (0<<6)
#define SYS_CTRL_MODE_BB_IFC_CH2_MANUAL (1<<6)
#define SYS_CTRL_MODE_BB_IFC_CH3_AUTOMATIC (0<<7)
#define SYS_CTRL_MODE_BB_IFC_CH3_MANUAL (1<<7)
#define SYS_CTRL_MODE_BB_SRAM_AUTOMATIC (0<<8)
#define SYS_CTRL_MODE_BB_SRAM_MANUAL (1<<8)
#define SYS_CTRL_MODE_BB_A2A_AUTOMATIC (0<<9)
#define SYS_CTRL_MODE_BB_A2A_MANUAL (1<<9)
#define SYS_CTRL_MODE_BB_ITLV_AUTOMATIC (0<<10)
#define SYS_CTRL_MODE_BB_ITLV_MANUAL (1<<10)
#define SYS_CTRL_MODE_BB_VITERBI_AUTOMATIC (0<<11)
#define SYS_CTRL_MODE_BB_VITERBI_MANUAL (1<<11)
#define SYS_CTRL_MODE_BB_CIPHER_AUTOMATIC (0<<12)
#define SYS_CTRL_MODE_BB_CIPHER_MANUAL (1<<12)
#define SYS_CTRL_MODE_BB_RF_IF_AUTOMATIC (0<<13)
#define SYS_CTRL_MODE_BB_RF_IF_MANUAL (1<<13)
#define SYS_CTRL_MODE_BB_COPRO_AUTOMATIC (0<<14)
#define SYS_CTRL_MODE_BB_COPRO_MANUAL (1<<14)
#define SYS_CTRL_MODE_BB_CP2_REG_AUTOMATIC (0<<15)
#define SYS_CTRL_MODE_BB_CP2_REG_MANUAL (1<<15)
#define SYS_CTRL_MODE_BB_XCOR_AUTOMATIC (0<<16)
#define SYS_CTRL_MODE_BB_XCOR_MANUAL (1<<16)
#define SYS_CTRL_MODE_BB_EVITAC_AUTOMATIC (0<<17)
#define SYS_CTRL_MODE_BB_EVITAC_MANUAL (1<<17)
#define SYS_CTRL_MODE_CLK_BB(n)     (((n)&0x1FFFF)<<1)
#define SYS_CTRL_MODE_CLK_BB_MASK   (0x1FFFF<<1)
#define SYS_CTRL_MODE_CLK_BB_SHIFT  (1)

//Clk_BB_Enable
#define SYS_CTRL_ENABLE_BB_BCPU     (1<<0)
#define SYS_CTRL_ENABLE_BB_BCPU_INT (1<<1)
#define SYS_CTRL_ENABLE_BB_AMBA     (1<<2)
#define SYS_CTRL_ENABLE_BB_PCLK_CONF (1<<3)
#define SYS_CTRL_ENABLE_BB_PCLK_DATA (1<<4)
#define SYS_CTRL_ENABLE_BB_EXCOR    (1<<5)
#define SYS_CTRL_ENABLE_BB_IFC_CH2  (1<<6)
#define SYS_CTRL_ENABLE_BB_IFC_CH3  (1<<7)
#define SYS_CTRL_ENABLE_BB_SRAM     (1<<8)
#define SYS_CTRL_ENABLE_BB_A2A      (1<<9)
#define SYS_CTRL_ENABLE_BB_ITLV     (1<<10)
#define SYS_CTRL_ENABLE_BB_VITERBI  (1<<11)
#define SYS_CTRL_ENABLE_BB_CIPHER   (1<<12)
#define SYS_CTRL_ENABLE_BB_RF_IF    (1<<13)
#define SYS_CTRL_ENABLE_BB_COPRO    (1<<14)
#define SYS_CTRL_ENABLE_BB_CP2_REG  (1<<15)
#define SYS_CTRL_ENABLE_BB_XCOR     (1<<16)
#define SYS_CTRL_ENABLE_BB_EVITAC   (1<<17)
#define SYS_CTRL_ENABLE_BB_IRQ      (1<<18)
#define SYS_CTRL_ENABLE_BB_COM_REGS (1<<19)
#define SYS_CTRL_ENABLE_BB_CORDIC   (1<<20)
#define SYS_CTRL_ENABLE_CLK_BB(n)   (((n)&0x1FFFFF)<<0)
#define SYS_CTRL_ENABLE_CLK_BB_MASK (0x1FFFFF<<0)
#define SYS_CTRL_ENABLE_CLK_BB_SHIFT (0)

//Clk_BB_Disable
#define SYS_CTRL_DISABLE_BB_BCPU    (1<<0)
#define SYS_CTRL_DISABLE_BB_BCPU_INT (1<<1)
#define SYS_CTRL_DISABLE_BB_AMBA    (1<<2)
#define SYS_CTRL_DISABLE_BB_PCLK_CONF (1<<3)
#define SYS_CTRL_DISABLE_BB_PCLK_DATA (1<<4)
#define SYS_CTRL_DISABLE_BB_EXCOR   (1<<5)
#define SYS_CTRL_DISABLE_BB_IFC_CH2 (1<<6)
#define SYS_CTRL_DISABLE_BB_IFC_CH3 (1<<7)
#define SYS_CTRL_DISABLE_BB_SRAM    (1<<8)
#define SYS_CTRL_DISABLE_BB_A2A     (1<<9)
#define SYS_CTRL_DISABLE_BB_ITLV    (1<<10)
#define SYS_CTRL_DISABLE_BB_VITERBI (1<<11)
#define SYS_CTRL_DISABLE_BB_CIPHER  (1<<12)
#define SYS_CTRL_DISABLE_BB_RF_IF   (1<<13)
#define SYS_CTRL_DISABLE_BB_COPRO   (1<<14)
#define SYS_CTRL_DISABLE_BB_CP2_REG (1<<15)
#define SYS_CTRL_DISABLE_BB_XCOR    (1<<16)
#define SYS_CTRL_DISABLE_BB_EVITAC  (1<<17)
#define SYS_CTRL_DISABLE_BB_IRQ     (1<<18)
#define SYS_CTRL_DISABLE_BB_COM_REGS (1<<19)
#define SYS_CTRL_DISABLE_BB_CORDIC  (1<<20)
#define SYS_CTRL_DISABLE_CLK_BB(n)  (((n)&0x1FFFFF)<<0)
#define SYS_CTRL_DISABLE_CLK_BB_MASK (0x1FFFFF<<0)
#define SYS_CTRL_DISABLE_CLK_BB_SHIFT (0)

//Clk_Other_Mode
#define SYS_CTRL_MODE_OC_DEBUG_UART_AUTOMATIC (0<<0)
#define SYS_CTRL_MODE_OC_DEBUG_UART_MANUAL (1<<0)
#define SYS_CTRL_MODE_OC_RF_RX_AUTOMATIC (0<<1)
#define SYS_CTRL_MODE_OC_RF_RX_MANUAL (1<<1)
#define SYS_CTRL_MODE_OC_RF_TX_AUTOMATIC (0<<2)
#define SYS_CTRL_MODE_OC_RF_TX_MANUAL (1<<2)
#define SYS_CTRL_MODE_OC_MEM_BRIDGE_AUTOMATIC (0<<3)
#define SYS_CTRL_MODE_OC_MEM_BRIDGE_MANUAL (1<<3)
#define SYS_CTRL_MODE_CLK_OTHER(n)  (((n)&15)<<0)
#define SYS_CTRL_MODE_CLK_OTHER_MASK (15<<0)
#define SYS_CTRL_MODE_CLK_OTHER_SHIFT (0)

//Clk_Other_Enable
#define SYS_CTRL_ENABLE_OC_DEBUG_UART (1<<0)
#define SYS_CTRL_ENABLE_OC_RF_RX    (1<<1)
#define SYS_CTRL_ENABLE_OC_RF_TX    (1<<2)
#define SYS_CTRL_ENABLE_OC_MEM_BRIDGE (1<<3)
#define SYS_CTRL_ENABLE_OC_LPS      (1<<4)
#define SYS_CTRL_ENABLE_OC_GPIO     (1<<5)
#define SYS_CTRL_ENABLE_OC_CLK_OUT  (1<<6)
#define SYS_CTRL_ENABLE_OC_MEM_CLK_OUT (1<<7)
#define SYS_CTRL_ENABLE_OC_TCU      (1<<8)
#define SYS_CTRL_ENABLE_CLK_OTHER(n) (((n)&0x1FF)<<0)
#define SYS_CTRL_ENABLE_CLK_OTHER_MASK (0x1FF<<0)
#define SYS_CTRL_ENABLE_CLK_OTHER_SHIFT (0)

//Clk_Other_Disable
#define SYS_CTRL_DISABLE_OC_DEBUG_UART (1<<0)
#define SYS_CTRL_DISABLE_OC_RF_RX   (1<<1)
#define SYS_CTRL_DISABLE_OC_RF_TX   (1<<2)
#define SYS_CTRL_DISABLE_OC_MEM_BRIDGE (1<<3)
#define SYS_CTRL_DISABLE_OC_LPS     (1<<4)
#define SYS_CTRL_DISABLE_OC_GPIO    (1<<5)
#define SYS_CTRL_DISABLE_OC_CLK_OUT (1<<6)
#define SYS_CTRL_DISABLE_OC_MEM_CLK_OUT (1<<7)
#define SYS_CTRL_DISABLE_OC_TCU     (1<<8)
#define SYS_CTRL_DISABLE_CLK_OTHER(n) (((n)&0x1FF)<<0)
#define SYS_CTRL_DISABLE_CLK_OTHER_MASK (0x1FF<<0)
#define SYS_CTRL_DISABLE_CLK_OTHER_SHIFT (0)

//Pll_Ctrl
#define SYS_CTRL_PLL_ENABLE         (1<<0)
#define SYS_CTRL_PLL_ENABLE_MASK    (1<<0)
#define SYS_CTRL_PLL_ENABLE_SHIFT   (0)
#define SYS_CTRL_PLL_ENABLE_POWER_DOWN (0<<0)
#define SYS_CTRL_PLL_ENABLE_ENABLE  (1<<0)
#define SYS_CTRL_PLL_LOCK_RESET     (1<<4)
#define SYS_CTRL_PLL_LOCK_RESET_MASK (1<<4)
#define SYS_CTRL_PLL_LOCK_RESET_SHIFT (4)
#define SYS_CTRL_PLL_LOCK_RESET_RESET (0<<4)
#define SYS_CTRL_PLL_LOCK_RESET_NO_RESET (1<<4)
#define SYS_CTRL_PLL_BYPASS         (1<<8)
#define SYS_CTRL_PLL_BYPASS_MASK    (1<<8)
#define SYS_CTRL_PLL_BYPASS_SHIFT   (8)
#define SYS_CTRL_PLL_BYPASS_PASS    (0<<8)
#define SYS_CTRL_PLL_BYPASS_BYPASS  (1<<8)
#define SYS_CTRL_PLL_CLK_FAST_ENABLE (1<<12)
#define SYS_CTRL_PLL_CLK_FAST_ENABLE_MASK (1<<12)
#define SYS_CTRL_PLL_CLK_FAST_ENABLE_SHIFT (12)
#define SYS_CTRL_PLL_CLK_FAST_ENABLE_ENABLE (1<<12)
#define SYS_CTRL_PLL_CLK_FAST_ENABLE_DISABLE (0<<12)
#define SYS_CTRL_PLL_XP_CFG(n)      (((n)&4369)<<0)
#define SYS_CTRL_PLL_XP_CFG_MASK    (4369<<0)
#define SYS_CTRL_PLL_XP_CFG_SHIFT   (0)

//Sel_Clock
#define SYS_CTRL_SLOW_SEL_RF_OSCILLATOR (1<<0)
#define SYS_CTRL_SLOW_SEL_RF_RF     (0<<0)
#define SYS_CTRL_SYS_SEL_FAST_SLOW  (1<<4)
#define SYS_CTRL_SYS_SEL_FAST_FAST  (0<<4)
#define SYS_CTRL_TCU_13M_L_26M      (1<<5)
#define SYS_CTRL_TCU_13M_L_13M      (0<<5)
#define SYS_CTRL_PLL_DISABLE_LPS_DISABLE (1<<6)
#define SYS_CTRL_PLL_DISABLE_LPS_ENABLE (0<<6)
#define SYS_CTRL_DIGEN_H_ENABLE     (1<<7)
#define SYS_CTRL_DIGEN_H_DISABLE    (0<<7)
#define SYS_CTRL_RF_DETECTED_OK     (1<<20)
#define SYS_CTRL_RF_DETECTED_NO     (0<<20)
#define SYS_CTRL_RF_DETECT_BYPASS   (1<<21)
#define SYS_CTRL_RF_DETECT_RESET    (1<<22)
#define SYS_CTRL_RF_SELECTED_L      (1<<23)
#define SYS_CTRL_PLL_LOCKED         (1<<24)
#define SYS_CTRL_PLL_LOCKED_MASK    (1<<24)
#define SYS_CTRL_PLL_LOCKED_SHIFT   (24)
#define SYS_CTRL_PLL_LOCKED_LOCKED  (1<<24)
#define SYS_CTRL_PLL_LOCKED_NOT_LOCKED (0<<24)
#define SYS_CTRL_PLL_BYPASS_LOCK    (1<<27)
#define SYS_CTRL_FAST_SELECTED_L    (1<<31)
#define SYS_CTRL_FAST_SELECTED_L_MASK (1<<31)
#define SYS_CTRL_FAST_SELECTED_L_SHIFT (31)

//Cfg_Clk_Sys
#define SYS_CTRL_FREQ(n)            (((n)&15)<<0)
#define SYS_CTRL_FREQ_MASK          (15<<0)
#define SYS_CTRL_FREQ_SHIFT         (0)
#define SYS_CTRL_FREQ_312M          (13<<0)
#define SYS_CTRL_FREQ_250M          (12<<0)
#define SYS_CTRL_FREQ_208M          (11<<0)
#define SYS_CTRL_FREQ_178M          (10<<0)
#define SYS_CTRL_FREQ_156M          (9<<0)
#define SYS_CTRL_FREQ_139M          (8<<0)
#define SYS_CTRL_FREQ_125M          (7<<0)
#define SYS_CTRL_FREQ_113M          (6<<0)
#define SYS_CTRL_FREQ_104M          (5<<0)
#define SYS_CTRL_FREQ_89M           (4<<0)
#define SYS_CTRL_FREQ_78M           (3<<0)
#define SYS_CTRL_FREQ_52M           (2<<0)
#define SYS_CTRL_FREQ_39M           (1<<0)
#define SYS_CTRL_FREQ_26M           (0<<0)
#define SYS_CTRL_FORCE_DIV_UPDATE   (1<<4)
#define SYS_CTRL_REQ_DIV_UPDATE     (1<<8)

//Cfg_Clk_Mem_Bridge
#define SYS_CTRL_MEM_FREQ(n)        (((n)&15)<<0)
#define SYS_CTRL_MEM_FREQ_MASK      (15<<0)
#define SYS_CTRL_MEM_FREQ_SHIFT     (0)
#define SYS_CTRL_MEM_FREQ_312M      (13<<0)
#define SYS_CTRL_MEM_FREQ_250M      (12<<0)
#define SYS_CTRL_MEM_FREQ_208M      (11<<0)
#define SYS_CTRL_MEM_FREQ_178M      (10<<0)
#define SYS_CTRL_MEM_FREQ_156M      (9<<0)
#define SYS_CTRL_MEM_FREQ_139M      (8<<0)
#define SYS_CTRL_MEM_FREQ_125M      (7<<0)
#define SYS_CTRL_MEM_FREQ_113M      (6<<0)
#define SYS_CTRL_MEM_FREQ_104M      (5<<0)
#define SYS_CTRL_MEM_FREQ_89M       (4<<0)
#define SYS_CTRL_MEM_FREQ_78M       (3<<0)
#define SYS_CTRL_MEM_FREQ_52M       (2<<0)
#define SYS_CTRL_MEM_FREQ_39M       (1<<0)
#define SYS_CTRL_MEM_FREQ_26M       (0<<0)
#define SYS_CTRL_DDR_MODE_EN_NORMAL_MODE (0<<6)
#define SYS_CTRL_DDR_MODE_EN_DDR_MODE (1<<6)
#define SYS_CTRL_DDR_FAST_CLK_POL_INVERT (1<<7)
#define SYS_CTRL_DDR_FAST_CLK_POL_NORMAL (0<<7)
#define SYS_CTRL_DDR_DQSL_I(n)      (((n)&3)<<8)
#define SYS_CTRL_DDR_DQSU_I(n)      (((n)&3)<<10)
#define SYS_CTRL_DDR_DQSL_O(n)      (((n)&3)<<12)
#define SYS_CTRL_DDR_DQSU_O(n)      (((n)&3)<<14)
#define SYS_CTRL_DDR_PLL_OFF        (1<<16)
#define SYS_CTRL_MEM_REQ_DIV_UPDATE (1<<31)

//Cfg_Clk_Out
#define SYS_CTRL_CLKOUT_DIVIDER(n)  (((n)&31)<<0)
#define SYS_CTRL_CLKOUT_SEL_OSC     (0<<8)
#define SYS_CTRL_CLKOUT_SEL_RF      (1<<8)
#define SYS_CTRL_CLKOUT_SEL_DIVIDER (2<<8)

//Cfg_Clk_Host_Uart
#define SYS_CTRL_HOST_UART_DIVIDER(n) (((n)&0x3FF)<<0)
#define SYS_CTRL_HOST_UART_DIVIDER_MASK (0x3FF<<0)
#define SYS_CTRL_HOST_UART_DIVIDER_SHIFT (0)
#define SYS_CTRL_HOST_UART_SEL_PLL_SLOW (0<<12)
#define SYS_CTRL_HOST_UART_SEL_PLL_PLL (1<<12)

//Cfg_Clk_Auxclk
#define SYS_CTRL_AUXCLK_EN_DISABLE  (0<<0)
#define SYS_CTRL_AUXCLK_EN_ENABLE   (1<<0)

//Cfg_AHB
#define SYS_CTRL_SYS_NEW_ARBITRATION_ENABLE (1<<0)
#define SYS_CTRL_SYS_NEW_ARBITRATION_DISABLE (0<<0)
#define SYS_CTRL_ENABLE_SYS_MID_BIST_ENABLE (1<<1)
#define SYS_CTRL_ENABLE_SYS_MID_BIST_DISABLE (0<<1)
#define SYS_CTRL_ENABLE_SYS_MID_DMA_ENABLE (1<<2)
#define SYS_CTRL_ENABLE_SYS_MID_DMA_DISABLE (0<<2)
#define SYS_CTRL_ENABLE_SYS_MID_XCPU_ENABLE (1<<3)
#define SYS_CTRL_ENABLE_SYS_MID_XCPU_DISABLE (0<<3)
#define SYS_CTRL_ENABLE_SYS_MID_AHB2AHB_ENABLE (1<<4)
#define SYS_CTRL_ENABLE_SYS_MID_AHB2AHB_DISABLE (0<<4)
#define SYS_CTRL_ENABLE_SYS_MID_IFC_ENABLE (1<<5)
#define SYS_CTRL_ENABLE_SYS_MID_IFC_DISABLE (0<<5)
#define SYS_CTRL_ENABLE_SYS_MID_AXI2AHB_ENABLE (1<<6)
#define SYS_CTRL_ENABLE_SYS_MID_AXI2AHB_DISABLE (0<<6)
#define SYS_CTRL_BB_NEW_ARBITRATION_ENABLE (1<<16)
#define SYS_CTRL_BB_NEW_ARBITRATION_DISABLE (0<<16)
#define SYS_CTRL_ENABLE_BB_MID_BIST_ENABLE (1<<17)
#define SYS_CTRL_ENABLE_BB_MID_BIST_DISABLE (0<<17)
#define SYS_CTRL_ENABLE_BB_MID_IFC_ENABLE (1<<18)
#define SYS_CTRL_ENABLE_BB_MID_IFC_DISABLE (0<<18)
#define SYS_CTRL_ENABLE_BB_MID_BCPU_ENABLE (1<<19)
#define SYS_CTRL_ENABLE_BB_MID_BCPU_DISABLE (0<<19)
#define SYS_CTRL_ENABLE_BB_MID_AHB2AHB_ENABLE (1<<20)
#define SYS_CTRL_ENABLE_BB_MID_AHB2AHB_DISABLE (0<<20)
#define SYS_CTRL_SYS_ENABLE(n)      (((n)&0x3F)<<1)
#define SYS_CTRL_SYS_ENABLE_MASK    (0x3F<<1)
#define SYS_CTRL_SYS_ENABLE_SHIFT   (1)
#define SYS_CTRL_BB_ENABLE(n)       (((n)&15)<<17)
#define SYS_CTRL_BB_ENABLE_MASK     (15<<17)
#define SYS_CTRL_BB_ENABLE_SHIFT    (17)

//Ctrl_AHB
#define SYS_CTRL_SPLIT_SYS_MID_BIST_NORMAL (1<<0)
#define SYS_CTRL_SPLIT_SYS_MID_BIST_FORCE (0<<0)
#define SYS_CTRL_SPLIT_SYS_MID_DMA_NORMAL (1<<1)
#define SYS_CTRL_SPLIT_SYS_MID_DMA_FORCE (0<<1)
#define SYS_CTRL_SPLIT_SYS_MID_XCPU_NORMAL (1<<2)
#define SYS_CTRL_SPLIT_SYS_MID_XCPU_FORCE (0<<2)
#define SYS_CTRL_SPLIT_SYS_MID_AHB2AHB_NORMAL (1<<3)
#define SYS_CTRL_SPLIT_SYS_MID_AHB2AHB_FORCE (0<<3)
#define SYS_CTRL_SPLIT_SYS_MID_IFC_NORMAL (1<<4)
#define SYS_CTRL_SPLIT_SYS_MID_IFC_FORCE (0<<4)
#define SYS_CTRL_SPLIT_SYS_MID_AXI2AHB_NORMAL (1<<5)
#define SYS_CTRL_SPLIT_SYS_MID_AXI2AHB_FORCE (0<<5)
#define SYS_CTRL_SPLIT_BB_MID_BIST_NORMAL (1<<16)
#define SYS_CTRL_SPLIT_BB_MID_BIST_FORCE (0<<16)
#define SYS_CTRL_SPLIT_BB_MID_IFC_NORMAL (1<<17)
#define SYS_CTRL_SPLIT_BB_MID_IFC_FORCE (0<<17)
#define SYS_CTRL_SPLIT_BB_MID_BCPU_NORMAL (1<<18)
#define SYS_CTRL_SPLIT_BB_MID_BCPU_FORCE (0<<18)
#define SYS_CTRL_SPLIT_BB_MID_AHB2AHB_NORMAL (1<<19)
#define SYS_CTRL_SPLIT_BB_MID_AHB2AHB_FORCE (0<<19)
#define SYS_CTRL_SYS_FORCE_HSPLIT(n) (((n)&0x3F)<<0)
#define SYS_CTRL_SYS_FORCE_HSPLIT_MASK (0x3F<<0)
#define SYS_CTRL_SYS_FORCE_HSPLIT_SHIFT (0)
#define SYS_CTRL_BB_FORCE_HSPLIT(n) (((n)&15)<<16)
#define SYS_CTRL_BB_FORCE_HSPLIT_MASK (15<<16)
#define SYS_CTRL_BB_FORCE_HSPLIT_SHIFT (16)

//XCpu_Dbg_BKP
#define SYS_CTRL_BKPT_EN            (1<<0)
#define SYS_CTRL_BKPT_MODE(n)       (((n)&3)<<4)
#define SYS_CTRL_BKPT_MODE_I        (0<<4)
#define SYS_CTRL_BKPT_MODE_R        (1<<4)
#define SYS_CTRL_BKPT_MODE_W        (2<<4)
#define SYS_CTRL_BKPT_MODE_RW       (3<<4)
#define SYS_CTRL_STALLED            (1<<8)

//XCpu_Dbg_Addr
#define SYS_CTRL_BREAKPOINT_ADDRESS(n) (((n)&0x3FFFFFF)<<0)

//BCpu_Dbg_BKP
//#define SYS_CTRL_BKPT_EN          (1<<0)
//#define SYS_CTRL_BKPT_MODE(n)     (((n)&3)<<4)
//#define SYS_CTRL_BKPT_MODE_I      (0<<4)
//#define SYS_CTRL_BKPT_MODE_R      (1<<4)
//#define SYS_CTRL_BKPT_MODE_W      (2<<4)
//#define SYS_CTRL_BKPT_MODE_RW     (3<<4)
//#define SYS_CTRL_STALLED          (1<<8)

//BCpu_Dbg_Addr
//#define SYS_CTRL_BREAKPOINT_ADDRESS(n) (((n)&0x3FFFFFF)<<0)

//Cfg_Cpus_Cache_Ram_Disable
#define SYS_CTRL_XCPU_USE_MODE      (1<<0)
#define SYS_CTRL_XCPU_CLK_OFF_MODE  (1<<1)
#define SYS_CTRL_BCPU_USE_MODE      (1<<16)
#define SYS_CTRL_BCPU_CLK_OFF_MODE  (1<<17)
#define SYS_CTRL_XCPU_CACHE_RAM_DISABLE(n) (((n)&3)<<0)
#define SYS_CTRL_XCPU_CACHE_RAM_DISABLE_MASK (3<<0)
#define SYS_CTRL_XCPU_CACHE_RAM_DISABLE_SHIFT (0)
#define SYS_CTRL_BCPU_CACHE_RAM_DISABLE(n) (((n)&3)<<16)
#define SYS_CTRL_BCPU_CACHE_RAM_DISABLE_MASK (3<<16)
#define SYS_CTRL_BCPU_CACHE_RAM_DISABLE_SHIFT (16)

//Reset_Cause
#define SYS_CTRL_WATCHDOG_RESET_HAPPENED (1<<0)
#define SYS_CTRL_WATCHDOG_RESET_NO  (0<<0)
#define SYS_CTRL_APSOFT_RESET_HAPPENED (1<<1)
#define SYS_CTRL_APSOFT_RESET_NO    (0<<1)
#define SYS_CTRL_GLOBALSOFT_RESET_HAPPENED (1<<4)
#define SYS_CTRL_GLOBALSOFT_RESET_NO (0<<4)
#define SYS_CTRL_HOSTDEBUG_RESET_HAPPENED (1<<5)
#define SYS_CTRL_HOSTDEBUG_RESET_NO (0<<5)
#define SYS_CTRL_ALARMCAUSE_HAPPENED (1<<6)
#define SYS_CTRL_ALARMCAUSE_NO      (0<<6)
#define SYS_CTRL_MEMCHECKDONE_MASK  (1<<7)
#define SYS_CTRL_MEMCHECKDONE_SHIFT (7)
#define SYS_CTRL_MEMCHECKDONE_DONE  (1<<7)
#define SYS_CTRL_MEMCHECKDONE_RUNNING (0<<7)
#define SYS_CTRL_BOOT_MODE(n)       (((n)&0xFFFF)<<8)
#define SYS_CTRL_BOOT_MODE_MASK     (0xFFFF<<8)
#define SYS_CTRL_BOOT_MODE_SHIFT    (8)
#define SYS_CTRL_SW_BOOT_MODE(n)    (((n)&0x7F)<<24)
#define SYS_CTRL_SW_BOOT_MODE_MASK  (0x7F<<24)
#define SYS_CTRL_SW_BOOT_MODE_SHIFT (24)
#define SYS_CTRL_FONCTIONAL_TEST_MODE (1<<31)

//WakeUp
#define SYS_CTRL_FORCE_WAKEUP       (1<<0)

//AP_Ctrl
#define SYS_CTRL_AP_INT_STATUS      (1<<0)
#define SYS_CTRL_AP_INT_MASK        (1<<16)
#define SYS_CTRL_AP_DEEPSLEEP_EN    (1<<24)

//Ignore_Charger
#define SYS_CTRL_IGNORE_CHARGER     (1<<0)

//Cfg_Reserve
#define SYS_CTRL_RESERVE(n)         (((n)&0xFFFF)<<0)

#endif
