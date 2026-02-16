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
    Module      : LLCP
    File        : ptx_RUL_LLCP_TLV.h

    Description : Implementation of the Logical Link Control Protocol TLVs.
*/

/**
 * \addtogroup RUL_LLCP
 * @{
 */

#if !defined(PTX_LLCPTLV_H)
#define PTX_LLCPTLV_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include <stddef.h>
#include "ptx_Components.h"
#include "ptx_Status.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#define LLCP_TLV_MAX_PARAMS             (5u)
#define LLCP_CONNECT_SAP_BY_ID_TLVS     (2u)
#define LLCP_CONNECT_SAP_BY_NAME_TLVS   (3u)
#define LLCP_SNL_TLVS                   (5u)

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief LLCP Link Service Class
 */
typedef enum ptx_RUL_LLCP_LSC
{
    LLCP_LSC_Unkown = 0x00, /**< Class unknown at time of Link Activation */
    LLCP_LSC_Class1 = 0x01, /**< Connectionless Link Service */
    LLCP_LSC_Class2 = 0x02, /**< Connection oriented Link Service */
    LLCP_LSC_Class3 = 0x03  /**< Connectionless & connection-oriented Link Service */
} ptx_RUL_LLCP_LSC_t;

/**
 * \brief LLCP TLV type
 */
typedef enum ptx_RUL_LLCP_TagType
{
    LLCP_TagType_RFU        = 0x00, /**< Reserved for future use */
    LLCP_TagType_Version    = 0x01, /**< Version Tag Type */
    LLCP_TagType_MIUX       = 0x02, /**< Maximum Information Unit Extension Tag Type */
    LLCP_TagType_WKS        = 0x03, /**< Well Known Service Tag Type */
    LLCP_TagType_LTO        = 0x04, /**< Link Timeout Tag Type */
    LLCP_TagType_RW         = 0x05, /**< Receive Window Tag Type */
    LLCP_TagType_SN         = 0x06, /**< Service Name Tag Type */
    LLCP_TagType_OPT        = 0x07, /**< Option Tag Type */
    LLCP_TagType_SDREQ      = 0x08, /**< Service Discovery Request Tag Type */
    LLCP_TagType_SDRES      = 0x09, /**< Service Discovery Response Tag Type */
    LLCP_TagType_ECPK       = 0x0A, /**< Elliptic Curve Public Key Tag Type */
    LLCP_TagType_RN         = 0x0B, /**< Random Nonce Tag Type */
    LLCP_TagType_Max                /**< Max Value */
} ptx_RUL_LLCP_TagType_t;

/**
 * \brief LLCP TLV Handler
 */
typedef struct ptx_RUL_LLCP_TLV_Handler
{
    ptx_Comps_t CompId;             /**< Component ID */
    uint8_t    *ScratchPad;         /**< Scratchpad, to form the TLV structure */
    uint32_t    ScratchPadSize;     /**< Scratchpad size (= Size of the buffer) */
    uint32_t    ScratchPadIndex;    /**< Number of "used" bytes within the buffer/scratchpad */
} ptx_RUL_LLCP_TLV_Handler_t;

/**
 * \brief LLCP Tag Length Value
 */
typedef struct ptx_RUL_LLCP_TLV
{
    ptx_RUL_LLCP_TagType_t  Type;    /**< Type of the PDU */
    uint8_t                 Length;  /**< Length of the PDU */
    uint8_t                *Data;    /**< Pointer to the payload */
} ptx_RUL_LLCP_TLV_t;

/**
 * \brief LLCP Version TLV
 */
typedef struct ptx_RUL_LLCP_TLV_Version
{
    uint8_t Major;              /**< Major Version Number */
    uint8_t Minor;              /**< Minor Version Number */
} ptx_RUL_LLCP_TLV_Version_t;

/**
 * \brief LLCP Maximum Information Unit Extension TLV
 */
typedef struct ptx_RUL_LLCP_TLV_Miux
{
    uint16_t Miux;              /**< Maximum Information Unit Extension */
} ptx_RUL_LLCP_TLV_Miux_t;

/**
 * \brief LLCP Well Known Service TLV
 */
typedef struct ptx_RUL_LLCP_TLV_Wks
{
    uint16_t ServiceList;       /**< Service List Bitfield */
} ptx_RUL_LLCP_TLV_Wks_t;

/**
 * \brief LLCP Link Timeout TLV
 */
typedef struct ptx_RUL_LLCP_TLV_Lto
{
    uint8_t Timeout;            /**< Link Timeout */
} ptx_RUL_LLCP_TLV_Lto_t;

/**
 * \brief LLCP Receive Window TLV
 */
typedef struct ptx_RUL_LLCP_TLV_Rw
{
    uint8_t WindowSize;         /**< Receive Window Size */
} ptx_RUL_LLCP_TLV_Rw_t;

/**
 * \brief LLCP Service Name TLV
 */
typedef struct ptx_RUL_LLCP_TLV_ServiceName
{
    char        *Name;          /**< Service Name */
    uint32_t    Length;         /**< Service Name Length */
} ptx_RUL_LLCP_TLV_ServiceName_t;

/**
 * \brief LLCP Option TLV
 */
typedef struct ptx_RUL_LLCP_TLV_Option
{
    ptx_RUL_LLCP_LSC_t  ServiceClass;           /**< Link Service Class */
    uint8_t             DataProtectionClass;    /**< Data Protection Class*/
} ptx_RUL_LLCP_TLV_Option_t;

/**
 * \brief LLCP Service Discovery Request TLV
 */
typedef struct ptx_RUL_LLCP_TLV_Sdreq
{
    uint8_t                         TransactionIdentifier;  /**< Transaction Identifier */
    ptx_RUL_LLCP_TLV_ServiceName_t  ServiceName;            /**< Service Name */
} ptx_RUL_LLCP_TLV_Sdreq_t;

/**
 * \brief LLCP Service Discovery Response TLV
 */
typedef struct ptx_RUL_LLCP_TLV_Sdres
{
    uint8_t TransactionIdentifier;  /**< Transaction Identifier */
    uint8_t ConnectServiceName;     /**< Connect with Service Name Flag */
    uint8_t ServiceAccessPoint;     /**< Service Access Point */
} ptx_RUL_LLCP_TLV_Sdres_t;

/**
 * \brief LLCP Elliptic Curve Public Key TLV
 */
typedef struct ptx_RUL_LLCP_TLV_Ecpk
{
    uint8_t   *CoordinateX;        /** Elliptic Curve Public Key X Coordinate */
    uint8_t   *CoordinateY;        /** Elliptic Curve Public Key Y Coordinate */
    uint32_t   Length;             /** Elliptic Curve Public Key Length */
} ptx_RUL_LLCP_TLV_Ecpk_t;

/**
 * \brief LLCP Random Nonce TLV
 */
typedef struct ptx_RUL_LLCP_TLV_Nonce
{
    uint8_t     *Nonce;             /** Nonce */
    uint32_t    Length;             /** Nonce Length */
} ptx_RUL_LLCP_TLV_Nonce_t;

/**
 * \brief LLCP TLV Structures
 */
typedef union ptx_RUL_LLCP_TLV_Structures
{
    ptx_RUL_LLCP_TLV_Version_t      Version;    /** Version TLV */
    ptx_RUL_LLCP_TLV_Miux_t         Miux;       /** Maximum Information Unit Externsion TLV */
    ptx_RUL_LLCP_TLV_Wks_t          Wks;        /** Well Known Service List TLV */
    ptx_RUL_LLCP_TLV_Lto_t          Lto;        /** Link Timeout TLV */
    ptx_RUL_LLCP_TLV_Rw_t           Rw;         /** Receive Window TLV */
    ptx_RUL_LLCP_TLV_ServiceName_t  Sn;         /** Service Name TLV */
    ptx_RUL_LLCP_TLV_Option_t       Option;     /** Options TLV */
    ptx_RUL_LLCP_TLV_Sdreq_t        SdReq;      /** Service Discovery Request TLV */
    ptx_RUL_LLCP_TLV_Sdres_t        SdRes;      /** Service Discovery Result TLV */
    ptx_RUL_LLCP_TLV_Ecpk_t         Ecpk;       /** Elliptic Curve Public Key TLV */
    ptx_RUL_LLCP_TLV_Nonce_t        Nonce;      /** Nonce TLV */
} ptx_RUL_LLCP_TLV_Structures_t;

/**
 * \brief LLCP TLV Container (Encapsulates Type & Data)
 */
typedef struct ptx_RUL_LLCP_TLV_Data
{
    ptx_RUL_LLCP_TagType_t          Type;
    ptx_RUL_LLCP_TLV_Structures_t   Data;
} ptx_RUL_LLCP_TLV_Data_t;


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Initializes the LLCP-TLV component.
 *
 * This API has to be called before any other API function of this component. It will setup & initialise all the
 * internal data structures.
 *
 * \param[in]       tlv         Pointer to the LLCP-TLV Handler component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_TLV_Open (ptx_RUL_LLCP_TLV_Handler_t *tlv);

/**
 * \brief Tears the LLCP-TLV component down.
 *
 * This API tears the complete LLCP-TLV component down.
 *
 * \param[in]       tlv         Pointer to the LLCP-TLV Handler component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_TLV_Close (ptx_RUL_LLCP_TLV_Handler_t *tlv);

/**
 * \brief Clears the LLCP-TLV buffer.
 *
 * \param[in]       tlv         Pointer to the LLCP-TLV Handler component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_TLV_ClearBuffer (ptx_RUL_LLCP_TLV_Handler_t *tlv);

/**
 * \brief Writes a Version TLV to the internal TLV buffer.
 *
 * \param[in]       tlv         Pointer to the LLCP-TLV Handler component.
 * \param[in]       version     Pointer to the TLV version datastructure.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_TLV_WriteVersion (ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_Version_t *version);

/**
 * \brief Writes a MIUX TLV to the internal TLV buffer.
 *
 * \param[in]       tlv         Pointer to the LLCP-TLV Handler component.
 * \param[in]       miux        Pointer to the miux datastructure.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_TLV_WriteMiux (ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_Miux_t *miux);

/**
 * \brief Writes a WKS TLV to the internal TLV buffer.
 *
 * \param[in]       tlv         Pointer to the LLCP-TLV Handler component.
 * \param[in]       wks         Pointer to the Well-Known-Service datastructure.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_TLV_WriteWks (ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_Wks_t *wks);

/**
 * \brief Writes an LTO TLV to the internal TLV buffer.
 *
 * \param[in]       tlv         Pointer to the LLCP-TLV Handler component.
 * \param[in]       lto         Pointer to the Link-Timeout datastructure.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_TLV_WriteLto (ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_Lto_t *lto);

/**
 * \brief Writes a Receive Windoww Size TLV to the internal TLV buffer.
 *
 * \param[in]       tlv         Pointer to the LLCP-TLV Handler component.
 * \param[in]       rw          Pointer to the Receive Windoww Size datastructure.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_TLV_WriteReceiveWindowSize (ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_Rw_t *rw);

/**
 * \brief Writes a Service Name TLV to the internal TLV buffer.
 *
 * \param[in]       tlv         Pointer to the LLCP-TLV Handler component.
 * \param[in]       serviceName Pointer to the Service Name datastructure.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_TLV_WriteServiceName (ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_ServiceName_t *serviceName);

/**
 * \brief Writes an Option TLV to the internal TLV buffer.
 *
 * \param[in]       tlv         Pointer to the LLCP-TLV Handler component.
 * \param[in]       option      Pointer to the Option datastructure.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_TLV_WriteOption (ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_Option_t *option);

/**
 * \brief Writes a Service Discovery Request TLV to the internal TLV buffer.
 *
 * \param[in]       tlv         Pointer to the LLCP-TLV Handler component.
 * \param[in]       sdReq       Pointer to the Service Discovery Request datastructure.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_TLV_WriteServiceDiscoveryRequest (ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_Sdreq_t *sdReq);

/**
 * \brief Writes a Service Discovery Response TLV to the internal TLV buffer.
 *
 * \param[in]       tlv         Pointer to the LLCP-TLV Handler component.
 * \param[in]       sdRes       Pointer to the Service Discovery Response datastructure.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_TLV_WriteServiceDiscoveryResponse (ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_Sdres_t *sdRes);

/**
 * \brief Writes an Elliptic-Curve-Public-Key-TLV to the internal TLV buffer.
 *
 * \param[in]       tlv         Pointer to the LLCP-TLV Handler component.
 * \param[in]       ecpk        Pointer to the Elliptic-Curve-Public-Key datastructure.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_TLV_WriteECPublicKey (ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_Ecpk_t *ecpk);

/**
 * \brief Writes a NONCE TLV to the internal TLV buffer.
 *
 * \param[in]       tlv         Pointer to the LLCP-TLV Handler component.
 * \param[in]       nonce       Pointer to the Nonce datastructure.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_TLV_WriteNonce (ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_Nonce_t *nonce);

/**
 * \brief Parses the TLV data buffer and extracts all TLVs into dedicated datastructures.
 *
 * \param[in]       tlv         Pointer to the LLCP-TLV Handler component.
 * \param[in]       data        Pointer to the data to be parsed.
 * \param[in]       dataLen     Length of the data to parse.
 * \param[in]       tlvList     Pointer to store the parsed TLV structures.
 * \param[in]       tlvListLen  Pointer to store the length of parsed TLV structures.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_TLV_ProcessReceivedData (ptx_RUL_LLCP_TLV_Handler_t *tlv, uint8_t *data, uint32_t dataLen, ptx_RUL_LLCP_TLV_Data_t *tlvList, uint32_t *tlvListLen);

/**
 * \brief Writes the given list of TLV strucutres to the buffer,.
 *
 * \param[in]       tlv         Pointer to the LLCP-TLV Handler component.
 * \param[in]       tlvList     Pointer to the LLCP-TLV structures.
 * \param[in]       tlvListLen  Length information for the amount of TLVs to write.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_TLV_Write (ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_Data_t *tlvList, uint32_t tlvListLen);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
