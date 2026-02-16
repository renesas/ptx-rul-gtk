/*
    ---------------------------------------------------------------
    SPDX-License-Identifier: BSD-3-Clause

    Copyright (c) 2025, Renesas Electronics Corporation and/or its affiliates


    Redistribution and use in source and binary forms, with or without modification,
    are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice, this list of
       conditions and the following disclaimer in the documentation and/or other
       materials provided with the distribution.

    3. Neither the name of Renesas nor the names of its
       contributors may be used to endorse or promote products derived from this
       software without specific prior written permission.



    THIS SOFTWARE IS PROVIDED BY Renesas "AS IS" AND ANY EXPRESS
    OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
    OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL RENESAS OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
    OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
    ---------------------------------------------------------------

    Project     : PTX2xxR/W
    Module      : Component Definitions
    File        : ptx_Components.h

    Description :
*/

/**
 * \addtogroup COMPONENTS
 * @{
 */

#if !defined(PTX_COMPONENTS_H_)
#define PTX_COMPONENTS_H_



/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stdint.h>
#include <stddef.h>

/**
 * INTERFACE
 */

#if defined(__cplusplus)
extern "C"
{
#endif

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */


/**
 * \brief Component Definitions
 *
 */
typedef enum ptx_Comps
{
    Comp_None,               /**< No component specified. */
    Comp_IO,                 /**< I/O Component. */
    Comp_NSC,                /**< NSC layer.  */
    Comp_NCI,                /**< NCI layer. */
    Comp_FACTORY,            /**< Factory; Stack Initializer. */
    Comp_IORQ,               /**< IORQ: Handler of the LIFO. */
    Comp_FIFO,               /**< FIFO: Handler of the FIFO. */
    Comp_INTEGRATION,        /**< Integration of all components. */
    Comp_OSAL,               /**< OSAL Component. */
    Comp_HAL,                /**< HAL Component. */
    /* */
    Comp_Reserved_10,        /**< Don't use. Reserved value. */
    Comp_Reserved_11,        /**< Don't use. Reserved value. */
    Comp_Reserved_12,        /**< Don't use. Reserved value. */
    Comp_Reserved_13,        /**< Don't use. Reserved value. */
    Comp_Reserved_14,        /**< Don't use. Reserved value. */
    Comp_Reserved_15,        /**< Don't use. Reserved value. */
    /* */
    Comp_LOG,                /**< Logger Component. */
    Comp_HCI,                /**< HCI Component. */
    Comp_NVM,                /**< NVM Component */
    Comp_BUFFER,             /**< Buffer */
    Comp_PT,                 /**< Processing Thread Component. */
    Comp_AIS,                /**< API AIS Component. */
    Comp_Reserved_22,        /**< Don't use. Reserved value. */
    Comp_Reserved_23,        /**< Don't use. Reserved value. */
    Comp_Reserved_24,        /**< Don't use. Reserved value. */
    Comp_Reserved_25,        /**< Don't use. Reserved value. */
    /* */
    Comp_Reserved_26,        /**< Don't use. Reserved value. */
    Comp_Reserved_27,        /**< Don't use. Reserved value. */
    Comp_Reserved_28,        /**< Don't use. Reserved value. */
    Comp_Reserved_29,        /**< Don't use. Reserved value. */
    Comp_Reserved_30,        /**< Don't use. Reserved value. */
    Comp_Reserved_31,        /**< Don't use. Reserved value. */
    /* */
    Comp_Instrument,         /**< Instrument */
    Comp_Counterpart,        /**< Counterpart component */
    Comp_HAL_HIF,            /**< HAL for Host Interface on the microcontroller */
    Comp_FileMngr,           /**< File Manager, used to manage flash memory */
    Comp_AOSP_INT,           /**< Component ID used for AOSP. */
    Comp_IO_FTDXX,           /**< FTD2XX Subcomponent in IO Component. */
    Comp_Pos,                /**< Pos Component */
    Comp_Apdu,               /**< APDU Component */
    Comp_IoTReader,          /**< IoT Reader */
    Comp_Support,            /**< Support(-Library) */
    /* */
    Comp_NciReader,          /**< NCI (2.0) Reader */
    Comp_TFCI,               /**< TFCI Framework */
    Comp_Reserved_44,        /**< Don't use. Reserved value. */
    Comp_Reserved_45,        /**< Don't use. Reserved value. */
    Comp_Reserved_46,        /**< Don't use. Reserved value. */
    Comp_Reserved_47,        /**< Don't use. Reserved value. */

    /* WPC Stack Specific Components. */
    Comp_WPC_Factory,        /**< WPC Stack; Factory. */
    Comp_WPC_NfcLink,        /**< WPC Stack; NFC-Link Component. */
    Comp_WPC_Pal,            /**< WPC Stack; Protocol Abstraction Layer. */
    Comp_WPC_Int,            /**< INT Stack; Integration Layer. */
    Comp_Reserved_52,        /**< Free */
    Comp_Reserved_53,        /**< Free */
    Comp_Reserved_54,        /**< Free */
    Comp_Reserved_55,        /**< Free */
    Comp_Reserved_56,        /**< Free */
    Comp_Reserved_57,        /**< Free */

    /* Native Tag Components. */
    Comp_Reserved_58,        /**< Don't use. Reserved value. */
    Comp_NativeTag_T2T,      /**< T2T Native Tag */
    Comp_NativeTag_T3T,      /**< T3T Native Tag */
    Comp_NativeTag_T4T,      /**< T4T Native Tag */
    Comp_NativeTag_T5T,      /**< T5T Native Tag */

    Comp_NativeTag_MFCC,     /**< Mifare Classic Compatibility Native Tag*/

    /* Tag Type Operation Components. */
    Comp_Reserved_64,        /**< Don't use. Reserved value. */
    Comp_T2TOP,              /**< T2T Operation Component */
    Comp_T3TOP,              /**< T3T Operation Component */
    Comp_T4TOP,              /**< T4T Operation Component */
    Comp_T5TOP,              /**< T5T Operation Component */

    /* NDEF Components. */
    Comp_NDEF,               /**< NDEF Component */

    /* FeliCa Components. */
    Comp_FeliCa,             /**< FeliCa Classic Library. */

    /* Transparent Mode Components. */
    Comp_TransparentMode,    /**< Transparent Mode. */

    Comp_Reserved_72,        /**< Free */
    Comp_Reserved_73,        /**< Free */

    /* ... */

    Comp_RF_TEST,           /**< RF-Test component*/

    Comp_HIP,                /**< Hardware Interface Protocol Layer */
    Comp_HAL_MOCK,           /**< HAL Mock Component */
    Comp_RUL,                /**< RUL */

    Comp_HCE,                /**< Host Card Emulation */
    Comp_WLC,                /**< Wireless Charging */
    Comp_WLC_PERIPHERALS,    /**< Wireless Charging Peripherals */

    Comp_NFC_Forum_DTA,      /**< NFC Forum DTA. */
    Comp_FeliCa_DTE,         /**< FeliCa DTE. */
    Comp_POS_DTE,            /**< POS DTE. */

    Comp_LLCP_Frame,         /**< LLCP Frame. */
    Comp_LLCP_TLV,           /**< LLCP TLV. */
    Comp_LLCP_TrxState,      /**< LLCP TrxState. */
    Comp_LLCP_MessageBuffer, /**< LLCP MessageBuffer. */
    Comp_LLCP_Sap,           /**< LLCP SAP. */
    Comp_LLCP_Link,          /**< LLCP Link. */
    Comp_LLCP,               /**< LLCP Component. */
    Comp_SNEP,               /**< SNEP Component. */
    Comp_DISCOVERY,          /**< Discovery Extension Component. */
    Comp_VAS_DTE,

    Comp_MAX                 /**< Maximum count. */
} ptx_Comps_t;


/*
 * ####################################################################################################################
 * DEFINES / MACROS
 * ####################################################################################################################
 */

/**
 * \name Component check
 *
 * @{
 */

/**
 * \brief Check whether the component is not NULL and actually what is appears to be.
 */
#define PTX_COMP_CHECK(ptx_p_ctx, ptx_p_comp_id_) \
    ((NULL!=ptx_p_ctx)&&(ptx_p_comp_id_==(ptx_p_ctx)->CompId)) /* */

/** @} */

/** @} */ /* grp_.. */

#if defined(__cplusplus)
}
#endif

#endif

