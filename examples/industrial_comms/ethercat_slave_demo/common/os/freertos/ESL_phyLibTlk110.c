/*!
 *  \file ESL_phyLibTlk110.c
 *
 *  \brief
 *  Application external PhyLib FreeRTOS.
 *
 *  \author
 *  KUNBUS GmbH
 *
 *  \date
 *  2021-05-18
 *
 *  \copyright
 *  Copyright (c) 2021, KUNBUS GmbH<br /><br />
 *  All rights reserved.<br />
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:<br />
 *  <ol>
 *  <li>Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.</li>
 *  <li>Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.</li>
 *  <li>Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.</li>
 *  </ol>
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <ESL_phyLibTlk110.h>

#define APPL_TLK110_MLED_100                        0u
#define APPL_TLK110_RXTX_ACT                        1u
#define APPL_TLK110_DIS_MLED                        2u

/*PHY LED Modes*/
#define APPL_TLK110_LED_CFG_MODE0                   0
#define APPL_TLK110_LED_CFG_MODE1                   1
#define APPL_TLK110_LED_CFG_MODE2                   2
#define APPL_TLK110_LED_CFG_MODE3                   3

/*PHY LED BLINK Rates*/
#define APPL_TLK110_LED_BLINK_500                   500
#define APPL_TLK110_LED_BLINK_200                   200
#define APPL_TLK110_LED_BLINK_100                   100
#define APPL_TLK110_LED_BLINK_50                    50

#define APPL_TLK110_FAST_LINKDOWN_SIGENERGY         1u
#define APPL_TLK110_FAST_LINKDOWN_LOWSNR            (1u<<1)
#define APPL_TLK110_FAST_LINKDOWN_MLT3ERR           (1u<<2)
#define APPL_TLK110_FAST_LINKDOWN_RXERR             (1u<<3)

/* TLK global */
#define TLKPHY_AUTOMDIX_ENABLE                      (1u<<15)
#define TLKPHY_PHYCR_REG                            (0x19)

// PHY TLK 110 register offsets
#define APPL_TLK110_BASIC_CTRL                      (0x00)
#define APPL_TLK110_CR1_REG                         (0x09)
#define APPL_TLK110_CR2_REG                         (0x0A)
#define APPL_TLK110_CR3_REG                         (0x0B)
#define APPL_TLK110_REGCR_REG                       (0x0D)
#define APPL_TLK110_ADDAR_REG                       (0x0E)

#define APPL_TLK110_VENDOR_STS_REG                  (0x0010u)   // offset to TLK110 PHY Status register
#define APPL_TLK110_VENDOR_SCR_REG                  (0x0011u)   // offset to TLK110 PHY Specific Control register
#define APPL_TLK110_VENDOR_MISR1_REG                (0x0012u)   // offset to TLK110 MII Interrupt Status register 1
#define APPL_TLK110_VENDOR_MISR2_REG                (0x0013u)   // offset to TLK110 MII Interrupt Status register 2
#define APPL_TLK110_VENDOR_FCSCR_REG                (0x0014u)   // offset to TLK110 False Carrier Sense Counter Register
#define APPL_TLK110_VENDOR_RECR_REG                 (0x0015u)   // offset to TLK110 Receive Error Count Register
#define APPL_TLK110_VENDOR_BISCR_REG                (0x0016u)   // offset to TLK110 BIST Control Register
#define APPL_TLK110_VENDOR_RBR_REG                  (0x0017u)   // offset to TLK110 RMII and Status Register
#define APPL_TLK110_VENDOR_LEDCR_REG                (0x0018u)   // offset to TLK110 LED Control Register
#define APPL_TLK110_VENDOR_PHYCR_REG                (0x0019u)   // offset to TLK110 PHY Control Register
#define APPL_TLK110_VENDOR_10BTSCR_REG              (0x001Au)   // offset to TLK110 10Base-T Status/Control Register
#define APPL_TLK110_VENDOR_BICSR1_REG               (0x001Bu)   // offset to TLK110 BIST Control and Status Register 1
#define APPL_TLK110_VENDOR_BICSR2_REG               (0x001Cu)   // offset to TLK110 BIST Control and Status Register 2
#define APPL_TLK110_VENDOR_RESERVED_REG             (0x001Du)   // offset to TLK110 RESERVEDMII Interrupt Status register 2
#define APPL_TLK110_VENDOR_CDCR_REG                 (0x001Eu)   // offset to TLK110 Cable Diagnostic Control Register
#define APPL_TLK110_VENDOR_PHYRCR_REG               (0x001Fu)   // offset to TLK110 PHY Reset Control Register
#define APPL_TLK110_VENDOR_MLEDCR_REG               (0x0025u)

/* PHY status definitions */
#define APPL_TLK110_STS_SPEED_MASK                  (0x0002u)
#define APPL_TLK110_STS_SPEED_SHIFT                 (0x01u)
#define APPL_TLK110_STS_SPEED_10_Mbps               (0x0001u)
#define APPL_TLK110_STS_SPEED_100_Mbps              (0x0000u)

#define APPL_TLK110_STS_DUPLEX_TYPE_MASK            (0x0004u)
#define APPL_TLK110_STS_DUPLEX_TYPE_SHIFT           (0x02u)
#define APPL_TLK110_STS_DUPLEX_TYPE_FULL            (0x0001u)
#define APPL_TLK110_STS_DUPLEX_TYPE_HALF	        (0x0000u)

#define APPL_TLK110_VENDOR_MISR1_RE_HF_EN           (0x0001u)
#define APPL_TLK110_VENDOR_MISR1_FC_HF_EN           (0x0002u)
#define APPL_TLK110_VENDOR_MISR1_AUTO_NEG_COMP_EN   (0x0004u)
#define APPL_TLK110_VENDOR_MISR1_DUPLEX_MODE_EN     (0x0008u)
#define APPL_TLK110_VENDOR_MISR1_SPEED_EN           (0x0010u)
#define APPL_TLK110_VENDOR_MISR1_LINK_STATUS_EN     (0x0020u)
#define APPL_TLK110_VENDOR_MISR1_RESERVED0          (0x0040u)
#define APPL_TLK110_VENDOR_MISR1_RESERVED1          (0x0080u)
#define APPL_TLK110_VENDOR_MISR1_RE_HF_INT          (0x0100u)
#define APPL_TLK110_VENDOR_MISR1_FC_HF_INT          (0x0200u)
#define APPL_TLK110_VENDOR_MISR1_AUTO_NEG_COMP_INT  (0x0400u)
#define APPL_TLK110_VENDOR_MISR1_DUPLEX_MODE_INT    (0x0800u)
#define APPL_TLK110_VENDOR_MISR1_SPEED_INT          (0x1000u)
#define APPL_TLK110_VENDOR_MISR1_LINK_STATUS_INT    (0x2000u)
#define APPL_TLK110_VENDOR_MISR1_RESERVED2          (0x4000u)
#define APPL_TLK110_VENDOR_MISR1_RESERVED3          (0x8000u)

#define APPL_TLK110_VENDOR_PHYRCR_SWRESET           (0x8000u)
#define APPL_TLK110_BASIC_CTRL_POWER_DOWN           (0x0800u)

/* Phy Flags */
#define APPL_TLK110_AUTOMDIX_ENABLE                 (1u<<15)
#define APPL_TLK110_EXT_FD_ENABLE                   (1u<<5)
#define APPL_TLK110_ADDRESS_ACCESS                  0x001F
#define APPL_TLK110_DATA_NORMAL_ACCESS              0x401F

#define APPL_TLK110_ODDNIBBLE_DET_ENABLE            (1u<<1)
#define APPL_TLK110_RXERROR_IDLE_ENABLE             (1u<<2)
#define APPL_TLK110_ENH_LEDLINK_ENABLE              (1u<<4)
#define APPL_TLK110_EXT_FD_ENABLE                   (1u<<5)

#define APPL_TLK110_FASTRXDV_DET_ENABLE             (1u<<1)
#define APPL_TLK110_SWSTRAP_CONFIG_DONE             (1u<<15)

/*! <!-- Description: -->
 *
 *  \brief
 *  Detect Phy Type and setup access structures accordingly
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  pPhyLibCtxt_p	Context of External PhyLib.
 *  \param[in]  phyIdx_p        Phy number (0,1)
 *  \param[in]  phyId_p         Phy ID read from hardware
 *  \param[in]  phyAddr_p       Phy address
 *  \param[in]  pPhyLibDesc_p   External PhyLib Hooks
 *  \return     0 on success and Phy detected, error code otherwise
 *
 *  <!-- Group: -->
 *
 *  \ingroup APPPHYLIB
 *
 * */
int16_t EC_SLV_APP_TLK110_phyLibDetect(void* pPhyLibCtxt_p, uint8_t phyIdx_p, uint32_t phyId_p
                                      ,uint8_t phyAddr_p, EC_API_SLV_SPhyDescriptor_t* pPhyLibDesc_p)
{
    int16_t     retVal  = -6; /* uknown phy */

    OSALUNREF_PARM(pPhyLibCtxt_p);
    OSALUNREF_PARM(phyIdx_p);      /* currently we do not care about instance Index */
    OSALUNREF_PARM(phyAddr_p);     /* we do not need to remember our address here */

    /* exact TLK110, uper word is TLK105 also */
    if (phyId_p == 0x2000A211)
    {
        OSAL_printf("TLK110 detected\r\n");
        pPhyLibDesc_p->softwareReset                = EC_SLV_APP_TLK110_softwareReset;
        pPhyLibDesc_p->softwareRestart              = NULL;
        pPhyLibDesc_p->enablePhyAutoMDIX            = EC_SLV_APP_TLK110_enablePhyAutoMDIX;
        pPhyLibDesc_p->setMiiMode                   = EC_SLV_APP_TLK110_setMIIMode;
        pPhyLibDesc_p->setPowerMode                 = EC_SLV_APP_TLK110_powerMode;
        pPhyLibDesc_p->getPowerMode                 = NULL;
        pPhyLibDesc_p->mLEDConfig                   = EC_SLV_APP_TLK110_mLEDConfig;
        pPhyLibDesc_p->extFDEnable                  = EC_SLV_APP_TLK110_extFDEnable;
        pPhyLibDesc_p->oDDNibbleDetEnable           = EC_SLV_APP_TLK110_oDDNibbleDetEnable;
        pPhyLibDesc_p->rxErrIdleEnable              = EC_SLV_APP_TLK110_rxErrIdleEnable;
        pPhyLibDesc_p->ledConfig                    = EC_SLV_APP_TLK110_ledConfig;
        pPhyLibDesc_p->ledBlinkConfig               = EC_SLV_APP_TLK110_ledBlinkConfig;
        pPhyLibDesc_p->fastLinkDownDetEnable        = EC_SLV_APP_TLK110_fastLinkDownDetEnable;
        pPhyLibDesc_p->fastRXDVDetEnable            = EC_SLV_APP_TLK110_fastRXDVDetEnable;
        pPhyLibDesc_p->swStrapConfigDone            = EC_SLV_APP_TLK110_swStrapConfigDone;
        pPhyLibDesc_p->setLinkConfig                = NULL;
        pPhyLibDesc_p->getAutonegotiation           = NULL;
        pPhyLibDesc_p->setMdixMode                  = NULL;
        pPhyLibDesc_p->getMdixMode                  = NULL;
        pPhyLibDesc_p->disable1GbAdver              = NULL;
        pPhyLibDesc_p->rgmiiLowLatencyEnable        = NULL;
        pPhyLibDesc_p->rgmiiTxHalfFullThreshold     = NULL;
        pPhyLibDesc_p->rgmiiRxHalfFullThreshold     = NULL;

        retVal = 0;
    }

    return retVal;
}

/*! <!-- Description: -->
 *
 *  \brief
 *  Issue PHY Reset by software (used if no Hard Reset is available)
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  pAppCtxt_p      application context
 *  \param[in]  pStackCtxt_p    stack context
 *  \param[in]  phyAddr_p       phy address
 *
 *  <!-- Group: -->
 *
 *  \ingroup APPPHYLIB
 *
 * */
void EC_SLV_APP_TLK110_softwareReset(void* pAppCtxt_p, void* pStackCtxt_p, uint32_t phyAddr_p)
{
    uint16_t    phyRegVal   = 0;

    OSALUNREF_PARM(pAppCtxt_p);
    OSALUNREF_PARM(phyAddr_p);

    EC_API_SLV_phyRegRead(pStackCtxt_p, APPL_TLK110_VENDOR_PHYRCR_REG, &phyRegVal);
    phyRegVal |= APPL_TLK110_VENDOR_PHYRCR_SWRESET;
    EC_API_SLV_phyRegWrite(pStackCtxt_p, APPL_TLK110_VENDOR_PHYRCR_REG, phyRegVal);

    do
    {
        phyRegVal = 0;
        EC_API_SLV_phyRegRead(pStackCtxt_p, APPL_TLK110_VENDOR_PHYRCR_REG, &phyRegVal);
    }
    while (phyRegVal & APPL_TLK110_VENDOR_PHYRCR_SWRESET);
}

/*! <!-- Description: -->
 *
 *  \brief
 *  Enable Auto MDIX
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  pAppCtxt_p         application context
 *  \param[in]  pStackCtxt_p       stack context
 *  \param[in]  phyAddr_p         phy address
 *
 *  <!-- Group: -->
 *
 *  \ingroup APPPHYLIB
 *
 * */
void EC_SLV_APP_TLK110_enablePhyAutoMDIX(void* pAppCtxt_p, void* pStackCtxt_p, uint32_t phyAddr_p)
{
    uint16_t    phyRegVal   = 0;

    OSALUNREF_PARM(pAppCtxt_p);
    OSALUNREF_PARM(phyAddr_p);

    EC_API_SLV_phyRegRead(pStackCtxt_p, TLKPHY_PHYCR_REG, &phyRegVal);
    phyRegVal |= TLKPHY_AUTOMDIX_ENABLE;
    EC_API_SLV_phyRegWrite(pStackCtxt_p, TLKPHY_PHYCR_REG, phyRegVal);
}

/*! <!-- Description: -->
 *
 *  \brief
 *  Set MII Mode
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  pAppCtxt_p         application context
 *  \param[in]  pStackCtxt_p       stack context
 *  \param[in]  phyAddr_p         phy address
 *
 *  <!-- Group: -->
 *
 *  \ingroup APPPHYLIB
 *
 * */
void EC_SLV_APP_TLK110_setMIIMode(void *pAppCtxt_p, void *pStackCtxt_p, uint32_t phyAddr_p)
{
    uint16_t phyRegVal = 0;

    OSALUNREF_PARM(pAppCtxt_p);
    OSALUNREF_PARM(phyAddr_p);

    EC_API_SLV_phyRegWrite(pStackCtxt_p, APPL_TLK110_VENDOR_RBR_REG, phyRegVal);
}

/*! <!-- Description: -->
 *
 *  \brief
 *  Configure PhyMLED to detect RxLink by MLED (e.g. TLK)
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  pAppCtxt_p         application context
 *  \param[in]  pStackCtxt_p       stack context
 *  \param[in]  phyAddr_p         phy address
 *phyAddr_p
 *
 *  <!-- Group: -->
 *
 *  \ingroup APPPHYLIB
 *
 * */
void EC_SLV_APP_TLK110_mLEDConfig(void *pAppCtxt_p, void *pStackCtxt_p, uint32_t phyAddr_p)
{
    uint16_t phyRegVal      = 0;
    uint16_t mode           = APPL_TLK110_MLED_100;

    OSALUNREF_PARM(pAppCtxt_p);
    OSALUNREF_PARM(phyAddr_p);

    OSAL_printf("TLK110 set MLED\r\n");

    EC_API_SLV_phyRegWrite(pStackCtxt_p, APPL_TLK110_REGCR_REG, APPL_TLK110_ADDRESS_ACCESS);
    EC_API_SLV_phyRegWrite(pStackCtxt_p, APPL_TLK110_ADDAR_REG, APPL_TLK110_VENDOR_MLEDCR_REG);
    EC_API_SLV_phyRegWrite(pStackCtxt_p, APPL_TLK110_REGCR_REG, APPL_TLK110_DATA_NORMAL_ACCESS);

    EC_API_SLV_phyRegRead(pStackCtxt_p, APPL_TLK110_ADDAR_REG, &phyRegVal);

    switch (mode)
    {
    case APPL_TLK110_MLED_100:
        /*Bit10: MLED Pin29 Route and Enable (COL disable)
        Bit6:3: 0101 : High for 100 base TX
        Bit 0:1 : 0 MLED Pin17 routing disable (LINK_LED) */
        phyRegVal |= ((1 << 10) | (5 << 3));
        phyRegVal &= ~(3);
        break;

    case APPL_TLK110_RXTX_ACT:
        /*Bit10: MLED Pin29 Route and Enable (COL disable)
        Bit9: MLED polarity - important to keep it ACTIVE HIGH(PD) due to IDK HW errata
        Bit6:3: 0001 :RX/TX activity
        Bit 0:1 : 0 MLED Pin17 routing disable (LINK_LED) */
        phyRegVal |= ((1 << 10) | (1 << 3));
        phyRegVal &= ~((1 << 9) | 3);/*Bit 9 clear,LED on */
        break;

    case APPL_TLK110_DIS_MLED:
        /*Bit10: MLED Pin29 Route and Disable (COL enable)
        Bit9: MLED polarity - important to keep it ACTIVE HIGH(PD) due to IDK HW errata
        Bit6:3: XXXX : Don;t care
        Bit 0:1 : 0 MLED Pin17 routing disable (LINK_LED) */
        phyRegVal &= ~((1 << 10) | (1 << 9) | 3); /*Bit 9 clear,LED on */
        break;
    }

    EC_API_SLV_phyRegWrite(pStackCtxt_p, APPL_TLK110_ADDAR_REG, phyRegVal);
}

/*! <!-- Description: -->
 *
 *  \brief
 *  Set Ext Full Duplex enable
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  pAppCtxt_p         application context
 *  \param[in]  pStackCtxt_p       stack context
 *  \param[in]  phyAddr_p         phy address
 *
 *  <!-- Group: -->
 *
 *  \ingroup APPPHYLIB
 *
 * */
void EC_SLV_APP_TLK110_extFDEnable(void *pAppCtxt_p, void *pStackCtxt_p, uint32_t phyAddr_p)
{
    uint16_t    phyRegVal   = 0;

    OSALUNREF_PARM(pAppCtxt_p);
    OSALUNREF_PARM(phyAddr_p);

    EC_API_SLV_phyRegRead(pStackCtxt_p, APPL_TLK110_CR2_REG, &phyRegVal);
    phyRegVal |= APPL_TLK110_EXT_FD_ENABLE;
    EC_API_SLV_phyRegWrite(pStackCtxt_p, APPL_TLK110_CR2_REG, phyRegVal);
}

/*! <!-- Description: -->
 *
 *  \brief
 *  Set ODD Nibble detection enable
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  pAppCtxt_p         application context
 *  \param[in]  pStackCtxt_p       stack context
 *  \param[in]  phyAddr_p         phy address
 *
 *  <!-- Group: -->
 *
 *  \ingroup APPPHYLIB
 *
 * */
void EC_SLV_APP_TLK110_oDDNibbleDetEnable(void *pAppCtxt_p, void *pStackCtxt_p, uint32_t phyAddr_p)
{
    uint16_t    phyRegVal   = 0;

    OSALUNREF_PARM(pAppCtxt_p);
    OSALUNREF_PARM(phyAddr_p);

    EC_API_SLV_phyRegRead(pStackCtxt_p, APPL_TLK110_CR2_REG, &phyRegVal);
    phyRegVal |= APPL_TLK110_ODDNIBBLE_DET_ENABLE;
    EC_API_SLV_phyRegWrite(pStackCtxt_p, APPL_TLK110_CR2_REG, phyRegVal);
}

/*! <!-- Description: -->
 *
 *  \brief
 *  Set Rx Error Idle enable
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  pAppCtxt_p         application context
 *  \param[in]  pStackCtxt_p       stack context
 *  \param[in]  phyAddr_p         phy address
 *
 *  <!-- Group: -->
 *
 *  \ingroup APPPHYLIB
 *
 * */
void EC_SLV_APP_TLK110_rxErrIdleEnable(void *pAppCtxt_p, void *pStackCtxt_p, uint32_t phyAddr_p)
{
    uint16_t phyRegVal = 0;

    OSALUNREF_PARM(pAppCtxt_p);
    OSALUNREF_PARM(phyAddr_p);

    EC_API_SLV_phyRegRead(pStackCtxt_p, APPL_TLK110_CR2_REG, &phyRegVal);
    phyRegVal |= APPL_TLK110_RXERROR_IDLE_ENABLE;
    EC_API_SLV_phyRegWrite(pStackCtxt_p, APPL_TLK110_CR2_REG, phyRegVal);
}

/*! <!-- Description: -->
 *
 *  \brief
 *  Configure PHY LEDs
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  pAppCtxt_p         application context
 *  \param[in]  pStackCtxt_p       stack context
 *  \param[in]  phyAddr_p         phy address
 *
 *  <!-- Group: -->
 *
 *  \ingroup APPPHYLIB
 *
 * */
void EC_SLV_APP_TLK110_ledConfig(void *pAppCtxt_p, void *pStackCtxt_p, uint32_t phyAddr_p)
{
    uint16_t phyRegVal      = 0;
    uint16_t mode           = APPL_TLK110_LED_CFG_MODE2;

    OSALUNREF_PARM(pAppCtxt_p);
    OSALUNREF_PARM(phyAddr_p);

    EC_API_SLV_phyRegRead(pStackCtxt_p, APPL_TLK110_VENDOR_PHYCR_REG, &phyRegVal);

    switch (mode)
    {
    case APPL_TLK110_LED_CFG_MODE1:
        phyRegVal |= (1 << 5);   /*Set LED_CFG[0] to 1*/
        break;

    case APPL_TLK110_LED_CFG_MODE2:
        phyRegVal &= ~(1 << 5);   /*Set LED_CFG[0] to 0*/
        break;
    }

    EC_API_SLV_phyRegWrite(pStackCtxt_p, APPL_TLK110_VENDOR_PHYCR_REG, phyRegVal);
}

/*! <!-- Description: -->
 *
 *  \brief
 *  Configure PHY Blink LED mode
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  pAppCtxt_p         application context
 *  \param[in]  pStackCtxt_p       stack context
 *  \param[in]  phyAddr_p         phy address
 *
 *  <!-- Group: -->
 *
 *  \ingroup APPPHYLIB
 *
 * */
void EC_SLV_APP_TLK110_ledBlinkConfig(void *pAppCtxt_p, void *pStackCtxt_p, uint32_t phyAddr_p)
{
    uint16_t phyRegVal      = 0;
    uint16_t value          = APPL_TLK110_LED_BLINK_100;

    OSALUNREF_PARM(pAppCtxt_p);
    OSALUNREF_PARM(phyAddr_p);

    EC_API_SLV_phyRegRead(pStackCtxt_p, APPL_TLK110_VENDOR_LEDCR_REG, &phyRegVal);

    switch (value)
    {
    case APPL_TLK110_LED_BLINK_500:
        phyRegVal &= 0xF9FF;
        phyRegVal |= 0x0600;
        break;

    case APPL_TLK110_LED_BLINK_200:
        phyRegVal &= 0xF9FF;
        phyRegVal |= 0x0400;
        break;

    case APPL_TLK110_LED_BLINK_100:
        phyRegVal &= 0xF9FF;
        phyRegVal |= 0x0200;
        break;

    case APPL_TLK110_LED_BLINK_50:
        phyRegVal &= 0xF9FF;
        break;
    }

    EC_API_SLV_phyRegWrite(pStackCtxt_p, APPL_TLK110_VENDOR_LEDCR_REG, phyRegVal);
}

/*! <!-- Description: -->
 *
 *  \brief
 *  Set fast link down Detection enable
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  pAppCtxt_p         application context
 *  \param[in]  pStackCtxt_p       stack context
 *  \param[in]  phyAddr_p         phy address
 *
 *  <!-- Group: -->
 *
 *  \ingroup APPPHYLIB
 *
 * */
void EC_SLV_APP_TLK110_fastLinkDownDetEnable(void *pAppCtxt_p, void *pStackCtxt_p, uint32_t phyAddr_p)
{
    uint16_t phyRegVal      = 0;
    uint16_t value          = APPL_TLK110_FAST_LINKDOWN_SIGENERGY | APPL_TLK110_FAST_LINKDOWN_RXERR;

    OSALUNREF_PARM(pAppCtxt_p);
    OSALUNREF_PARM(phyAddr_p);

    EC_API_SLV_phyRegRead(pStackCtxt_p, APPL_TLK110_CR3_REG, &phyRegVal);
    phyRegVal |= value;
    EC_API_SLV_phyRegWrite(pStackCtxt_p, APPL_TLK110_CR3_REG, phyRegVal);
}

/*! <!-- Description: -->
 *
 *  \brief
 *  Set Fast RX DV detection enable
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  pAppCtxt_p         application context
 *  \param[in]  pStackCtxt_p       stack context
 *  \param[in]  phyAddr_p         phy address
 *
 *  <!-- Group: -->
 *
 *  \ingroup APPPHYLIB
 *
 * */
void EC_SLV_APP_TLK110_fastRXDVDetEnable(void *pAppCtxt_p, void *pStackCtxt_p, uint32_t phyAddr_p)
{
    uint16_t    phyRegVal   = 0;

    OSALUNREF_PARM(pAppCtxt_p);
    OSALUNREF_PARM(phyAddr_p);

    EC_API_SLV_phyRegRead(pStackCtxt_p, APPL_TLK110_CR1_REG, &phyRegVal);
    phyRegVal |= APPL_TLK110_FASTRXDV_DET_ENABLE;
    EC_API_SLV_phyRegWrite(pStackCtxt_p, APPL_TLK110_CR1_REG, phyRegVal);
}

/*! <!-- Description: -->
 *
 *  \brief
 *  Set SW Strap config done
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  pAppCtxt_p         application context
 *  \param[in]  pStackCtxt_p       stack context
 *  \param[in]  phyAddr_p         phy address
 *
 *  <!-- Group: -->
 *
 *  \ingroup APPPHYLIB
 *
 * */
void EC_SLV_APP_TLK110_swStrapConfigDone(void *pAppCtxt_p, void *pStackCtxt_p, uint32_t phyAddr_p)
{
    uint16_t    phyRegVal   = 0;

    OSALUNREF_PARM(pAppCtxt_p);
    OSALUNREF_PARM(phyAddr_p);

    EC_API_SLV_phyRegRead(pStackCtxt_p, APPL_TLK110_CR1_REG, &phyRegVal);
    phyRegVal |= APPL_TLK110_SWSTRAP_CONFIG_DONE;
    EC_API_SLV_phyRegWrite(pStackCtxt_p, APPL_TLK110_CR1_REG, phyRegVal);
}

/*! <!-- Description: -->
 *
 *  \brief
 *  Set power mode
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  pCtxt_p             phyDescriptor context
 *  \param[in]  phyAddr_p           phy address
 *  \param[in]  powerDown_p         False = Normal Operation - True = Power Down
 *
 *  <!-- Group: -->
 *
 *  \ingroup APPPHYLIB
 *
 * */
void EC_SLV_APP_TLK110_powerMode(void* pCtxt_p, void *pStackCtxt_p, uint32_t phyAddr_p, bool powerDown_p)
{
    uint16_t    phyRegVal   = 0;

    OSALUNREF_PARM(pCtxt_p);
    OSALUNREF_PARM(phyAddr_p);
    if(powerDown_p)
    {
        //Power Down mode
        EC_API_SLV_phyRegRead(pStackCtxt_p, APPL_TLK110_BASIC_CTRL, &phyRegVal);
        phyRegVal |= APPL_TLK110_BASIC_CTRL_POWER_DOWN;
        EC_API_SLV_phyRegWrite(pStackCtxt_p, APPL_TLK110_BASIC_CTRL, phyRegVal);

        do
        {
            phyRegVal = 0;
            EC_API_SLV_phyRegRead(pStackCtxt_p, APPL_TLK110_BASIC_CTRL, &phyRegVal);
        }
        while (!(phyRegVal & APPL_TLK110_BASIC_CTRL_POWER_DOWN));
    }
    else
    {
        //Normal Operation
        EC_API_SLV_phyRegRead(pStackCtxt_p, APPL_TLK110_BASIC_CTRL, &phyRegVal);
        phyRegVal &= ~APPL_TLK110_BASIC_CTRL_POWER_DOWN;
        EC_API_SLV_phyRegWrite(pStackCtxt_p, APPL_TLK110_BASIC_CTRL, phyRegVal);

        do
        {
            phyRegVal = 0;
            EC_API_SLV_phyRegRead(pStackCtxt_p, APPL_TLK110_BASIC_CTRL, &phyRegVal);
        }
        while (phyRegVal & APPL_TLK110_BASIC_CTRL_POWER_DOWN);
    }
}

//*************************************************************************************************
