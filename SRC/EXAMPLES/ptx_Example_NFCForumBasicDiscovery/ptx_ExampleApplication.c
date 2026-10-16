/*
    ---------------------------------------------------------------
    SPDX-License-Identifier: BSD-3-Clause

    Copyright (c) 2026, Renesas Electronics Corporation and/or its affiliates


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
    Module      : SDK Example (NFC Forum Basic Discovery)
    File        : EXAMPLES/ptx_Example_NFCForumBasicDiscovery/ptx_ExampleApplication.c

    Description : GTK4 application using the PTX205R and the RUL SDK's reference implementation
                  for the Raspberry Pi and SPI.
                  Polls for RF-technologies A, B, F, V. If a card is activated, card
                  information is displayed. If the card contains an NDEF message, it's
                  parsed into its records, which are displayed decoded (URI, Text, Smart Poster,
                  MIME, absolute URI, external types) together with the raw message bytes.
                  Only one single card in the field is supported.
                  On the NDEF tab, an NDEF message can be composed of standard record types (URI, Text,
                  Smart Poster, Phone, Email, SMS, Geo location, vCard, MIME, Wi-Fi, Android app) - see
                  ptx_ExampleNdefRecords.c. The WRITE button starts discovery for max. 10 polling cycles,
                  writes the message to the first activated card, then stops discovery.
                  When the app is launched, the PTX is initialized and discovery can be
                  started/stopped by clicking the START/STOP button on the Discovery tab
                  (RESET clears the displayed card information and restarts discovery), or
                  the READ/STOP buttons on the NDEF tab (NDEF read). There's a menu offering
                  the following options:
                  - Preferences - enable/disable dark mode.
                  - About - display about information.
                  - Quit - close the application.

                  Settings are permanently stored in the gschemas file.
                  If run from the command line, messages will be printed there.

    Licensing   : This program is linked against GTK4 (LGPL v2.1+) and related libraries.
                  See COPYING or COPYING.LESSER for details on LGPL requirements and
                  source availability. See DEPENDENCIES.md and LGPL_COMPLIANCE.md for
                  complete licensing information.
*/

#include "ptx_ExampleApplication.h"
#include "ptx_Status.h"
#include "ptx_RUL.h"
#include "ptx_Utils_AppParameters.h"
#include "ptx_Utils_Print.h"
#include "ptx_Utils_VersionInfo.h"
#include "ptx_Utils_CardInfo.h"
#include "ptx_NDEF.h"
#include "ptx_NDEFRecord_URI.h"
#include "ptx_ExampleNdefRecords.h"
#include "ptx_NDEFRecord_Text.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdarg.h>

#include <gtk/gtk.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES / CONSTANTS
 * ####################################################################################################################
 */

/* Delay/Sleep between checking RF-Discovery state and no device was found. */
static const uint32_t PTX_DEMO_APP_NO_CARD_SLEEP_MS = 10u;

/* Idle time between two consecutive polling-loop cycles. */
static const uint32_t PTX_DEMO_APP_DISCOVERY_IDLE_TIME_MS = 100u;

/* Estimated RF-polling time per polling-loop cycle (NFC-A, -B, -F, -V). The RUL doesn't report polling cycles,
 * so this is used to derive a time budget for waiting for a card when writing. */
static const uint32_t PTX_DEMO_APP_POLLING_TIME_MS = 50u;

/* Max. number of polling-loop cycles to wait for a card when writing an NDEF message. */
#define NDEF_WRITE_MAX_POLLING_CYCLES 10u

/* Device Name used during RUL-initialization. Only used for Desktop-OS versions like Windows or Linux. Can be omitted for MCU platforms. */
static const char *EXAMPLE_DEVICE_NAME = "COM4";

/* Device Name Length. See above. */
static const uint16_t EXAMPLE_DEVICE_NAME_LEN = 4u;

/* Interface Type for the connection to the PTX2xxX chips. */
static const ptx_HAL_Interface_Type_t EXAMPLE_INTERFACE_TYPE = HAL_Interface_Type_SPI;

/* Interface Driver Port implementation type for the connection to the PTX2xxX chips. */
static const ptx_HAL_DRV_Interface_Port_Type_t EXAMPLE_INTERFACE_PORT_TYPE = HAL_DRV_Interface_Port_Type_Default;

/* Interface Driver Port implementation ID for the connection to the PTX2xxX chips. */
static const uint8_t EXAMPLE_INTERFACE_PORT_ID = 0;

/* Requested number of hardware timer instances. */
static const uint8_t EXAMPLE_NUMBER_OF_TIMERS = 2;

/* Keyboard accelerator shortcuts. */
#define ACCEL_QUIT        "<Primary>q"
#define ACCEL_PREFERENCES "<Primary>p"

/* Work-buffer max. sizes for current demo example. */
#define NDEF_BUFFER_SIZE    256u
#define NDEF_MAX_RECORDS    16u        /* Max. number of records parsed per NDEF message */
#define NDEF_MAX_NESTING    2u         /* Max. nesting depth of NDEF messages (e.g. Smart Poster) */

/* NDEF record type of a Smart Poster (NFC Forum RTD), the payload is a nested NDEF message. */
#define NDEF_RECORD_TYPE_RTD_SMART_POSTER     ("Sp")
#define NDEF_RECORD_TYPE_RTD_SMART_POSTER_LEN (2u)
#define APP_TEXT_BUFFER_SIZE 256u
#define HEX_BUFFER_SIZE     256u
#define CWD_BUFFER_SIZE     4096u
#define ERROR_MSG_SIZE      512u

/* What the discovery loop does with an activated card */
typedef enum {
    DiscoveryMode_CardInfo,                    /* Display card information (Discovery tab) */
    DiscoveryMode_NdefRead,                    /* Read NDEF message (NDEF tab) */
    DiscoveryMode_NdefWrite                    /* Write default NDEF message, then stop (NDEF tab) */
} DiscoveryMode;

/* Outcome of an NDEF write attempt */
typedef enum {
    NdefWriteResult_NoCard,                    /* No card activated within the polling-cycle limit */
    NdefWriteResult_Success,                   /* NDEF message written */
    NdefWriteResult_NotNdef,                   /* Card is not NDEF-formatted */
    NdefWriteResult_Failed                     /* Creating or writing the NDEF message failed */
} NdefWriteResult;

/* Consolidated shared state with single mutex protecting all shared data */
typedef struct {
    GMutex mutex;                              /* Single mutex protecting all shared state */

    /* Control flags */
    int gExit;                                 /* Exit flag (was volatile int) */
    gboolean doStopDiscovery;                  /* Was sDoStopDiscovery (volatile bool) */
    gboolean doRunDiscovery;                   /* Was sDoRunDiscovery (volatile bool) */
    gboolean doReset;                          /* Was sDoReset (volatile bool) */
    DiscoveryMode mode;                        /* What to do with an activated card */

    /* NDEF write */
    gint64 writeDeadline;                      /* Monotonic time (us) after which waiting for a card is aborted */
    NdefWriteResult writeResult;               /* Outcome of the last write attempt */
    ptx_Status_t writeStatus;                  /* Status code of the last failed write step */
    ptx_RUL_CardProtocolType_t writeProtocol;  /* Protocol of the card written to */
    uint8_t writeMsgBuffer[NDEF_BUFFER_SIZE];  /* NDEF message to write */
    uint32_t writeMsgLen;                      /* Length of the NDEF message to write */
    guint writeRecordCount;                    /* Number of records in the NDEF message to write */

    /* Card state */
    ptx_RUL_CardParams_t activeCard;           /* Was sActiveCard (volatile) */
    ptx_RUL_CardProtocolType_t activeProtocol; /* Was sActiveProtocol (volatile) */
    gboolean isNdefMessage;                    /* Was sIsNdefMessage (volatile bool) */
    gboolean isNdefMessageRead;                /* Was sIsNdefMessageRead (volatile bool) */

    /* NDEF buffer */
    uint8_t ndefMsgBuffer[NDEF_BUFFER_SIZE];   /* Was sNdefMsgBuffer */
    uint32_t ndefMsgBufferSize;                /* Was sNdefMsgBufferSize (volatile uint32_t) */

    /* Status */
    ptx_Status_t ptxStatus;                    /* Was sPtxStatus (volatile) */
    ptx_Status_t lastError;                    /* Most recent error code */
    char errorContext[256];                    /* Human-readable error context */
    gboolean hasNewError;                      /* Flag for UI update */
} SharedState;

static SharedState sSharedState = {0};

/* Temporary NDEF work buffers (not shared across threads) */
static uint8_t sNdefRxBuffer[NDEF_BUFFER_SIZE];
static uint8_t sNdefTxBuffer[NDEF_BUFFER_SIZE];
static uint8_t sNdefWorkBuffer[NDEF_BUFFER_SIZE];

/* PTX device handle (protected by application synchronization, not by sSharedState mutex) */
static ptx_RUL_t sRul = {0};

typedef struct {
    ptx_Status_t errorCode;
} ErrorCallbackData;

typedef struct {
    gboolean darkMode;
    GtkTextView *contentTextView;
    GtkButton *startStopButton;
    GtkWidget *stackSwitcher;
    GtkButton *ndefReadButton;
    GtkButton *ndefStopButton;
    GtkTextView *ndefTextView;
    GtkButton *ndefWriteButton;
    GtkLabel *ndefWriteStatusLabel;
    GtkWidget *ndefWriteEditor;                /* Fields and record list */
    GtkWidget *ndefWriteEditControls;          /* Record type picker, ADD, REMOVE and CLEAR buttons */
    GtkDropDown *ndefWriteTypeDropDown;
    GtkGrid *ndefWriteFieldsGrid;
    GtkListBox *ndefWriteRecordsList;
    GtkLabel *ndefWriteRawBytesLabel;          /* Raw bytes of the NDEF message to write */
    GPtrArray *writeFieldWidgets;              /* Input widget per field of the selected record type */
    GPtrArray *writeRecords;                   /* NdefRecordData* of the message to write */
    gboolean ptxReady;                         /* PTX initialized and no fatal error */
    gboolean ndefReadRunning;                  /* NDEF read discovery running (until the reader is idle again) */
    gboolean ndefReadStopping;                 /* STOP clicked, waiting for the reader to become idle */
    gboolean ndefWriteRunning;                 /* NDEF write in progress */
    GtkWindow *prefsWindow;
    GtkSwitch *darkModeSwitch;
    GSettings *settings;
    GtkWindow *aboutWindow;
    GThread *workerThread;
    GThread *initThread;
} AppState;


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
static void appendToTextBuffer(GtkTextBuffer *buffer, const char *fmt, ...);
static void formatHexBytes(char *buffer, size_t maxLen, const volatile uint8_t *data, size_t len);
static void displayCardTechInfo(GtkTextBuffer *buffer, const volatile ptx_RUL_CardParams_t *card);
static void updateNdefButtons(AppState *state);
typedef GtkWindow* (*LoadWindowFunc)(GtkApplication *app, AppState *state, GtkWindow *parent);
static void presentWindow(GtkWindow **window, LoadWindowFunc loader, GtkApplication *app, AppState *state);
static ptx_Status_t runDemo();
static ptx_Status_t processStateStartDiscovery(ptx_RUL_t *rul, demoState_t *demoState);
static ptx_Status_t processStateWaitForDiscovery(ptx_RUL_t *rul, demoState_t *demoState, ptx_RUL_CardRegistry_t *cardRegistry);
static ptx_Status_t processStateDataExchange(ptx_RUL_t *rul, demoState_t *demoState, ptx_RUL_CardRegistry_t *cardRegistry);
static ptx_Status_t processStateRestartDiscovery(ptx_RUL_t *rul, demoState_t *demoState);
static ptx_Status_t processStateDeactivateReader(ptx_RUL_t *rul, demoState_t *demoState);
static ptx_Status_t writeNdefMessage(ptx_RUL_t *rul, ptx_RUL_CardProtocolType_t protocol);
static gboolean onNdefWriteDone(gpointer data);
static gboolean onNdefReadStopped(gpointer data);

static inline void lockSharedState(void) {
    g_mutex_lock(&sSharedState.mutex);
}

static inline void unlockSharedState(void) {
    g_mutex_unlock(&sSharedState.mutex);
}

static inline void setErrorState(ptx_Status_t status, const char *context) {
    lockSharedState();
    sSharedState.lastError = status;
    sSharedState.hasNewError = TRUE;
    snprintf(sSharedState.errorContext, sizeof(sSharedState.errorContext), "%s", context);
    unlockSharedState();
}

static const char* getErrorDescription(ptx_Status_t status)
{
    switch(status) {
        case PTX_STATUS_SUCCESS: return "Success";
        case PTX_STATUS_E_GEN_INVALID_PARAMETER: return "Invalid parameter";
        case PTX_STATUS_E_GEN_INTERNAL: return "Internal system error";
        case PTX_STATUS_E_GEN_TIMEOUT: return "Timeout occurred";
        default: return "Unknown error";
    }
}

static const char* getProtocolTypeString(ptx_RUL_CardProtocolType_t protocol)
{
    switch(protocol) {
        case ptx_RUL_CardProtocolType_T2T: return "Protocol: T2T";
        case ptx_RUL_CardProtocolType_T3T: return "Protocol: T3T";
        case ptx_RUL_CardProtocolType_T4T: return "Protocol: T4T (ISO-DEP)";
        case ptx_RUL_CardProtocolType_NfcDep: return "Protocol: P2P (NFC-DEP)";
        case ptx_RUL_CardProtocolType_T5T: return "Protocol: T5T";
        default: return "Protocol: Unknown / Unsupported";
    }
}

static void appendToTextBuffer(GtkTextBuffer *buffer, const char *fmt, ...)
{
    char content[APP_TEXT_BUFFER_SIZE];
    va_list args;
    va_start(args, fmt);
    vsnprintf(content, sizeof(content), fmt, args);
    va_end(args);

    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));
}

static void formatHexBytes(char *buffer, size_t maxLen, const volatile uint8_t *data, size_t len)
{
    size_t n = 0;
    for (size_t i = 0; i < len && n + 2 < maxLen; ++i) {
        int written = snprintf(&buffer[n], maxLen - n, "%02X", data[i]);
        if (written > 0) {
            n += (size_t)written;
        }
    }
}

static void presentWindow(GtkWindow **window, LoadWindowFunc loader, GtkApplication *app, AppState *state)
{
    GtkWindow *parent = gtk_application_get_active_window(app);
    if (!*window) {
        *window = loader(app, state, parent);
    } else if (parent) {
        gtk_window_set_transient_for(*window, parent);
    }
    gtk_window_present(*window);
}

static void displayCardTechInfo(GtkTextBuffer *buffer, const volatile ptx_RUL_CardParams_t *card)
{
    char hexBuf[HEX_BUFFER_SIZE];

    switch(card->TechType)
    {
        case ptx_RUL_CardRfTechType_A:
            appendToTextBuffer(buffer, "RF-technology: Type-A\n");
            appendToTextBuffer(buffer, "SENS_RES: ");
            formatHexBytes(hexBuf, sizeof(hexBuf), card->TechParam.TypeA.SensRes, sizeof(card->TechParam.TypeA.SensRes));
            appendToTextBuffer(buffer, "%s\n", hexBuf);
            appendToTextBuffer(buffer, "ID length: %02X\n", card->TechParam.TypeA.NfcId1Len);
            appendToTextBuffer(buffer, "ID: ");
            formatHexBytes(hexBuf, sizeof(hexBuf), card->TechParam.TypeA.NfcId1, card->TechParam.TypeA.NfcId1Len);
            appendToTextBuffer(buffer, "%s\n", hexBuf);
            break;

        case ptx_RUL_CardRfTechType_B:
            appendToTextBuffer(buffer, "RF-technology: Type-B\n");
            appendToTextBuffer(buffer, "SENSB_RES: ");
            formatHexBytes(hexBuf, sizeof(hexBuf), card->TechParam.TypeB.SensBRes, sizeof(card->TechParam.TypeB.SensBRes));
            appendToTextBuffer(buffer, "%s\n", hexBuf);
            break;

        case ptx_RUL_CardRfTechType_F:
            appendToTextBuffer(buffer, "RF-technology: Type-F\n");
            appendToTextBuffer(buffer, "SENSF_RES: ");
            formatHexBytes(hexBuf, sizeof(hexBuf), card->TechParam.TypeF.SensFRes, sizeof(card->TechParam.TypeF.SensFRes));
            appendToTextBuffer(buffer, "%s\n", hexBuf);
            break;

        case ptx_RUL_CardRfTechType_V:
            appendToTextBuffer(buffer, "RF-technology: Type-V\n");
            appendToTextBuffer(buffer, "INVENTORY_RES: ");
            formatHexBytes(hexBuf, sizeof(hexBuf), card->TechParam.TypeV.InventoryRes, sizeof(card->TechParam.TypeV.InventoryRes));
            appendToTextBuffer(buffer, "%s\n", hexBuf);
            break;

        default:
            appendToTextBuffer(buffer, "RF-technology: Unsupported\n");
            break;
    }
}

static gboolean displayDiscoveryLoopError(gpointer data)
{
    ErrorCallbackData *errorData = (ErrorCallbackData *)data;
    GtkApplication *app = GTK_APPLICATION(g_application_get_default());
    AppState *state = g_object_get_data(G_OBJECT(app), "app-state");

    if (state == NULL) {
        g_free(errorData);
        return FALSE;
    }

    const char *description = getErrorDescription(errorData->errorCode);
    GtkTextBuffer *const buffer = gtk_text_view_get_buffer(state->contentTextView);

    appendToTextBuffer(buffer, "\nERROR 0x%04X - %s\nPlease restart the application.\n",
                       errorData->errorCode, description);
    appendToTextBuffer(gtk_text_view_get_buffer(state->ndefTextView),
                       "\nERROR 0x%04X - %s\nPlease restart the application.\n",
                       errorData->errorCode, description);

    gtk_widget_set_sensitive(GTK_WIDGET(state->startStopButton), FALSE);
    state->ptxReady = FALSE;
    updateNdefButtons(state);
    gtk_widget_set_sensitive(state->stackSwitcher, TRUE);

    char content[128];
    snprintf(content, sizeof(content), "ERROR 0x%04X - %s\nPlease restart the application.", errorData->errorCode, description);
    gtk_label_set_text(state->ndefWriteStatusLabel, content);

    g_free(errorData);
    return FALSE;
}

static gboolean updateCardInfo()
{
    lockSharedState();
    gboolean isRunning = sSharedState.doRunDiscovery;
    unlockSharedState();

    if(!isRunning)
        return FALSE;

    GtkApplication *app = GTK_APPLICATION(g_application_get_default());
    AppState *state = g_object_get_data(G_OBJECT(app), "app-state");
    GtkTextBuffer * const buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(state->contentTextView));

    gtk_text_buffer_set_text(buffer, "Card activated\n", -1);

    lockSharedState();
    ptx_RUL_CardParams_t localCard = sSharedState.activeCard;
    ptx_RUL_CardProtocolType_t localProtocol = sSharedState.activeProtocol;
    unlockSharedState();

    displayCardTechInfo(buffer, &localCard);
    appendToTextBuffer(buffer, "%s\n", getProtocolTypeString(localProtocol));

    return FALSE;
}


static const char* getTnfString(uint8_t tnf)
{
    switch(tnf) {
        case TNF_EMPTY: return "Empty";
        case TNF_WELL_KNOWN_TYPE: return "Well-known";
        case TNF_MEDIA_TYPE: return "Media type";
        case TNF_ABSOLUTE_URI: return "Absolute URI";
        case TNF_EXTERNAL_TYPE: return "External";
        case TNF_UNKNOWN: return "Unknown";
        case TNF_UNCHANGED: return "Unchanged";
        default: return "Reserved";
    }
}

/*
 * Parses an NDEF message into its records. The record fields point into 'buf'.
 * Unlike ptx_NDEFMessage_Parse, every record is checked against the end of the buffer and long-record
 * payload lengths are decoded big-endian. On error, 'count' holds the number of records parsed so far.
 */
static ptx_Status_t parseNdefMessage(uint8_t *buf, uint32_t len, ptx_NDEFRecord_t *records, uint32_t maxRecords, uint32_t *count)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INTERNAL;
    uint32_t pos = 0;
    uint8_t me = 0;

    *count = 0;

    while (!me && (pos < len))
    {
        if (*count >= maxRecords)
        {
            status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
            break;
        }

        /* Header byte and type length */
        if ((len - pos) < 2u)
            break;

        uint8_t header = buf[pos++];
        uint8_t typeLen = buf[pos++];
        uint32_t payloadLen = 0;
        uint8_t idLen = 0;

        if ((0u == *count) && (0u == (header & PTX_FLAG_MASK_MB)))
            break;

        if (header & PTX_FLAG_MASK_SR)
        {
            if ((len - pos) < 1u)
                break;
            payloadLen = buf[pos++];
        }
        else
        {
            if ((len - pos) < 4u)
                break;
            payloadLen = ((uint32_t)buf[pos] << 24) | ((uint32_t)buf[pos + 1] << 16) |
                         ((uint32_t)buf[pos + 2] << 8) | (uint32_t)buf[pos + 3];
            pos += 4u;
        }

        if (header & PTX_FLAG_MASK_IL)
        {
            if ((len - pos) < 1u)
                break;
            idLen = buf[pos++];
        }

        /* Type, ID and payload must fit into the remaining buffer */
        uint32_t remaining = len - pos;
        if ((typeLen > remaining) || (idLen > (remaining - typeLen)) || (payloadLen > (remaining - typeLen - idLen)))
            break;

        ptx_NDEFRecord_t *record = &records[*count];
        me = header & PTX_FLAG_MASK_ME;
        ptx_NDEFRecord_SetMB(record, header & PTX_FLAG_MASK_MB);
        ptx_NDEFRecord_SetME(record, me);
        ptx_NDEFRecord_SetCF(record, header & PTX_FLAG_MASK_CF);
        ptx_NDEFRecord_SetSR(record, header & PTX_FLAG_MASK_SR);
        ptx_NDEFRecord_SetIL(record, header & PTX_FLAG_MASK_IL);
        ptx_NDEFRecord_SetTNF(record, header & PTX_FLAG_MASK_TNF);

        record->Type.Length = typeLen;
        record->Type.bData = (typeLen > 0u) ? &buf[pos] : NULL;
        pos += typeLen;
        record->ID.Length = idLen;
        record->ID.Data = (idLen > 0u) ? &buf[pos] : NULL;
        pos += idLen;
        record->Payload.Length = payloadLen;
        record->Payload.Data = &buf[pos];
        pos += payloadLen;

        (*count)++;

        if (me)
            status = PTX_STATUS_SUCCESS;
    }

    return status;
}

/* Appends bytes as hex digits without separators (raw NDEF message). */
static void appendHexString(GString *out, const uint8_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; ++i) {
        g_string_append_printf(out, "%02X", data[i]);
    }
}

static void appendHexBytes(GString *out, const uint8_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; ++i) {
        g_string_append_printf(out, (i == 0) ? "%02X" : " %02X", data[i]);
    }
}

/* Appends bytes as UTF-8, invalid sequences are replaced. */
static void appendUtf8(GString *out, const uint8_t *data, uint32_t len)
{
    gchar *valid = g_utf8_make_valid((const gchar *)data, (gssize)len);
    g_string_append(out, valid);
    g_free(valid);
}

/* TRUE if the bytes are valid UTF-8 without control characters (except tab and line breaks). */
static gboolean isPrintableUtf8(const uint8_t *data, uint32_t len)
{
    if (!g_utf8_validate((const gchar *)data, (gssize)len, NULL))
        return FALSE;

    for (uint32_t i = 0; i < len; ++i) {
        if ((data[i] < 0x20u) && (data[i] != '\t') && (data[i] != '\n') && (data[i] != '\r'))
            return FALSE;
    }
    return TRUE;
}

static gboolean isRecordType(const ptx_NDEFRecord_t *record, const char *type, uint8_t typeLen)
{
    return (record->Type.Length == typeLen) && (0 == memcmp(record->Type.bData, type, typeLen));
}

static void appendNdefRecords(GString *out, uint8_t *buf, uint32_t len, guint depth);

static void appendUriPayload(GString *out, const ptx_NDEFRecord_t *record)
{
    const uint8_t *payload = record->Payload.Data;
    uint32_t len = record->Payload.Length;

    g_string_append(out, "URI: ");
    if (len > 0u)
    {
        const char *prefix = ptx_ExampleNdef_GetUriPrefix(payload[0]);
        if (NULL != prefix)
            g_string_append(out, prefix);
        else
            g_string_append_printf(out, "[unknown prefix 0x%02X]", payload[0]);
        appendUtf8(out, &payload[1], len - 1u);
    }
    g_string_append_c(out, '\n');
}

static void appendTextPayload(GString *out, const ptx_NDEFRecord_t *record, const char *indent)
{
    const uint8_t *payload = record->Payload.Data;
    uint32_t len = record->Payload.Length;

    if (len < 1u)
    {
        g_string_append_printf(out, "%sText: (malformed, missing status byte)\n", indent);
        return;
    }

    gboolean isUtf16 = (payload[0] & 0x80u) != 0u;
    uint32_t langLen = payload[0] & 0x3Fu;

    if (langLen > (len - 1u))
    {
        g_string_append_printf(out, "%sText: (malformed, language code exceeds payload)\n", indent);
        return;
    }

    const uint8_t *text = &payload[1u + langLen];
    uint32_t textLen = len - 1u - langLen;

    g_string_append_printf(out, "%sLanguage: ", indent);
    appendUtf8(out, &payload[1], langLen);
    g_string_append_printf(out, "\n%sText: ", indent);

    if (isUtf16)
    {
        /* Without BOM, UTF-16 text is big-endian (NFC Forum Text RTD). With BOM, let iconv detect the byte order. */
        gboolean hasBom = (textLen >= 2u) &&
                          (((text[0] == 0xFEu) && (text[1] == 0xFFu)) || ((text[0] == 0xFFu) && (text[1] == 0xFEu)));
        gchar *utf8 = g_convert((const gchar *)text, (gssize)textLen, "UTF-8", hasBom ? "UTF-16" : "UTF-16BE", NULL, NULL, NULL);
        if (utf8) {
            g_string_append(out, utf8);
            g_free(utf8);
        } else {
            g_string_append(out, "(invalid UTF-16) ");
            appendHexBytes(out, text, textLen);
        }
    }
    else
    {
        appendUtf8(out, text, textLen);
    }
    g_string_append_c(out, '\n');
}

static void appendNdefRecord(GString *out, const ptx_NDEFRecord_t *record, uint32_t index, guint depth)
{
    gchar *indent = g_strnfill(depth * 4u, ' ');
    uint8_t tnf = record->HeaderFlags.TNF;

    g_string_append_printf(out, "%sRecord %u: TNF=%s", indent, index + 1u, getTnfString(tnf));
    if (record->Type.Length > 0u)
    {
        g_string_append(out, ", Type=");
        appendUtf8(out, record->Type.bData, record->Type.Length);
    }
    if (record->ID.Length > 0u)
    {
        g_string_append(out, ", ID=");
        appendUtf8(out, record->ID.Data, record->ID.Length);
    }
    if (record->HeaderFlags.CF)
    {
        g_string_append(out, " (chunked, not reassembled)");
    }
    g_string_append_c(out, '\n');

    /* Payloads that aren't decoded below are shown as hex. */
    gboolean decoded = TRUE;

    if (TNF_WELL_KNOWN_TYPE == tnf && isRecordType(record, PTX_NDEFRECORD_TYPE_RTD_URI, PTX_NDEFRECORD_TYPE_RTD_URI_LEN))
    {
        g_string_append(out, indent);
        appendUriPayload(out, record);
    }
    else if (TNF_WELL_KNOWN_TYPE == tnf && isRecordType(record, PTX_NDEFRECORD_TYPE_RTD_TEXT, PTX_NDEFRECORD_TYPE_RTD_TEXT_LEN))
    {
        appendTextPayload(out, record, indent);
    }
    else if (TNF_WELL_KNOWN_TYPE == tnf && isRecordType(record, NDEF_RECORD_TYPE_RTD_SMART_POSTER, NDEF_RECORD_TYPE_RTD_SMART_POSTER_LEN))
    {
        g_string_append_printf(out, "%sSmart Poster:\n", indent);
        appendNdefRecords(out, record->Payload.Data, record->Payload.Length, depth + 1u);
    }
    else if (TNF_EMPTY == tnf)
    {
        g_string_append_printf(out, "%sEmpty record\n", indent);
    }
    else if (TNF_ABSOLUTE_URI == tnf)
    {
        g_string_append_printf(out, "%sURI: ", indent);
        appendUtf8(out, record->Type.bData, record->Type.Length);
        g_string_append_c(out, '\n');
        decoded = (0u == record->Payload.Length);
    }
    else if ((TNF_MEDIA_TYPE == tnf) && isPrintableUtf8(record->Payload.Data, record->Payload.Length))
    {
        g_string_append_printf(out, "%sContent: ", indent);
        appendUtf8(out, record->Payload.Data, record->Payload.Length);
        g_string_append_c(out, '\n');
    }
    else
    {
        decoded = FALSE;
    }

    if (!decoded)
    {
        g_string_append_printf(out, "%sPayload (%u bytes): ", indent, record->Payload.Length);
        appendHexBytes(out, record->Payload.Data, record->Payload.Length);
        g_string_append_c(out, '\n');
    }

    g_free(indent);
}

/* Parses an NDEF message and appends a human-readable description of all of its records. */
static void appendNdefRecords(GString *out, uint8_t *buf, uint32_t len, guint depth)
{
    ptx_NDEFRecord_t records[NDEF_MAX_RECORDS];
    uint32_t count = 0;
    ptx_Status_t status = parseNdefMessage(buf, len, records, NDEF_MAX_RECORDS, &count);

    for (uint32_t i = 0; i < count; ++i)
    {
        if ((depth >= NDEF_MAX_NESTING) &&
            isRecordType(&records[i], NDEF_RECORD_TYPE_RTD_SMART_POSTER, NDEF_RECORD_TYPE_RTD_SMART_POSTER_LEN))
        {
            g_string_append_printf(out, "%*sRecord %u: Smart Poster nested too deep, not parsed\n", (int)(depth * 4u), "", i + 1u);
            continue;
        }
        appendNdefRecord(out, &records[i], i, depth);
    }

    if (PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES == status)
    {
        g_string_append_printf(out, "%*sWarning: more than %u records, remaining records not shown.\n", (int)(depth * 4u), "", NDEF_MAX_RECORDS);
    }
    else if (PTX_STATUS_SUCCESS != status)
    {
        g_string_append_printf(out, "%*sWarning: malformed NDEF message, parsing stopped after %u record(s).\n", (int)(depth * 4u), "", count);
    }
}

static const char* getProtocolShortString(ptx_RUL_CardProtocolType_t protocol)
{
    switch(protocol) {
        case ptx_RUL_CardProtocolType_T2T: return "T2T";
        case ptx_RUL_CardProtocolType_T3T: return "T3T";
        case ptx_RUL_CardProtocolType_T4T: return "T4T (ISO-DEP)";
        case ptx_RUL_CardProtocolType_NfcDep: return "P2P (NFC-DEP)";
        case ptx_RUL_CardProtocolType_T5T: return "T5T";
        default: return "Unknown";
    }
}

/* Appends a one-line card summary: RF technology, protocol and card ID. */
static void appendCardSummary(GString *out, const ptx_RUL_CardParams_t *card, ptx_RUL_CardProtocolType_t protocol)
{
    const char *tech = "Unsupported";
    const uint8_t *id = NULL;
    uint32_t idLen = 0;
    uint8_t uid[PTX_RUL_UID_LEN];

    switch(card->TechType)
    {
        case ptx_RUL_CardRfTechType_A:
            tech = "Type-A";
            id = card->TechParam.TypeA.NfcId1;
            idLen = MIN(card->TechParam.TypeA.NfcId1Len, PTX_RUL_NFCID1_MAX_LEN);
            break;

        case ptx_RUL_CardRfTechType_B:
            tech = "Type-B";
            id = &card->TechParam.TypeB.SensBRes[PTX_RUL_SENSB_RES_NFCID0_OFFSET];
            idLen = PTX_RUL_NFCID0_LEN;
            break;

        case ptx_RUL_CardRfTechType_F:
            tech = "Type-F";
            id = &card->TechParam.TypeF.SensFRes[PTX_RUL_SENSF_RES_NFCID2_OFFSET];
            idLen = PTX_RUL_NFCID2_LEN;
            break;

        case ptx_RUL_CardRfTechType_V:
            /* UID in INVENTORY_RES (after flags and DSFID) is transmitted LSB first - display it MSB first */
            tech = "Type-V";
            for (uint32_t i = 0; i < PTX_RUL_UID_LEN; ++i) {
                uid[i] = card->TechParam.TypeV.InventoryRes[2u + PTX_RUL_UID_LEN - 1u - i];
            }
            id = uid;
            idLen = PTX_RUL_UID_LEN;
            break;

        default:
            break;
    }

    g_string_append_printf(out, "Card: RF-Tech %s, Protocol %s", tech, getProtocolShortString(protocol));
    if (NULL != id)
    {
        g_string_append(out, ", ID ");
        appendHexString(out, id, idLen);
    }
    g_string_append_c(out, '\n');
}

static gboolean updateNdefInfo()
{
    lockSharedState();
    gboolean isRunning = sSharedState.doRunDiscovery;
    gboolean isNdef = sSharedState.isNdefMessage;
    gboolean isNdefRead = sSharedState.isNdefMessageRead;
    uint32_t ndefSize = sSharedState.ndefMsgBufferSize;
    ptx_RUL_CardParams_t localCard = sSharedState.activeCard;
    ptx_RUL_CardProtocolType_t localProtocol = sSharedState.activeProtocol;
    unlockSharedState();

    if(!isRunning)
        return FALSE;

    GtkApplication *app = GTK_APPLICATION(g_application_get_default());
    AppState *state = g_object_get_data(G_OBJECT(app), "app-state");
    GtkTextBuffer * const buffer = gtk_text_view_get_buffer(state->ndefTextView);

    /* Decoded records and raw bytes may exceed APP_TEXT_BUFFER_SIZE so build the text dynamically */
    GString *text = g_string_new(NULL);
    appendCardSummary(text, &localCard, localProtocol);

    if(isNdef)
    {
        if(isNdefRead)
        {
            // Make a local copy of the NDEF buffer
            uint8_t ndefCopy[NDEF_BUFFER_SIZE] = {0};
            if (ndefSize > NDEF_BUFFER_SIZE)
                ndefSize = NDEF_BUFFER_SIZE;
            lockSharedState();
            memcpy(ndefCopy, sSharedState.ndefMsgBuffer, ndefSize);
            unlockSharedState();

            g_string_append_printf(text, "NDEF message (%u bytes):\n", ndefSize);
            appendNdefRecords(text, ndefCopy, ndefSize, 0);

            g_string_append(text, "Raw bytes: ");
            appendHexString(text, ndefCopy, ndefSize);
            g_string_append_c(text, '\n');
        }
        else
        {
            g_string_append(text, "Could not read NDEF message.");
        }
    }
    else
    {
        g_string_append(text, "No NDEF message found on card.");
    }

    gtk_text_buffer_set_text(buffer, text->str, (int)text->len);
    g_string_free(text, TRUE);

    return FALSE;
}

static gint onInitDone(gpointer userData)
{
    GtkApplication *app = GTK_APPLICATION(userData);
    AppState *state = g_object_get_data(G_OBJECT(app), "app-state");
    GtkTextBuffer * buffer = gtk_text_view_get_buffer(state->contentTextView);

    lockSharedState();
    ptx_Status_t status = sSharedState.ptxStatus;
    unlockSharedState();

    if(status == PTX_STATUS_SUCCESS)
    {
        gtk_text_buffer_set_text(buffer, "System initialized. Press START to begin discovery!", -1);
        gtk_widget_set_sensitive(GTK_WIDGET(state->startStopButton), TRUE);
        state->ptxReady = TRUE;
        updateNdefButtons(state);
        GAction* resetAction = g_action_map_lookup_action(G_ACTION_MAP(app), "reset");
        GAction* prefsAction = g_action_map_lookup_action(G_ACTION_MAP(app), "preferences");
        g_simple_action_set_enabled(G_SIMPLE_ACTION(resetAction), TRUE);
        g_simple_action_set_enabled(G_SIMPLE_ACTION(prefsAction), TRUE);
    }
    else
    {
        char content[128];
        snprintf(content, sizeof(content), "System initialization ERROR: 0x%04X\nPlease check the system and restart the application!", status);
        gtk_text_buffer_set_text(buffer, content, (int)strlen(content));
    }

    return 0;
}

static gpointer workerThread()
{
    g_print("Worker thread started.\n");
    ptx_Status_t status = runDemo();
    g_print("Demo loop exited with status %04X.\n", status);

    return NULL;
}

static gpointer initThread(gpointer userData)
{
    g_print("Init thread started.\n");
    ptx_Status_t status;

    /* Demo Application Parameters */
    ptx_Utils_AppParams_t appParams;

    /* Main API component */
    ptx_RUL_InitParams_t rulInitParams = {0};

    /* Initialize demo application parameters with default values. */
    (void)memcpy(&appParams.DeviceName[0], EXAMPLE_DEVICE_NAME, EXAMPLE_DEVICE_NAME_LEN);
    appParams.DeviceNameLen = EXAMPLE_DEVICE_NAME_LEN;

    /* Initialize Main RUL API Component.
     *
     * Note: The used Host-Interface i.e. the HAL-component and the Logging-component (if used) get selected via the Build-process (see User Manual)
     *       and get also initialized during the next call to "ptx_RUL_Init".
     */
    rulInitParams.InterfaceInitParams.InterfaceName = (uint8_t*)&appParams.DeviceName[0];
    rulInitParams.InterfaceInitParams.InterfaceNameLen = appParams.DeviceNameLen;
    rulInitParams.InterfaceInitParams.InterfaceType = EXAMPLE_INTERFACE_TYPE;
    rulInitParams.InterfaceInitParams.InterfacePortType = EXAMPLE_INTERFACE_PORT_TYPE;
    rulInitParams.InterfaceInitParams.InterfacePortID = EXAMPLE_INTERFACE_PORT_ID;
    rulInitParams.InterfaceInitParams.NumberOfTimers = EXAMPLE_NUMBER_OF_TIMERS;

    status = ptx_RUL_Init(&sRul, &rulInitParams);

    if (PTX_STATUS_SUCCESS == status)
    {
        /* Initialize the Hardware by
        *  - downloading the Firmware-image and starting execution,
        *  - initializing the hardware / firmware,
        *  - downloading the default RF-configuration,
        *  - changing into READY-state.
        */
        status = ptx_RUL_InitHardware(&sRul);
    }

    lockSharedState();
    sSharedState.ptxStatus = status;
    unlockSharedState();
    g_idle_add(onInitDone, userData);
    g_print("Init thread finished with status %04X\n", status);

    return NULL;
}

static void startDiscovery(AppState *state, DiscoveryMode mode)
{
    gtk_widget_set_sensitive(state->stackSwitcher, FALSE);

    lockSharedState();
    sSharedState.mode = mode;
    sSharedState.doRunDiscovery = true;
    unlockSharedState();

    if(state->workerThread == NULL)
    {
        state->workerThread = g_thread_new("worker", workerThread, NULL);
    }
}

static void stopDiscovery(AppState *state)
{
    gtk_widget_set_sensitive(state->stackSwitcher, TRUE);

    lockSharedState();
    sSharedState.doStopDiscovery = true;
    sSharedState.activeCard = (ptx_RUL_CardParams_t){0};
    unlockSharedState();
}

static void onStartStop(GtkWidget *widget, gpointer userData)
{
    (void)widget;
    AppState *state = g_object_get_data(G_OBJECT(userData), "app-state");

    lockSharedState();
    gboolean isRunning = sSharedState.doRunDiscovery;
    unlockSharedState();

    if(!isRunning)
    {
        GtkTextBuffer * buffer = gtk_text_view_get_buffer(state->contentTextView);
        gtk_text_buffer_set_text(buffer, "", -1);
        gtk_button_set_label(state->startStopButton, "STOP");
        startDiscovery(state, DiscoveryMode_CardInfo);
    }
    else
    {
        gtk_button_set_label(state->startStopButton, "START");
        stopDiscovery(state);
    }
}

/*
 * Sets the sensitivity of the NDEF tab buttons, all of them require an initialized PTX:
 * - READ:  no NDEF read or write running.
 * - STOP:  NDEF read running and not being stopped already.
 * - WRITE: no NDEF read or write running and the message isn't empty.
 */
static void updateNdefButtons(AppState *state)
{
    gboolean idle = state->ptxReady && !state->ndefReadRunning && !state->ndefWriteRunning;

    gtk_widget_set_sensitive(GTK_WIDGET(state->ndefReadButton), idle);
    gtk_widget_set_sensitive(GTK_WIDGET(state->ndefStopButton),
                             state->ptxReady && state->ndefReadRunning && !state->ndefReadStopping);
    gtk_widget_set_sensitive(GTK_WIDGET(state->ndefWriteButton), idle && (state->writeRecords->len > 0u));
}

/* Shows the number of records, the size and the raw bytes of the NDEF message to write. */
static void updateWriteSummary(AppState *state)
{
    gtk_label_set_text(state->ndefWriteRawBytesLabel, "Raw bytes: -");

    if (0u == state->writeRecords->len)
    {
        gtk_label_set_text(state->ndefWriteStatusLabel, "No records - pick a record type, fill in the fields and press ADD.");
        return;
    }

    uint8_t message[NDEF_BUFFER_SIZE];
    uint32_t len = (uint32_t)sizeof(message);
    GError *error = NULL;
    char content[APP_TEXT_BUFFER_SIZE];

    if (ptx_ExampleNdef_BuildMessage(state->writeRecords, message, &len, &error))
    {
        snprintf(content, sizeof(content), "%u record(s), %u / %u bytes. Press WRITE to write them to a card.",
                 state->writeRecords->len, len, NDEF_BUFFER_SIZE);

        /* Raw bytes may exceed APP_TEXT_BUFFER_SIZE so build the text dynamically */
        GString *rawBytes = g_string_new(NULL);
        g_string_append_printf(rawBytes, "Raw bytes (%u): ", len);
        appendHexString(rawBytes, message, len);
        gtk_label_set_text(state->ndefWriteRawBytesLabel, rawBytes->str);
        g_string_free(rawBytes, TRUE);
    }
    else
    {
        snprintf(content, sizeof(content), "%s", error->message);
        g_error_free(error);
    }
    gtk_label_set_text(state->ndefWriteStatusLabel, content);
}

static void refreshWriteRecordList(AppState *state)
{
    gtk_list_box_remove_all(state->ndefWriteRecordsList);

    for (guint i = 0; i < state->writeRecords->len; ++i)
    {
        const NdefRecordData *data = g_ptr_array_index(state->writeRecords, i);
        gchar *text = g_strdup_printf("%u. %s", i + 1u, data->summary);
        GtkWidget *label = gtk_label_new(text);
        gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
        gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
        gtk_widget_set_margin_start(label, 4);
        gtk_widget_set_margin_end(label, 4);
        gtk_list_box_append(state->ndefWriteRecordsList, label);
        g_free(text);
    }

    updateNdefButtons(state);
}

static const NdefRecordTypeDef *getSelectedRecordType(AppState *state)
{
    guint count = 0;
    const NdefRecordTypeDef *types = ptx_ExampleNdef_GetRecordTypes(&count);
    guint selected = gtk_drop_down_get_selected(state->ndefWriteTypeDropDown);
    return (selected < count) ? &types[selected] : NULL;
}

/* Returns the current field values (borrowed strings, free the array only). */
static const gchar **collectWriteFieldValues(AppState *state)
{
    const gchar **values = g_new0(const gchar *, state->writeFieldWidgets->len + 1u);

    for (guint i = 0; i < state->writeFieldWidgets->len; ++i)
    {
        GtkWidget *widget = g_ptr_array_index(state->writeFieldWidgets, i);
        if (GTK_IS_DROP_DOWN(widget))
        {
            GObject *item = gtk_drop_down_get_selected_item(GTK_DROP_DOWN(widget));
            values[i] = (NULL != item) ? gtk_string_object_get_string(GTK_STRING_OBJECT(item)) : "";
        }
        else
        {
            values[i] = gtk_editable_get_text(GTK_EDITABLE(widget));
        }
    }

    return values;
}

static void onWriteAdd(GtkWidget *widget, gpointer userData);

/* Builds the input form of the selected record type from its field definitions. */
static void rebuildWriteFields(AppState *state, GtkApplication *app)
{
    GtkWidget *child;
    while (NULL != (child = gtk_widget_get_first_child(GTK_WIDGET(state->ndefWriteFieldsGrid))))
    {
        gtk_grid_remove(state->ndefWriteFieldsGrid, child);
    }
    g_ptr_array_set_size(state->writeFieldWidgets, 0);

    const NdefRecordTypeDef *def = getSelectedRecordType(state);
    if (NULL == def)
        return;

    for (guint i = 0; i < def->nFields; ++i)
    {
        const NdefFieldDef *field = &def->fields[i];
        gchar *labelText = g_strdup_printf("%s%s", field->label, field->required ? " *" : "");
        GtkWidget *label = gtk_label_new(labelText);
        GtkWidget *input;
        g_free(labelText);
        gtk_label_set_xalign(GTK_LABEL(label), 0.0f);

        if (NdefField_Choice == field->kind)
        {
            input = gtk_drop_down_new_from_strings(field->choices);
            for (guint c = 0; (NULL != field->defaultValue) && (NULL != field->choices[c]); ++c)
            {
                if (0 == g_strcmp0(field->choices[c], field->defaultValue))
                    gtk_drop_down_set_selected(GTK_DROP_DOWN(input), c);
            }
        }
        else
        {
            input = gtk_entry_new();
            gtk_editable_set_text(GTK_EDITABLE(input), (NULL != field->defaultValue) ? field->defaultValue : "");
            gtk_entry_set_placeholder_text(GTK_ENTRY(input), field->required ? "required" : "optional");
            g_signal_connect(input, "activate", G_CALLBACK(onWriteAdd), app);
        }
        gtk_widget_set_hexpand(input, TRUE);

        gtk_grid_attach(state->ndefWriteFieldsGrid, label, 0, (int)i, 1, 1);
        gtk_grid_attach(state->ndefWriteFieldsGrid, input, 1, (int)i, 1, 1);
        g_ptr_array_add(state->writeFieldWidgets, input);
    }
}

static void onWriteTypeChanged(GObject *object, GParamSpec *pspec, gpointer userData)
{
    (void)object;
    (void)pspec;
    AppState *state = g_object_get_data(G_OBJECT(userData), "app-state");
    rebuildWriteFields(state, GTK_APPLICATION(userData));
}

/* Encodes the record from the form and adds it to the message, if the message still fits into the buffer. */
static void onWriteAdd(GtkWidget *widget, gpointer userData)
{
    (void)widget;
    AppState *state = g_object_get_data(G_OBJECT(userData), "app-state");
    const NdefRecordTypeDef *def = getSelectedRecordType(state);

    if ((NULL == def) || state->ndefWriteRunning)
        return;

    const gchar **values = collectWriteFieldValues(state);
    GError *error = NULL;
    NdefRecordData *data = ptx_ExampleNdef_EncodeRecord(def, values, &error);
    g_free(values);

    if (NULL == data)
    {
        gtk_label_set_text(state->ndefWriteStatusLabel, error->message);
        g_error_free(error);
        return;
    }

    g_ptr_array_add(state->writeRecords, data);

    uint8_t message[NDEF_BUFFER_SIZE];
    uint32_t len = (uint32_t)sizeof(message);
    if (!ptx_ExampleNdef_BuildMessage(state->writeRecords, message, &len, &error))
    {
        g_ptr_array_remove_index(state->writeRecords, state->writeRecords->len - 1u);
        gchar *text = g_strdup_printf("Record not added - %s", error->message);
        gtk_label_set_text(state->ndefWriteStatusLabel, text);
        g_free(text);
        g_error_free(error);
        return;
    }

    refreshWriteRecordList(state);
    updateWriteSummary(state);
}

static void onWriteRemove(GtkWidget *widget, gpointer userData)
{
    (void)widget;
    AppState *state = g_object_get_data(G_OBJECT(userData), "app-state");
    GtkListBoxRow *row = gtk_list_box_get_selected_row(state->ndefWriteRecordsList);

    if (NULL == row)
    {
        gtk_label_set_text(state->ndefWriteStatusLabel, "Select a record in the list to remove it.");
        return;
    }

    int index = gtk_list_box_row_get_index(row);
    if ((index >= 0) && ((guint)index < state->writeRecords->len))
    {
        g_ptr_array_remove_index(state->writeRecords, (guint)index);
    }
    refreshWriteRecordList(state);
    updateWriteSummary(state);
}

static void onWriteClear(GtkWidget *widget, gpointer userData)
{
    (void)widget;
    AppState *state = g_object_get_data(G_OBJECT(userData), "app-state");
    g_ptr_array_set_size(state->writeRecords, 0);
    refreshWriteRecordList(state);
    updateWriteSummary(state);
}

/* Adds a record of the first record type (URI) with its default values, so WRITE works out of the box. */
static void addDefaultWriteRecord(AppState *state)
{
    guint count = 0;
    const NdefRecordTypeDef *def = ptx_ExampleNdef_GetRecordTypes(&count);
    const gchar **values = g_new0(const gchar *, def->nFields + 1u);

    for (guint i = 0; i < def->nFields; ++i)
    {
        values[i] = (NULL != def->fields[i].defaultValue) ? def->fields[i].defaultValue : "";
    }

    NdefRecordData *data = ptx_ExampleNdef_EncodeRecord(def, values, NULL);
    if (NULL != data)
    {
        g_ptr_array_add(state->writeRecords, data);
    }
    g_free(values);
}

static void onNdefRead(GtkWidget *widget, gpointer userData)
{
    (void)widget;
    AppState *state = g_object_get_data(G_OBJECT(userData), "app-state");

    lockSharedState();
    gboolean isRunning = sSharedState.doRunDiscovery;
    unlockSharedState();

    if(isRunning || state->ndefReadRunning || state->ndefWriteRunning)
        return;

    GtkTextBuffer * buffer = gtk_text_view_get_buffer(state->ndefTextView);
    gtk_text_buffer_set_text(buffer, "", -1);
    state->ndefReadRunning = TRUE;
    startDiscovery(state, DiscoveryMode_NdefRead);
    updateNdefButtons(state);
}

static void onNdefStop(GtkWidget *widget, gpointer userData)
{
    (void)widget;
    AppState *state = g_object_get_data(G_OBJECT(userData), "app-state");

    if(!state->ndefReadRunning || state->ndefReadStopping)
        return;

    state->ndefReadStopping = TRUE;
    stopDiscovery(state);
    updateNdefButtons(state);
}

/* Called once discovery has been stopped after an NDEF read. */
static gboolean onNdefReadStopped(gpointer data)
{
    (void)data;
    GtkApplication *app = GTK_APPLICATION(g_application_get_default());
    AppState *state = g_object_get_data(G_OBJECT(app), "app-state");

    if (state == NULL)
        return FALSE;

    state->ndefReadRunning = FALSE;
    state->ndefReadStopping = FALSE;
    updateNdefButtons(state);

    return FALSE;
}

static void onNdefWrite(GtkWidget *widget, gpointer userData)
{
    (void)widget;
    AppState *state = g_object_get_data(G_OBJECT(userData), "app-state");

    lockSharedState();
    gboolean isRunning = sSharedState.doRunDiscovery;
    unlockSharedState();

    if(isRunning)
    {
        gtk_label_set_text(state->ndefWriteStatusLabel, "Discovery is still being stopped - please try again.");
        return;
    }

    uint8_t message[NDEF_BUFFER_SIZE];
    uint32_t len = (uint32_t)sizeof(message);
    GError *error = NULL;

    if (!ptx_ExampleNdef_BuildMessage(state->writeRecords, message, &len, &error))
    {
        gtk_label_set_text(state->ndefWriteStatusLabel, error->message);
        g_error_free(error);
        return;
    }

    lockSharedState();
    memcpy(sSharedState.writeMsgBuffer, message, len);
    sSharedState.writeMsgLen = len;
    sSharedState.writeRecordCount = state->writeRecords->len;
    unlockSharedState();

    state->ndefWriteRunning = TRUE;
    updateNdefButtons(state);
    gtk_widget_set_sensitive(state->ndefWriteEditor, FALSE);
    gtk_widget_set_sensitive(state->ndefWriteEditControls, FALSE);
    gtk_label_set_text(state->ndefWriteStatusLabel, "Waiting for card ...");
    startDiscovery(state, DiscoveryMode_NdefWrite);
}

/* Called once discovery has been stopped after a write attempt. */
static gboolean onNdefWriteDone(gpointer data)
{
    (void)data;
    GtkApplication *app = GTK_APPLICATION(g_application_get_default());
    AppState *state = g_object_get_data(G_OBJECT(app), "app-state");

    if (state == NULL)
        return FALSE;

    lockSharedState();
    NdefWriteResult result = sSharedState.writeResult;
    ptx_Status_t writeStatus = sSharedState.writeStatus;
    ptx_RUL_CardProtocolType_t protocol = sSharedState.writeProtocol;
    uint32_t msgLen = sSharedState.writeMsgLen;
    guint recordCount = sSharedState.writeRecordCount;
    unlockSharedState();

    char content[APP_TEXT_BUFFER_SIZE];
    switch(result)
    {
        case NdefWriteResult_Success:
            snprintf(content, sizeof(content), "NDEF message written successfully (%u record(s), %u bytes).\n%s",
                     recordCount, msgLen, getProtocolTypeString(protocol));
            break;

        case NdefWriteResult_NotNdef:
            snprintf(content, sizeof(content), "Card is not NDEF-formatted.\n%s", getProtocolTypeString(protocol));
            break;

        case NdefWriteResult_Failed:
            snprintf(content, sizeof(content), "Writing NDEF message failed: ERROR 0x%04X - %s\n%s",
                     writeStatus, getErrorDescription(writeStatus), getProtocolTypeString(protocol));
            break;

        case NdefWriteResult_NoCard:
        default:
            snprintf(content, sizeof(content), "No card found within %u polling cycles.", NDEF_WRITE_MAX_POLLING_CYCLES);
            break;
    }
    gtk_label_set_text(state->ndefWriteStatusLabel, content);

    state->ndefWriteRunning = FALSE;
    updateNdefButtons(state);
    gtk_widget_set_sensitive(state->ndefWriteEditor, TRUE);
    gtk_widget_set_sensitive(state->ndefWriteEditControls, TRUE);
    gtk_widget_set_sensitive(state->stackSwitcher, TRUE);

    return FALSE;
}

static void onCloseRequest(GtkApplication *app, gpointer userData)
{
    (void)app;
    lockSharedState();
    sSharedState.doRunDiscovery = false;
    sSharedState.gExit = 1;
    unlockSharedState();
    gtk_window_destroy(GTK_WINDOW(userData));
}

static gboolean onDarkModeSwitchSet(GtkSwitch *self, gboolean state, gpointer userData)
{
    (void)self;
    (void)userData;
    
    g_object_set(gtk_settings_get_default(), "gtk-application-prefer-dark-theme", state, NULL);
    // Return FALSE to let the switch change its visual state normally
    return FALSE; 
}

/* Setup function for preferences window */
static void setupPrefsWindow(GtkBuilder *builder, AppState *state)
{
    state->darkModeSwitch = GTK_SWITCH(gtk_builder_get_object(builder, "darkModeSwitch"));
    gtk_switch_set_active(state->darkModeSwitch, state->darkMode);
    g_settings_bind(state->settings, "dark-mode",
                    state->darkModeSwitch, "active",
                    G_SETTINGS_BIND_DEFAULT);
    g_signal_connect(state->darkModeSwitch, "state-set", G_CALLBACK(onDarkModeSwitchSet), NULL);
}

/* Setup function for about window (empty, no special setup needed) */
static void setupAboutWindow(GtkBuilder *builder, AppState *state)
{
    (void)builder;
    (void)state;
}

/* Generic window loader that handles common setup */
static GtkWindow* loadWindowFromUI(const char *filename, const char *objectName,
                                    GtkApplication *app, GtkWindow *parent,
                                    AppState *state,
                                    void (*setupFn)(GtkBuilder*, AppState*))
{
    GtkBuilder *builder = gtk_builder_new_from_file(filename);
    GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, objectName));

    if (!window) {
        g_warning("Failed to load '%s' from %s", objectName, filename);
        g_object_unref(builder);
        return NULL;
    }

    gtk_window_set_application(GTK_WINDOW(window), app);
    if (parent) gtk_window_set_transient_for(GTK_WINDOW(window), parent);

    if (setupFn) setupFn(builder, state);

    g_object_unref(builder);
    return GTK_WINDOW(window);
}

static GtkWindow *loadPrefsWindow(GtkApplication *app, AppState *state, GtkWindow *parent)
{
    return loadWindowFromUI("prefs.ui", "prefsWindow", app, parent, state, setupPrefsWindow);
}

static GtkWindow *loadAboutWindow(GtkApplication *app, AppState *state, GtkWindow *parent)
{
    return loadWindowFromUI("about.ui", "aboutWindow", app, parent, state, setupAboutWindow);
}

static void onReset(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    (void)action;
    (void)parameter;

    AppState *state = g_object_get_data(G_OBJECT(userData), "app-state");
    GtkTextBuffer * buffer = gtk_text_view_get_buffer(state->contentTextView);
    gtk_text_buffer_set_text(buffer, "", -1);

    lockSharedState();
    gboolean isRunning = sSharedState.doRunDiscovery;
    if(isRunning)
        sSharedState.doReset = true;
    sSharedState.activeCard = (ptx_RUL_CardParams_t){0};
    unlockSharedState();
}

static void onPreferences(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    (void)action;
    (void)parameter;

    GtkApplication *app = GTK_APPLICATION(userData);
    AppState *state = g_object_get_data(G_OBJECT(app), "app-state");
    presentWindow(&state->prefsWindow, (LoadWindowFunc)loadPrefsWindow, app, state);
}

static void onAbout(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    (void)action;
    (void)parameter;

    GtkApplication *app = GTK_APPLICATION(userData);
    AppState *state = g_object_get_data(G_OBJECT(app), "app-state");
    presentWindow(&state->aboutWindow, (LoadWindowFunc)loadAboutWindow, app, state);
}

static void onQuit(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    (void)action;
    (void)parameter;

    lockSharedState();
    sSharedState.doRunDiscovery = false;
    sSharedState.gExit = 1;
    unlockSharedState();
    g_application_quit(G_APPLICATION(userData));
}

static void freeAppState(AppState *state)
{
    g_ptr_array_unref(state->writeFieldWidgets);
    g_ptr_array_unref(state->writeRecords);
    g_free(state);
}

static void onActivate(GtkApplication *app, gpointer userData)
{
    (void)userData;

    // initialize mutexes for thread-safe access
    g_mutex_init(&sSharedState.mutex);

    // intialize app state
    AppState *state = g_new0(AppState, 1);
    state->writeFieldWidgets = g_ptr_array_new();
    state->writeRecords = g_ptr_array_new_with_free_func((GDestroyNotify)ptx_ExampleNdef_RecordDataFree);
    g_object_set_data_full(G_OBJECT(app), "app-state", state, (GDestroyNotify)freeAppState);

    // set up settings
    g_setenv("GSETTINGS_SCHEMA_DIR", ".", FALSE); // compiled schema file is expected to be inside the current working directory
    char cwdPath[CWD_BUFFER_SIZE];
    gchar *cwd = g_get_current_dir();
    if (!cwd) {
        g_warning("Failed to get current directory, using fallback");
        cwd = g_strdup(".");
    }
    snprintf(cwdPath, sizeof(cwdPath), "%s/", cwd);
    state->settings = g_settings_new_with_path("app", cwdPath);
    g_free(cwd);
    state->darkMode = g_settings_get_boolean(state->settings, "dark-mode");
    g_object_set(gtk_settings_get_default(), "gtk-application-prefer-dark-theme", state->darkMode, NULL);

    // set up UI
    GtkBuilder *windowBuilder = gtk_builder_new_from_file("window.ui");
    /* Connect signal handlers to the constructed widgets. */
    GObject *window = gtk_builder_get_object(windowBuilder, "window");
    gtk_window_set_application(GTK_WINDOW(window), app);
    g_signal_connect(window, "close_request", G_CALLBACK(onCloseRequest), window);

    GObject *obj = gtk_builder_get_object(windowBuilder, "startstop");
    if(obj == NULL) {
        g_warning("Failed to load 'startstop' button from window.ui - application may not function correctly");
        g_object_unref(windowBuilder);
        return;
    }
    state->startStopButton = GTK_BUTTON(obj);
    gtk_widget_set_sensitive(GTK_WIDGET(state->startStopButton), FALSE);
    g_signal_connect(state->startStopButton, "clicked", G_CALLBACK(onStartStop), app);

    obj = gtk_builder_get_object(windowBuilder, "cardInfoText");
    if(obj == NULL) {
        g_warning("Failed to load 'cardInfoText' from window.ui - cannot display card information");
        g_object_unref(windowBuilder);
        return;
    }
    state->contentTextView = GTK_TEXT_VIEW(obj);

    obj = gtk_builder_get_object(windowBuilder, "stackSwitcher");
    if(obj == NULL) {
        g_warning("Failed to load 'stackSwitcher' from window.ui - application may not function correctly");
        g_object_unref(windowBuilder);
        return;
    }
    state->stackSwitcher = GTK_WIDGET(obj);

    obj = gtk_builder_get_object(windowBuilder, "ndefRead");
    if(obj == NULL) {
        g_warning("Failed to load 'ndefRead' button from window.ui - application may not function correctly");
        g_object_unref(windowBuilder);
        return;
    }
    state->ndefReadButton = GTK_BUTTON(obj);
    gtk_widget_set_sensitive(GTK_WIDGET(state->ndefReadButton), FALSE);
    g_signal_connect(state->ndefReadButton, "clicked", G_CALLBACK(onNdefRead), app);

    obj = gtk_builder_get_object(windowBuilder, "ndefStop");
    if(obj == NULL) {
        g_warning("Failed to load 'ndefStop' button from window.ui - application may not function correctly");
        g_object_unref(windowBuilder);
        return;
    }
    state->ndefStopButton = GTK_BUTTON(obj);
    gtk_widget_set_sensitive(GTK_WIDGET(state->ndefStopButton), FALSE);
    g_signal_connect(state->ndefStopButton, "clicked", G_CALLBACK(onNdefStop), app);

    obj = gtk_builder_get_object(windowBuilder, "ndefText");
    if(obj == NULL) {
        g_warning("Failed to load 'ndefText' from window.ui - cannot display NDEF message");
        g_object_unref(windowBuilder);
        return;
    }
    state->ndefTextView = GTK_TEXT_VIEW(obj);

    obj = gtk_builder_get_object(windowBuilder, "ndefWrite");
    if(obj == NULL) {
        g_warning("Failed to load 'ndefWrite' button from window.ui - application may not function correctly");
        g_object_unref(windowBuilder);
        return;
    }
    state->ndefWriteButton = GTK_BUTTON(obj);
    gtk_widget_set_sensitive(GTK_WIDGET(state->ndefWriteButton), FALSE);
    g_signal_connect(state->ndefWriteButton, "clicked", G_CALLBACK(onNdefWrite), app);

    obj = gtk_builder_get_object(windowBuilder, "ndefWriteStatus");
    if(obj == NULL) {
        g_warning("Failed to load 'ndefWriteStatus' from window.ui - cannot display NDEF write status");
        g_object_unref(windowBuilder);
        return;
    }
    state->ndefWriteStatusLabel = GTK_LABEL(obj);

    /* NDEF write editor: record type picker, field form, record list and their buttons */
    const char * const writeEditorIds[] = { "ndefWriteEditor", "ndefWriteType", "ndefWriteFields", "ndefWriteRecords",
                                            "ndefWriteAdd", "ndefWriteRemove", "ndefWriteClear", "ndefWriteRawBytes", "ndefWriteEditControls" };
    GObject *writeEditorObjects[G_N_ELEMENTS(writeEditorIds)];
    for (guint i = 0; i < G_N_ELEMENTS(writeEditorIds); ++i)
    {
        writeEditorObjects[i] = gtk_builder_get_object(windowBuilder, writeEditorIds[i]);
        if(writeEditorObjects[i] == NULL) {
            g_warning("Failed to load '%s' from window.ui - cannot compose NDEF messages", writeEditorIds[i]);
            g_object_unref(windowBuilder);
            return;
        }
    }
    state->ndefWriteEditor = GTK_WIDGET(writeEditorObjects[0]);
    state->ndefWriteTypeDropDown = GTK_DROP_DOWN(writeEditorObjects[1]);
    state->ndefWriteFieldsGrid = GTK_GRID(writeEditorObjects[2]);
    state->ndefWriteRecordsList = GTK_LIST_BOX(writeEditorObjects[3]);
    g_signal_connect(writeEditorObjects[4], "clicked", G_CALLBACK(onWriteAdd), app);
    g_signal_connect(writeEditorObjects[5], "clicked", G_CALLBACK(onWriteRemove), app);
    g_signal_connect(writeEditorObjects[6], "clicked", G_CALLBACK(onWriteClear), app);
    state->ndefWriteRawBytesLabel = GTK_LABEL(writeEditorObjects[7]);
    state->ndefWriteEditControls = GTK_WIDGET(writeEditorObjects[8]);

    /* The record type picker is populated from the record type table */
    guint recordTypeCount = 0;
    const NdefRecordTypeDef *recordTypes = ptx_ExampleNdef_GetRecordTypes(&recordTypeCount);
    GtkStringList *recordTypeNames = gtk_string_list_new(NULL);
    for (guint i = 0; i < recordTypeCount; ++i)
    {
        gtk_string_list_append(recordTypeNames, recordTypes[i].name);
    }
    gtk_drop_down_set_model(state->ndefWriteTypeDropDown, G_LIST_MODEL(recordTypeNames));
    g_object_unref(recordTypeNames);
    g_signal_connect(state->ndefWriteTypeDropDown, "notify::selected", G_CALLBACK(onWriteTypeChanged), app);
    rebuildWriteFields(state, app);

    addDefaultWriteRecord(state);
    refreshWriteRecordList(state);
    updateWriteSummary(state);

    GtkTextBuffer * buffer = gtk_text_view_get_buffer(state->contentTextView);
    gtk_text_buffer_set_text(buffer, "Initializing ...", -1);

    // set up menu
    GObject *menuButton = gtk_builder_get_object(windowBuilder, "menu");
    GtkBuilder *menuBuilder = gtk_builder_new_from_file("menu.ui");
    GMenuModel *menuModel = G_MENU_MODEL(gtk_builder_get_object(menuBuilder, "menu"));
    gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(menuButton), menuModel);

    // add menu actions
    const GActionEntry actions[] = {
        { .name = "reset", .activate = onReset },
        { .name = "preferences", .activate = onPreferences },
        { .name = "about", .activate = onAbout },
        { .name = "quit", .activate = onQuit },
    };

    g_action_map_add_action_entries(G_ACTION_MAP(app),
                                    actions,
                                    G_N_ELEMENTS(actions),
                                    app);
    
    gtk_application_set_accels_for_action(app, "app.quit", (const char *[]) { ACCEL_QUIT, NULL });
    gtk_application_set_accels_for_action(app, "app.preferences", (const char *[]) { ACCEL_PREFERENCES, NULL });
    
    GAction* resetAction = g_action_map_lookup_action(G_ACTION_MAP(app), "reset");
    GAction* prefsAction = g_action_map_lookup_action(G_ACTION_MAP(app), "preferences");

    // disable menu items until the PTX is initialized
    g_simple_action_set_enabled(G_SIMPLE_ACTION(resetAction), FALSE);
    g_simple_action_set_enabled(G_SIMPLE_ACTION(prefsAction), FALSE);

    // initialze the PTX
    state->initThread = g_thread_new("init", initThread, app);

    gtk_widget_set_visible(GTK_WIDGET (window), TRUE);

    g_object_unref(windowBuilder);
    g_object_unref(menuBuilder);
}

/*
 * Add a platform dependent handler for a console event.
 * It is used to catch the user key press application-quit event Ctrl-C.
 */
static void handler(int sig)
{
    if (sig == SIGINT)
    {
        ptx_Utils_Print("Application interrupted by user\n");
        sSharedState.gExit = 1;
    }
    else
    {
        exit(sig);
    }
}

int main(int argc, char **argv)
{
    signal(SIGINT,  handler);

    GtkApplication *gtkApp = gtk_application_new("com.renesas.nfc",
                                            G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(gtkApp, "activate", G_CALLBACK(onActivate), NULL);
    g_application_run (G_APPLICATION (gtkApp), argc, argv);
    AppState *state = g_object_get_data(G_OBJECT(gtkApp), "app-state");

    if(!state)
        return 0;

    if(state->initThread != NULL)
    {   // wait for thread to end and unref
        g_thread_join(state->initThread);
        g_thread_unref(state->initThread);
    }

    if(state->workerThread != NULL)
    {   // wait for thread to end and unref
        g_thread_join(state->workerThread);
        g_thread_unref(state->workerThread);
    }

    // cleanup mutexes
    g_mutex_clear(&sSharedState.mutex);

    ptx_RUL_DeactivateParams_t deactParams = {0};

    /* Switch RF-Field off (independent of previous state) */
    deactParams.Type = ptx_RUL_DeactivateType_Generic;
    deactParams.State = ptx_RUL_DeactivateState_Idle;
    (void)ptx_RUL_Deactivate(&sRul, &deactParams);

    /* De-initialize the RUL API Component (independent of previous state) */
    ptx_Status_t status = ptx_RUL_DeInit(&sRul);

    if (PTX_STATUS_SUCCESS == status)
    {
        ptx_Utils_Print("RUL-Stack successfully de-initialized!\n");
    }
    else
    {
        ptx_Utils_Print("RUL-Stack de-initialization failed with Error-Status %04X!\n", status);
    }

    (void)argc;
    (void)argv;
    g_object_unref(gtkApp);

    return 0;
}

static ptx_Status_t runDemo()
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    demoState_t demoState = demoStateStartDiscovery;
    ptx_RUL_Status_Params_t statusInfo = {0};
    ptx_RUL_CardRegistry_t *cardRegistry = NULL;

    /* Get and store a pointer to the RUL card registry */
    status = ptx_RUL_GetCardRegistry(&sRul, &cardRegistry);

    if (PTX_STATUS_SUCCESS == status)
    {
        gboolean doRun = FALSE;
        gboolean doReset = FALSE;
        gboolean doStop = FALSE;
        int exitFlag = 0;

        do
        {
            lockSharedState();
            doRun = sSharedState.doRunDiscovery;
            doReset = sSharedState.doReset;
            doStop = sSharedState.doStopDiscovery;
            if(doReset)
                sSharedState.doReset = false;
            exitFlag = sSharedState.gExit;
            unlockSharedState();

            if(doRun)
            {
                if(doReset)
                {
                    demoState = demoStateRestartDiscovery;
                }

                if(doStop)
                {
                    demoState = demoStateDeactivateReader;
                }
                /* Check regularly for System-state/-health (e.g. temperature/over-current errors etc.) */
                statusInfo.Type = ptx_RUL_StatusType_System;

                status = ptx_RUL_GetStatusInfo(&sRul, &statusInfo);

                if (PTX_STATUS_SUCCESS == status)
                {
                    if (ptx_RUL_StatusInfoSystem_ErrorStandby == statusInfo.Info.System)
                    {
                        demoState = demoStateStartDiscovery;
                    }
                    else if (ptx_RUL_StatusInfoSystem_OK != statusInfo.Info.System)
                    {
                        lockSharedState();
                        sSharedState.ptxStatus = PTX_STATUS_E_GEN_INTERNAL;
                        unlockSharedState();
                        ptx_Utils_Print("ERROR [System error: status code %d]: %s (0x%04X)\n", statusInfo.Info.System, getErrorDescription(PTX_STATUS_E_GEN_INTERNAL), PTX_STATUS_E_GEN_INTERNAL);
                        status = PTX_STATUS_E_GEN_INTERNAL;
                    }
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    switch (demoState)
                    {
                        case demoStateStartDiscovery:
                            status = processStateStartDiscovery(&sRul, &demoState);
                            break;

                        case demoStateWaitForDiscovery:
                            status = processStateWaitForDiscovery(&sRul, &demoState, cardRegistry);
                            break;

                        case demoStateDataExchange:
                            status = processStateDataExchange(&sRul, &demoState, cardRegistry);
                            break;

                        case demoStateRestartDiscovery:
                            status = processStateRestartDiscovery(&sRul, &demoState);
                            break;

                        case demoStateDeactivateReader:
                            status = processStateDeactivateReader(&sRul, &demoState);
                            break;

                        case demoStateExit:
                            break;

                        default:
                            lockSharedState();
                            sSharedState.ptxStatus = PTX_STATUS_E_GEN_INTERNAL;
                            unlockSharedState();
                            ptx_Utils_Print("ERROR [Invalid application state detected]: %s (0x%04X)\n", getErrorDescription(PTX_STATUS_E_GEN_INTERNAL), PTX_STATUS_E_GEN_INTERNAL);
                            demoState = demoStateExit;
                            break;
                    }
                }
            }
        }
        while ((PTX_STATUS_SUCCESS == status) && (demoStateExit != demoState) && (0 == exitFlag));
    }

    if (status != PTX_STATUS_SUCCESS) {
        ErrorCallbackData *errorData = g_malloc0(sizeof(ErrorCallbackData));
        errorData->errorCode = status;
        g_idle_add(displayDiscoveryLoopError, errorData);
    }

    return status;
}

static ptx_Status_t processStateStartDiscovery(ptx_RUL_t *rul, demoState_t *demoState)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_RUL_DiscoverConfig_t discParams = {0};

    if ((NULL != rul) && (NULL != demoState))
    {
        /* Initialize RF-Discovery / Polling Loop in NFC Forum Poll-Mode with standard parameters.*/
        discParams.PollMode = ptx_RUL_PollMode_NfcForum;   /* Enable PolL-Mode only in this example (Listen-mode can be optionally enabled). */
        discParams.EnablePolling = 1u;                     /* Enable Polling only (Listen-mode can be optionally enabled). */
        discParams.IdleTime = PTX_DEMO_APP_DISCOVERY_IDLE_TIME_MS; /* System pauses / idles for 100ms after every full polling loop-cycle. */
        discParams.PollTypeA = 1u;                         /* Poll for NFC-A Technology. */
        discParams.PollTypeAPollFrequency = 1u;            /* Poll for NFC-A Technology during every Polling-Loop cycle (otherwise every n-th cycle). */
        discParams.PollTypeADeviceLimit = 5u;              /* Limit the number of NFC-A devices / cards to max. 5 devices (application can define max. value). */
        discParams.PollTypeB = 1u;                         /* Poll for NFC-B Technology. */
        discParams.PollTypeBPollFrequency = 1u;            /* Poll for NFC-B Technology during every Polling-Loop cycle (otherwise every n-th cycle). */
        discParams.PollTypeBDeviceLimit = 5u;              /* Limit the number of NFC-B devices / cards to max. 5 devices (application can define max. value). */
        discParams.PollTypeF212 = 1u;                      /* Poll for NFC-F 212 kBit/s Technology. */
        discParams.PollTypeFPollFrequency = 1u;            /* Poll for NFC-F Technology during every Polling-Loop cycle (otherwise every n-th cycle). */
        discParams.PollTypeFDeviceLimit = 5u;              /* Limit the number of NFC-F devices / cards to max. 5 devices (application can define max. value). */
        discParams.PollTypeV = 1u;                         /* Poll for NFC-V Technology. */
        discParams.PollTypeVPollFrequency = 1u;            /* Poll for NFC-V Technology during every Polling-Loop cycle (otherwise every n-th cycle). */
        discParams.PollTypeVDeviceLimit = 5u;              /* Limit the number of NFC-V devices / cards to max. 5 devices (application can define max. value). */
        discParams.EnableLPCD = 0;                         /* Don't use low-power-card-detection (LPCD) in this demo. */
#if !defined(PTX_DISABLE_LISTEN_MODE)
        discParams.ListenMode = 0;                         /* Don't use Listen-mode (Card Emulation) in this demo. */
#endif /* PTX_DISABLE_LISTEN_MODE */

        status = ptx_RUL_InitiateDiscovery(rul, &discParams);

        if (PTX_STATUS_SUCCESS == status)
        {
            *demoState = demoStateWaitForDiscovery;

            /* When writing, wait for a card for a limited number of polling-loop cycles only. */
            lockSharedState();
            if (DiscoveryMode_NdefWrite == sSharedState.mode)
            {
                sSharedState.writeResult = NdefWriteResult_NoCard;
                sSharedState.writeDeadline = g_get_monotonic_time() +
                    (gint64)NDEF_WRITE_MAX_POLLING_CYCLES * (PTX_DEMO_APP_DISCOVERY_IDLE_TIME_MS + PTX_DEMO_APP_POLLING_TIME_MS) * 1000;
            }
            unlockSharedState();

            ptx_Utils_Print("RF-Discovery successfully started - waiting for Cards ... \n");
        }
        else
        {
            *demoState = demoStateExit;
            lockSharedState();
            sSharedState.ptxStatus = status;
            unlockSharedState();
            ptx_Utils_Print("ERROR [RF-Discovery initialization failed]: %s (0x%04X)\n", getErrorDescription(status), status);
        }
    }

    return status;
}

static ptx_Status_t processStateWaitForDiscovery(ptx_RUL_t *rul, demoState_t *demoState, ptx_RUL_CardRegistry_t *cardRegistry)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != rul) && (NULL != demoState) && (NULL != cardRegistry))
    {
        /* Check the status of the Discovery-Loop */
        ptx_RUL_Status_Params_t discoverStatus = {0};
        discoverStatus.Type = ptx_RUL_StatusType_Discover;
        status = ptx_RUL_GetStatusInfo(rul, &discoverStatus);
        
        if (PTX_STATUS_SUCCESS == status)
        {
            switch (discoverStatus.Info.Discover)
            {
                case ptx_RUL_StatusInfoDiscover_NoCard:
                {
                    /* No card found so far, just wait for a certain amount of time to unblock/off-load CPU / Thread etc. */
                    (void)ptx_RUL_Sleep(rul, PTX_DEMO_APP_NO_CARD_SLEEP_MS);
                    break;
                }

                case ptx_RUL_StatusInfoDiscover_Active:
                {
                    /*
                     * An NFC-device has been activated i.e. selected and is ready for (user) data exchange.
                     * Activation / Selection takes directly place if single device / card supporting only one protocol was detected.
                     */
                    *demoState = demoStateDataExchange;
                    break;
                }

                case ptx_RUL_StatusInfoDiscover_CardDetectionOngoing:
                case ptx_RUL_StatusInfoDiscover_CardsAvailable:
                case ptx_RUL_StatusInfoDiscover_MultiProtocol:
                    // not dealing with multiple cards, multiprotocol or card selection
                    *demoState = demoStateRestartDiscovery;
                    break;

                default:
                {
                    *demoState = demoStateExit;
                    lockSharedState();
                    sSharedState.ptxStatus = PTX_STATUS_E_GEN_INTERNAL;
                    unlockSharedState();
                    ptx_Utils_Print("ERROR [Unsupported discovery state detected]: %s (0x%04X)\n", getErrorDescription(PTX_STATUS_E_GEN_INTERNAL), PTX_STATUS_E_GEN_INTERNAL);
                    break;
                }
            }

            /* When writing and no card got activated within the polling-cycle limit, stop discovery. */
            if ((demoStateWaitForDiscovery == *demoState) || (demoStateRestartDiscovery == *demoState))
            {
                lockSharedState();
                if ((DiscoveryMode_NdefWrite == sSharedState.mode) && (g_get_monotonic_time() > sSharedState.writeDeadline))
                {
                    sSharedState.doStopDiscovery = true;
                    ptx_Utils_Print("No card found within %u polling cycles - stopping RF-Discovery.\n", NDEF_WRITE_MAX_POLLING_CYCLES);
                }
                unlockSharedState();
            }
        }
        else
        {
            *demoState = demoStateExit;
            lockSharedState();
            sSharedState.ptxStatus = status;
            unlockSharedState();
            ptx_Utils_Print("ERROR [RF-Discovery status check failed]: %s (0x%04X)\n", getErrorDescription(status), status);
        }
    }

    return status;
}

/* Writes the composed NDEF message to the activated card and stores the outcome in the shared state.
 * Write errors are reported to the UI only, so the returned status is always SUCCESS for valid parameters. */
static ptx_Status_t writeNdefMessage(ptx_RUL_t *rul, ptx_RUL_CardProtocolType_t protocol)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    NdefWriteResult result = NdefWriteResult_Failed;
    ptx_Status_t writeStatus = PTX_STATUS_SUCCESS;

    if (NULL != rul)
    {
        ptx_NDEF_t ndefComp = {0};
        ptx_NDEF_InitParams_t ndefInitParams = {0};
        ndefInitParams.rul           = rul;
        ndefInitParams.rx            = sNdefRxBuffer;
        ndefInitParams.rxLen         = (uint32_t)NDEF_BUFFER_SIZE;
        ndefInitParams.tx            = sNdefTxBuffer;
        ndefInitParams.txLen         = (uint32_t)NDEF_BUFFER_SIZE;
        ndefInitParams.workBuffer    = sNdefWorkBuffer;
        ndefInitParams.workBufferLen = (uint32_t)NDEF_BUFFER_SIZE;

        writeStatus = ptx_NDEF_Open(&ndefComp, &ndefInitParams);

        if (PTX_STATUS_SUCCESS == writeStatus)
        {
            /* Check if the activated Tag is NDEF-formatted (also required to set up the tag context for writing). */
            if (PTX_STATUS_SUCCESS != ptx_NDEF_CheckMessage(&ndefComp))
            {
                result = NdefWriteResult_NotNdef;
            }
            else
            {
                /* Write the NDEF message composed on the NDEF tab (local copy, the SDK may modify the buffer). */
                uint8_t msgBuffer[NDEF_BUFFER_SIZE];
                lockSharedState();
                uint32_t msgLen = sSharedState.writeMsgLen;
                memcpy(msgBuffer, sSharedState.writeMsgBuffer, msgLen);
                unlockSharedState();

                writeStatus = ptx_NDEF_WriteMessage(&ndefComp, msgBuffer, msgLen);

                if (PTX_STATUS_SUCCESS == writeStatus)
                {
                    result = NdefWriteResult_Success;
                    ptx_Utils_Print("NDEF message written successfully.\n");
                }
                else
                {
                    ptx_Utils_Print("Warning: NDEF write failed with status 0x%04X\n", writeStatus);
                }
            }

            ptx_Status_t closeStatus = ptx_NDEF_Close(&ndefComp);
            if (closeStatus != PTX_STATUS_SUCCESS) {
                ptx_Utils_Print("Warning: NDEF close failed with status 0x%04X\n", closeStatus);
            }
        }
        else
        {
            ptx_Utils_Print("Warning: NDEF open failed with status 0x%04X\n", writeStatus);
        }

        lockSharedState();
        sSharedState.writeResult = result;
        sSharedState.writeStatus = writeStatus;
        sSharedState.writeProtocol = protocol;
        unlockSharedState();

        status = PTX_STATUS_SUCCESS; /* Allow graceful degradation, error is reported in the UI */
    }

    return status;
}

static ptx_Status_t processStateDataExchange(ptx_RUL_t *rul, demoState_t *demoState, ptx_RUL_CardRegistry_t *cardRegistry)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    /* Initialize NDEF state */
    lockSharedState();
    sSharedState.isNdefMessage = false;
    sSharedState.isNdefMessageRead = false;
    DiscoveryMode mode = sSharedState.mode;
    unlockSharedState();

    if ((NULL != rul) && (NULL != demoState) && (NULL != cardRegistry))
    {
        status = PTX_STATUS_SUCCESS;
    }

    /* NDEF message is only read/written when discovery was started from the NDEF tab */
    if ((PTX_STATUS_SUCCESS == status) && (DiscoveryMode_NdefWrite == mode))
    {
        status = writeNdefMessage(rul, cardRegistry->ActiveProtocol);

        /* Writing is a one-shot operation - stop discovery afterwards. */
        lockSharedState();
        sSharedState.doStopDiscovery = true;
        unlockSharedState();
        *demoState = demoStateWaitForDiscovery;
    }

    if ((PTX_STATUS_SUCCESS == status) && (DiscoveryMode_NdefRead == mode))
    {
        ptx_NDEF_t ndefComp = {0};
        ptx_NDEF_InitParams_t ndefInitParams = {0};
        ndefInitParams.rul           = rul;
        ndefInitParams.rx            = sNdefRxBuffer;
        ndefInitParams.rxLen         = (uint32_t)NDEF_BUFFER_SIZE;
        ndefInitParams.tx            = sNdefTxBuffer;
        ndefInitParams.txLen         = (uint32_t)NDEF_BUFFER_SIZE;
        ndefInitParams.workBuffer    = sNdefWorkBuffer;
        ndefInitParams.workBufferLen = (uint32_t)NDEF_BUFFER_SIZE;

        status = ptx_NDEF_Open(&ndefComp, &ndefInitParams);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Check if the activated Tag is NDEF-formatted. */
            status = ptx_NDEF_CheckMessage(&ndefComp);

            if (PTX_STATUS_SUCCESS == status)
            {
                /* Tag is NDEF-formatted - let's try to read an NDEF message from it. */
                uint32_t bufferSize = (uint32_t)NDEF_BUFFER_SIZE;
                status = ptx_NDEF_ReadMessage(&ndefComp, sSharedState.ndefMsgBuffer, &bufferSize);

                lockSharedState();
                sSharedState.isNdefMessage = true;
                sSharedState.ndefMsgBufferSize = bufferSize;
                sSharedState.isNdefMessageRead = (status == PTX_STATUS_SUCCESS);
                unlockSharedState();

                if(status != PTX_STATUS_SUCCESS) {
                    ptx_Utils_Print("Warning: NDEF read failed with status 0x%04X\n", status);
                    status = PTX_STATUS_SUCCESS; /* Allow graceful degradation, not silent failure */
                }
            }
            else {
                /* Tag is not NDEF-formatted - no special handling needed */
                status = PTX_STATUS_SUCCESS; /* Allow graceful degradation */
            }

            ptx_Status_t closeStatus = ptx_NDEF_Close(&ndefComp);
            if (closeStatus != PTX_STATUS_SUCCESS) {
                ptx_Utils_Print("Warning: NDEF close failed with status 0x%04X\n", closeStatus);
            }
        }
    }

    if ((NULL != rul) && (NULL != demoState) && (NULL != cardRegistry) && (DiscoveryMode_NdefWrite != mode))
    {
        /* Check if card changed and update if needed */
        lockSharedState();
        ptx_RUL_CardParams_t activeCard = sSharedState.activeCard;
        unlockSharedState();

        if(0 != memcmp(&activeCard, cardRegistry->ActiveCard, sizeof(activeCard)))
        {
            lockSharedState();
            sSharedState.activeCard = *cardRegistry->ActiveCard;
            sSharedState.activeProtocol = cardRegistry->ActiveProtocol;
            unlockSharedState();
            g_idle_add((DiscoveryMode_NdefRead == mode) ? updateNdefInfo : updateCardInfo, NULL);
        }

        *demoState = demoStateRestartDiscovery;
    }

    return status;
}

static ptx_Status_t processStateRestartDiscovery(ptx_RUL_t *rul, demoState_t *demoState)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_RUL_DeactivateParams_t deactParams = {0};

    if ((NULL != rul) && (NULL != demoState))
    {
        /* Perform short reset of RF-Field and restart Polling-Loop again. */
        deactParams.Type = ptx_RUL_DeactivateType_Generic;
        deactParams.State = ptx_RUL_DeactivateState_DiscoveryRfReset;
        status = ptx_RUL_Deactivate(rul, &deactParams);

        if (PTX_STATUS_SUCCESS == status)
        {
            *demoState = demoStateWaitForDiscovery;
        }
        else
        {
            lockSharedState();
            sSharedState.ptxStatus = status;
            unlockSharedState();
            ptx_Utils_Print("ERROR [RF-Field reset failed]: %s (0x%04X)\n", getErrorDescription(status), status);
            *demoState = demoStateExit;
        }
    }

    return status;
}

static ptx_Status_t processStateDeactivateReader(ptx_RUL_t *rul, demoState_t *demoState)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_RUL_DeactivateParams_t deactParams = {0};

    if ((NULL != rul) && (NULL != demoState))
    {
        /* Generic deactivation -> switch RF-Field off and switch to IDLE-state. */
        deactParams.Type = ptx_RUL_DeactivateType_Generic;
        deactParams.State = ptx_RUL_DeactivateState_Idle;
        status = ptx_RUL_Deactivate(rul, &deactParams);

        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_Utils_Print("RF-Field switched off successfully.\n");
            lockSharedState();
            sSharedState.doStopDiscovery = false;
            sSharedState.doRunDiscovery = false;
            DiscoveryMode mode = sSharedState.mode;
            unlockSharedState();
            *demoState = demoStateStartDiscovery;

            /* Notify the UI only after the reader is idle, so a new discovery can be started right away. */
            if (DiscoveryMode_NdefWrite == mode)
            {
                g_idle_add(onNdefWriteDone, NULL);
            }
            else if (DiscoveryMode_NdefRead == mode)
            {
                g_idle_add(onNdefReadStopped, NULL);
            }
        }
        else
        {
            lockSharedState();
            sSharedState.ptxStatus = status;
            unlockSharedState();
            ptx_Utils_Print("ERROR [RF-Field deactivation failed]: %s (0x%04X)\n", getErrorDescription(status), status);
            *demoState = demoStateExit;
        }
    }

    return status;
}
