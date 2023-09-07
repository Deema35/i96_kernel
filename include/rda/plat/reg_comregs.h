#ifndef _COMREGS_H_
#define _COMREGS_H_

#include "rda/mach/hardware.h"

// =============================================================================
//  MACROS
// =============================================================================
#define NB_INT                                   (16)
#define NB_INT_HALF                              (8)

// =============================================================================
//  TYPES
// =============================================================================

// ============================================================================
// COMREGS_T
// -----------------------------------------------------------------------------
///
// =============================================================================
#define REG_AP_COMREGS_BASE         0x00980000

typedef volatile struct
{
    REG32                          Snapshot;                     //0x00000000
    REG32                          Snapshot_Cfg;                 //0x00000004
    REG32                          Cause;                        //0x00000008
    REG32                          Mask_Set;                     //0x0000000C
    REG32                          Mask_Clr;                     //0x00000010
    /// If accesses to ItReg_Set and ItReg_Clr registers are done simultaneously
    /// from both CPUs and affecting the same bits, the priority is given to set
    /// a bit.
    /// Note: Read value is the SET value by itself.
    REG32                          ItReg_Set;                    //0x00000014
    /// If accesses to ItReg_Set and ItReg_Clr registers are done simultaneously
    /// from both CPUs and affecting the same bits, the priority is given to set
    /// a bit.
    /// Note: Read value is the IRQ value from the other side.
    REG32                          ItReg_Clr;                    //0x00000018
} HWP_COMREGS_T;


//Snapshot
#define COMREGS_SNAPSHOT(n)         (((n)&3)<<0)

//Snapshot_Cfg
#define COMREGS_SNAPSHOT_CFG_WRAP_2 (2<<0)
#define COMREGS_SNAPSHOT_CFG_WRAP_3 (3<<0)

//Cause
#define COMREGS_IRQ0_CAUSE(n)       (((n)&0xFF)<<0)
#define COMREGS_IRQ0_CAUSE_MASK     (0xFF<<0)
#define COMREGS_IRQ0_CAUSE_SHIFT    (0)
#define COMREGS_IRQ1_CAUSE(n)       (((n)&0xFF)<<8)
#define COMREGS_IRQ1_CAUSE_MASK     (0xFF<<8)
#define COMREGS_IRQ1_CAUSE_SHIFT    (8)

//Mask_Set
#define COMREGS_IRQ0_MASK_SET(n)    (((n)&0xFF)<<0)
#define COMREGS_IRQ0_MASK_SET_MASK  (0xFF<<0)
#define COMREGS_IRQ0_MASK_SET_SHIFT (0)
#define COMREGS_IRQ1_MASK_SET(n)    (((n)&0xFF)<<8)
#define COMREGS_IRQ1_MASK_SET_MASK  (0xFF<<8)
#define COMREGS_IRQ1_MASK_SET_SHIFT (8)

//Mask_Clr
#define COMREGS_IRQ0_MASK_CLR(n)    (((n)&0xFF)<<0)
#define COMREGS_IRQ0_MASK_CLR_MASK  (0xFF<<0)
#define COMREGS_IRQ0_MASK_CLR_SHIFT (0)
#define COMREGS_IRQ1_MASK_CLR(n)    (((n)&0xFF)<<8)
#define COMREGS_IRQ1_MASK_CLR_MASK  (0xFF<<8)
#define COMREGS_IRQ1_MASK_CLR_SHIFT (8)

//ItReg_Set
#define COMREGS_IRQ0_SET(n)         (((n)&0xFF)<<0)
#define COMREGS_IRQ0_SET_MASK       (0xFF<<0)
#define COMREGS_IRQ0_SET_SHIFT      (0)
#define COMREGS_IRQ1_SET(n)         (((n)&0xFF)<<8)
#define COMREGS_IRQ1_SET_MASK       (0xFF<<8)
#define COMREGS_IRQ1_SET_SHIFT      (8)
#define COMREGS_IRQ(n)              (((n)&0xFFFF)<<0)
#define COMREGS_IRQ_MASK            (0xFFFF<<0)
#define COMREGS_IRQ_SHIFT           (0)

//ItReg_Clr
#define COMREGS_IRQ0_CLR(n)         (((n)&0xFF)<<0)
#define COMREGS_IRQ0_CLR_MASK       (0xFF<<0)
#define COMREGS_IRQ0_CLR_SHIFT      (0)
#define COMREGS_IRQ1_CLR(n)         (((n)&0xFF)<<8)
#define COMREGS_IRQ1_CLR_MASK       (0xFF<<8)
#define COMREGS_IRQ1_CLR_SHIFT      (8)
//#define COMREGS_IRQ(n)            (((n)&0xFFFF)<<0)
//#define COMREGS_IRQ_MASK          (0xFFFF<<0)
//#define COMREGS_IRQ_SHIFT         (0)





#endif

