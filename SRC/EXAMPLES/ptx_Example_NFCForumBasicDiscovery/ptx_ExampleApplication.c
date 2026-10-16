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
                  Polls for RF-technologies A, B, F, V. On the Discovery tab, all cards in the
                  field (up to 5 per RF-technology) are displayed in a table. On the NDEF tab
                  (single card only), the NDEF message of an activated card is parsed into its
                  records, which are displayed decoded (URI, Text, Smart Poster, MIME, absolute
                  URI, external types) together with the raw message bytes.
                  On the NDEF tab, an NDEF message can be composed of standard record types (URI, Text,
                  Smart Poster, Phone, Email, SMS, Geo location, vCard, MIME, Wi-Fi, Android app) - see
                  ptx_ExampleNdefRecords.c. The WRITE button starts discovery for max. 10 polling cycles,
                  writes the message to the first activated card, then stops discovery.
                  On the Polling tab, the discovery settings (idle time, LPCD, per-technology
                  enable / poll frequency / device limit, Type-F bit rate) can be changed.
                  When the app is launched, the PTX is initialized and discovery can be
                  started/stopped by clicking the START/STOP button on the Discovery tab
                  (RESET clears the displayed card information and restarts discovery), or
                  the READ/STOP buttons on the NDEF tab (NDEF read, CLEAR clears the output). There's a menu offering
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

/* Delay/Sleep between checking RF-Discovery state and Discovery-procedure is ongoing (resolving multiple cards). */
static const uint32_t PTX_DEMO_APP_CARD_RESOLVE_SLEEP_MS = 5u;

/* Default and max. number of cards resolved per RF-technology (A, B, F, V) and discovery cycle (5 as in the SDK's example),
 * the limit can be lowered on the Polling tab. */
#define PTX_DEMO_APP_DEVICE_LIMIT_PER_TECH  5u

/* Max. number of cards resolved per discovery cycle. The SDK's card registry holds up to PTX_RUL_MAX_SUPPORTED_DEVICES cards. */
#define PTX_DEMO_APP_MAX_CARDS              (4u * PTX_DEMO_APP_DEVICE_LIMIT_PER_TECH)
G_STATIC_ASSERT(PTX_DEMO_APP_MAX_CARDS <= PTX_RUL_MAX_SUPPORTED_DEVICES);

/* Max. number of cards listed on the Discovery tab - cards are collected until discovery is restarted. */
#define PTX_DEMO_APP_MAX_LISTED_CARDS       50u

/* Max. length of a card ID (NFCID1, NFCID0, NFCID2, UID). */
#define CARD_ID_MAX_LEN                     PTX_RUL_NFCID1_MAX_LEN
G_STATIC_ASSERT((CARD_ID_MAX_LEN >= PTX_RUL_NFCID0_LEN) && (CARD_ID_MAX_LEN >= PTX_RUL_NFCID2_LEN) && (CARD_ID_MAX_LEN >= PTX_RUL_UID_LEN));

/* Default idle time between two consecutive polling-loop cycles, changeable on the Polling tab. */
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

/* RF technologies configurable on the Polling tab */
enum { POLL_TECH_A, POLL_TECH_B, POLL_TECH_F, POLL_TECH_V, POLL_TECH_COUNT };

/* LPCD modes of the Polling tab (index of the drop-down) */
enum { POLL_LPCD_DISABLED, POLL_LPCD_EVERY_CYCLE, POLL_LPCD_EVERY_NTH_CYCLE };

/* Discovery settings of the Polling tab */
typedef struct {
    uint32_t idleTime;                         /* Idle time between polling cycles (ms) */
    uint8_t lpcdMode;                          /* POLL_LPCD_* */
    uint8_t lpcdCycles;                        /* Regular polling every n-th cycle (2-255), POLL_LPCD_EVERY_NTH_CYCLE only */
    struct {
        gboolean enabled;                      /* Poll for this technology */
        uint8_t frequency;                     /* Poll every n-th cycle (1-10) */
        uint8_t devices;                       /* Max. devices resolved per cycle (1-PTX_DEMO_APP_DEVICE_LIMIT_PER_TECH) */
    } tech[POLL_TECH_COUNT];
    gboolean f424;                             /* Type-F at 424 kbit/s instead of 212 kbit/s */
} PollingValues;

/* Card listed on the Discovery tab */
typedef struct {
    ptx_RUL_CardParams_t card;                 /* Latest card parameters */
    ptx_RUL_CardProtocolType_t activeProtocol; /* Protocol of the last activation, Undefined if never activated */
    uint32_t detections;                       /* Number of discovery cycles the card was found in */
} ListedCard;

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

    /* Card state of the NDEF tab (single card) - the Discovery tab uses listedCards */
    ptx_RUL_CardParams_t activeCard;           /* Was sActiveCard (volatile) */
    ptx_RUL_CardProtocolType_t activeProtocol; /* Was sActiveProtocol (volatile) */

    /* Cards detected since discovery was started (Discovery tab) */
    ListedCard listedCards[PTX_DEMO_APP_MAX_LISTED_CARDS]; /* In order of first detection */
    uint32_t listedCardsCnt;                   /* Number of listed cards */
    gboolean listFull;                         /* A card was not listed because the list is full */
    gboolean cardUpdatePending;                /* UI update of the card table is queued */

    /* Discovery settings (Polling tab), used when discovery is started */
    ptx_RUL_DiscoverConfig_t pollConfig;
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
    GtkLabel *cardStatusLabel;                 /* Discovery tab: status and number of cards */
    GtkGrid *cardTable;                        /* Discovery tab: one row per card */
    guint cardRowsShown;                       /* Number of card rows in the table */
    GtkButton *startStopButton;
    GtkWidget *stackSwitcher;
    GtkButton *ndefReadButton;
    GtkButton *ndefStopButton;
    GtkButton *ndefReadClearButton;
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
    GtkSpinButton *pollIdleTime;               /* Polling tab widgets */
    GtkDropDown *pollLpcdMode;
    GtkSpinButton *pollLpcdCycles;
    GtkCheckButton *pollEnabled[POLL_TECH_COUNT];
    GtkSpinButton *pollFrequency[POLL_TECH_COUNT];
    GtkSpinButton *pollDevices[POLL_TECH_COUNT];
    GtkCheckButton *pollF212;
    GtkCheckButton *pollF424;
    GtkLabel *pollingNote;
    gboolean pollingUiLoading;                 /* Widgets are being set, ignore their change signals */
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
static void updateNdefButtons(AppState *state);
typedef GtkWindow* (*LoadWindowFunc)(GtkApplication *app, AppState *state, GtkWindow *parent);
static void presentWindow(GtkWindow **window, LoadWindowFunc loader, GtkApplication *app, AppState *state);
static ptx_Status_t runDemo();
static ptx_Status_t processStateStartDiscovery(ptx_RUL_t *rul, demoState_t *demoState);
static ptx_Status_t processStateWaitForDiscovery(ptx_RUL_t *rul, demoState_t *demoState, ptx_RUL_CardRegistry_t *cardRegistry);
static void publishCardList(const ptx_RUL_CardRegistry_t *cardRegistry, gboolean activated);
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
    char content[128];
    snprintf(content, sizeof(content), "ERROR 0x%04X - %s\nPlease restart the application.", errorData->errorCode, description);

    gtk_label_set_text(state->cardStatusLabel, content);
    appendToTextBuffer(gtk_text_view_get_buffer(state->ndefTextView),
                       "\nERROR 0x%04X - %s\nPlease restart the application.\n",
                       errorData->errorCode, description);

    gtk_widget_set_sensitive(GTK_WIDGET(state->startStopButton), FALSE);
    state->ptxReady = FALSE;
    updateNdefButtons(state);
    gtk_widget_set_sensitive(state->stackSwitcher, TRUE);

    gtk_label_set_text(state->ndefWriteStatusLabel, content);

    g_free(errorData);
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

static const char* getTechString(ptx_RUL_CardRfTechType_t tech)
{
    switch(tech) {
        case ptx_RUL_CardRfTechType_A: return "Type-A";
        case ptx_RUL_CardRfTechType_B: return "Type-B";
        case ptx_RUL_CardRfTechType_F: return "Type-F";
        case ptx_RUL_CardRfTechType_V: return "Type-V";
        default: return "Unsupported";
    }
}

/* Gets the card ID (CARD_ID_MAX_LEN bytes buffer), returns FALSE for unsupported RF technologies. */
static gboolean getCardId(const ptx_RUL_CardParams_t *card, uint8_t *id, uint32_t *len)
{
    switch(card->TechType)
    {
        case ptx_RUL_CardRfTechType_A:
            *len = MIN(card->TechParam.TypeA.NfcId1Len, PTX_RUL_NFCID1_MAX_LEN);
            memcpy(id, card->TechParam.TypeA.NfcId1, *len);
            return TRUE;

        case ptx_RUL_CardRfTechType_B:
            *len = PTX_RUL_NFCID0_LEN;
            memcpy(id, &card->TechParam.TypeB.SensBRes[PTX_RUL_SENSB_RES_NFCID0_OFFSET], *len);
            return TRUE;

        case ptx_RUL_CardRfTechType_F:
            *len = PTX_RUL_NFCID2_LEN;
            memcpy(id, &card->TechParam.TypeF.SensFRes[PTX_RUL_SENSF_RES_NFCID2_OFFSET], *len);
            return TRUE;

        case ptx_RUL_CardRfTechType_V:
            /* UID in INVENTORY_RES (after flags and DSFID) is transmitted LSB first - store it MSB first */
            *len = PTX_RUL_UID_LEN;
            for (uint32_t i = 0; i < PTX_RUL_UID_LEN; ++i) {
                id[i] = card->TechParam.TypeV.InventoryRes[2u + PTX_RUL_UID_LEN - 1u - i];
            }
            return TRUE;

        default:
            *len = 0;
            return FALSE;
    }
}

/* Appends the card ID as hex digits, returns FALSE (nothing appended) for unsupported RF technologies. */
static gboolean appendCardId(GString *out, const ptx_RUL_CardParams_t *card)
{
    uint8_t id[CARD_ID_MAX_LEN];
    uint32_t len = 0;

    if (!getCardId(card, id, &len))
        return FALSE;

    appendHexString(out, id, len);
    return TRUE;
}

/* TRUE if both card parameters belong to the same card (same RF technology and ID). */
static gboolean isSameCard(const ptx_RUL_CardParams_t *a, const ptx_RUL_CardParams_t *b)
{
    uint8_t idA[CARD_ID_MAX_LEN];
    uint8_t idB[CARD_ID_MAX_LEN];
    uint32_t lenA = 0;
    uint32_t lenB = 0;

    return (a->TechType == b->TechType) &&
           getCardId(a, idA, &lenA) && getCardId(b, idB, &lenB) &&
           (lenA == lenB) && (0 == memcmp(idA, idB, lenA));
}

/*
 * Appends the RF-technology specific data of a card that isn't part of the card ID:
 * - Type-A: ATQA (SENS_RES) and SAK (SEL_RES)
 * - Type-B: Application data and Protocol info of SENSB_RES
 * - Type-F: PMm (PAD) and, if present, System code (RD) of SENSF_RES
 * - Type-V: DSFID of INVENTORY_RES
 */
static void appendCardDetails(GString *out, const ptx_RUL_CardParams_t *card)
{
    switch(card->TechType)
    {
        case ptx_RUL_CardRfTechType_A:
            g_string_append(out, "ATQA ");
            appendHexString(out, card->TechParam.TypeA.SensRes, sizeof(card->TechParam.TypeA.SensRes));
            g_string_append_printf(out, ", SAK %02X", card->TechParam.TypeA.SelRes);
            break;

        case ptx_RUL_CardRfTechType_B:
            g_string_append(out, "App data ");
            appendHexString(out, &card->TechParam.TypeB.SensBRes[PTX_RUL_SENSB_RES_AFI_OFFSET], PTX_RUL_APPDATA_LEN);
            g_string_append(out, ", Protocol info ");
            appendHexString(out, &card->TechParam.TypeB.SensBRes[PTX_RUL_SENSB_BIT_RATE_CAPABILITY_OFFSET], PTX_RUL_PROTINFO_LEN);
            break;

        case ptx_RUL_CardRfTechType_F:
        {
            /* SENSF_RES: length, response code, NFCID2, PAD (PMm, 8 bytes), optional RD (System code) */
            const uint8_t *sensFRes = card->TechParam.TypeF.SensFRes;
            g_string_append(out, "PMm ");
            appendHexString(out, &sensFRes[PTX_RUL_SENSF_RES_PAD0_OFFSET], PTX_RUL_SENSF_RES_RD_OFFSET - PTX_RUL_SENSF_RES_PAD0_OFFSET);
            if (sensFRes[0] >= (PTX_RUL_SENSF_RES_RD_OFFSET + PTX_RUL_RD_LEN))
            {
                g_string_append(out, ", System code ");
                appendHexString(out, &sensFRes[PTX_RUL_SENSF_RES_RD_OFFSET], PTX_RUL_RD_LEN);
            }
            break;
        }

        case ptx_RUL_CardRfTechType_V:
            g_string_append_printf(out, "DSFID %02X", card->TechParam.TypeV.InventoryRes[PTX_RUL_INVENTORYRES_DSFID_OFFSET]);
            break;

        default:
            break;
    }
}

/* Appends the complete RF-technology specific response of a card (shown as tooltip of the details). */
static void appendCardRawResponse(GString *out, const ptx_RUL_CardParams_t *card)
{
    switch(card->TechType)
    {
        case ptx_RUL_CardRfTechType_A:
            g_string_append(out, "SENS_RES ");
            appendHexString(out, card->TechParam.TypeA.SensRes, sizeof(card->TechParam.TypeA.SensRes));
            g_string_append_printf(out, ", SEL_RES %02X", card->TechParam.TypeA.SelRes);
            break;

        case ptx_RUL_CardRfTechType_B:
            g_string_append(out, "SENSB_RES ");
            appendHexString(out, card->TechParam.TypeB.SensBRes, sizeof(card->TechParam.TypeB.SensBRes));
            break;

        case ptx_RUL_CardRfTechType_F:
            g_string_append(out, "SENSF_RES ");
            appendHexString(out, card->TechParam.TypeF.SensFRes, sizeof(card->TechParam.TypeF.SensFRes));
            break;

        case ptx_RUL_CardRfTechType_V:
            g_string_append(out, "INVENTORY_RES ");
            appendHexString(out, card->TechParam.TypeV.InventoryRes, sizeof(card->TechParam.TypeV.InventoryRes));
            break;

        default:
            break;
    }
}

/* Returns the bit rate in kbit/s, 0 for unknown values. */
static uint32_t getBitRateKbps(ptx_RUL_RfBitRate_t bitRate)
{
    switch(bitRate) {
        case ptx_RUL_RfBitRate_106: return 106u;
        case ptx_RUL_RfBitRate_212: return 212u;
        case ptx_RUL_RfBitRate_424: return 424u;
        case ptx_RUL_RfBitRate_848: return 848u;
        case ptx_RUL_RfBitRate_26:  return 26u;
        default: return 0u;
    }
}

/*
 * Appends the nominal RF bit rate of a card, e.g. "106 kbit/s" or "TX 212 / RX 106 kbit/s" if the directions differ.
 * Discovered cards report the default bit rate of their RF technology, higher bit rates are only reported after an
 * activation negotiating them.
 */
static void appendCardBitRate(GString *out, const ptx_RUL_CardParams_t *card)
{
    uint32_t tx = 0;
    uint32_t rx = 0;

    switch(card->TechType)
    {
        case ptx_RUL_CardRfTechType_A:
            tx = getBitRateKbps(card->TechParam.TypeA.TxBitRate);
            rx = getBitRateKbps(card->TechParam.TypeA.RxBitRate);
            break;

        case ptx_RUL_CardRfTechType_B:
            tx = getBitRateKbps(card->TechParam.TypeB.TxBitRate);
            rx = getBitRateKbps(card->TechParam.TypeB.RxBitRate);
            break;

        case ptx_RUL_CardRfTechType_F:
            tx = rx = getBitRateKbps(card->TechParam.TypeF.BitRate);
            break;

        case ptx_RUL_CardRfTechType_V:
            tx = rx = getBitRateKbps(card->TechParam.TypeV.BitRate);
            break;

        default:
            break;
    }

    if ((0u == tx) || (0u == rx))
        g_string_append(out, "-");
    else if (tx == rx)
        g_string_append_printf(out, "%u kbit/s", tx);
    else
        g_string_append_printf(out, "TX %u / RX %u kbit/s", tx, rx);
}

/* Appends the protocols supported by a (not activated) card, e.g. "T4T (ISO-DEP), P2P (NFC-DEP)". */
static void appendSupportedProtocols(GString *out, uint8_t protocols)
{
    static const struct { uint8_t mask; ptx_RUL_CardProtocolType_t type; } PROTOCOLS[] = {
        { ptx_RUL_SupportedProtocols_T2T,    ptx_RUL_CardProtocolType_T2T },
        { ptx_RUL_SupportedProtocols_T3T,    ptx_RUL_CardProtocolType_T3T },
        { ptx_RUL_SupportedProtocols_T4T,    ptx_RUL_CardProtocolType_T4T },
        { ptx_RUL_SupportedProtocols_NfcDep, ptx_RUL_CardProtocolType_NfcDep },
        { ptx_RUL_SupportedProtocols_T5T,    ptx_RUL_CardProtocolType_T5T },
    };
    gsize start = out->len;

    for (guint i = 0; i < G_N_ELEMENTS(PROTOCOLS); ++i) {
        if (0u != (protocols & PROTOCOLS[i].mask)) {
            g_string_append_printf(out, "%s%s", (out->len > start) ? ", " : "", getProtocolShortString(PROTOCOLS[i].type));
        }
    }

    if (out->len == start) {
        g_string_append(out, "Unknown");
    }
}

/* Appends a one-line card summary: RF technology, protocol and card ID. */
static void appendCardSummary(GString *out, const ptx_RUL_CardParams_t *card, ptx_RUL_CardProtocolType_t protocol)
{
    g_string_append_printf(out, "Card: RF-Tech %s, Protocol %s", getTechString(card->TechType), getProtocolShortString(protocol));

    GString *id = g_string_new(NULL);
    if (appendCardId(id, card))
    {
        g_string_append_printf(out, ", ID %s", id->str);
    }
    g_string_free(id, TRUE);
    g_string_append_c(out, '\n');
}

static void clearCardTable(AppState *state)
{
    GtkWidget *child;
    while (NULL != (child = gtk_widget_get_first_child(GTK_WIDGET(state->cardTable))))
    {
        gtk_grid_remove(state->cardTable, child);
    }
    state->cardRowsShown = 0;
}

/* Sets the tooltip of a table cell, only if it changed. */
static void setCardTableCellTooltip(GtkGrid *table, int column, int row, const char *text)
{
    GtkWidget *cell = gtk_grid_get_child_at(table, column, row);
    if ((NULL != cell) && (0 != g_strcmp0(gtk_widget_get_tooltip_text(cell), text)))
    {
        gtk_widget_set_tooltip_text(cell, text);
    }
}

/* Sets the text of a table cell, only if it changed (keeps a text selection in unchanged cells). */
static void setCardTableCellText(GtkGrid *table, int column, int row, const char *text)
{
    GtkWidget *cell = gtk_grid_get_child_at(table, column, row);
    if (GTK_IS_LABEL(cell) && (0 != g_strcmp0(gtk_label_get_text(GTK_LABEL(cell)), text)))
    {
        gtk_label_set_text(GTK_LABEL(cell), text);
    }
}

static void attachCardTableCell(GtkGrid *table, int column, int row, const char *text, gboolean header, gboolean monospace)
{
    GtkWidget *label = gtk_label_new(NULL);

    if (header)
    {
        gchar *markup = g_markup_printf_escaped("<b>%s</b>", text);
        gtk_label_set_markup(GTK_LABEL(label), markup);
        g_free(markup);
    }
    else
    {
        gtk_label_set_text(GTK_LABEL(label), text);
    }

    gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
    if (monospace)
    {
        gtk_widget_add_css_class(label, "monospace");
        gtk_label_set_selectable(GTK_LABEL(label), TRUE);
    }
    gtk_grid_attach(table, label, column, row, 1, 1);
}

/*
 * Shows all cards detected since discovery was started as a table: number, RF technology, protocol, nominal bit rate,
 * ID, number of detections and RF-technology specific details. Rows are only appended, existing rows are updated in place.
 */
static gboolean updateCardInfo()
{
    enum { COL_NUMBER, COL_TECH, COL_PROTOCOL, COL_BITRATE, COL_ID, COL_DETECTIONS, COL_DETAILS, COL_COUNT };
    static const char * const HEADERS[COL_COUNT] = { "#", "RF-Tech", "Protocol", "Bit rate", "ID", "Detections", "Details" };
    static ListedCard cards[PTX_DEMO_APP_MAX_LISTED_CARDS];

    lockSharedState();
    gboolean isRunning = sSharedState.doRunDiscovery;
    uint32_t cardsCnt = MIN(sSharedState.listedCardsCnt, PTX_DEMO_APP_MAX_LISTED_CARDS);
    gboolean listFull = sSharedState.listFull;
    memcpy(cards, sSharedState.listedCards, cardsCnt * sizeof(cards[0]));
    sSharedState.cardUpdatePending = FALSE;
    unlockSharedState();

    if(!isRunning)
        return FALSE;

    GtkApplication *app = GTK_APPLICATION(g_application_get_default());
    AppState *state = g_object_get_data(G_OBJECT(app), "app-state");

    if (0u == cardsCnt)
    {
        gtk_label_set_text(state->cardStatusLabel, "Waiting for cards ...");
        return FALSE;
    }

    if (NULL == gtk_grid_get_child_at(state->cardTable, COL_NUMBER, 0))
    {
        for (guint c = 0; c < COL_COUNT; ++c)
        {
            attachCardTableCell(state->cardTable, (int)c, 0, HEADERS[c], TRUE, FALSE);
        }
    }

    GString *text = g_string_new(NULL);
    for (uint32_t i = 0; i < cardsCnt; ++i)
    {
        int row = (int)i + 1;
        const ListedCard *entry = &cards[i];

        /* New card: append a row, the static cells (number, RF technology, ID) never change */
        if (i >= state->cardRowsShown)
        {
            g_string_printf(text, "%u", i + 1u);
            attachCardTableCell(state->cardTable, COL_NUMBER, row, text->str, FALSE, FALSE);
            attachCardTableCell(state->cardTable, COL_TECH, row, getTechString(entry->card.TechType), FALSE, FALSE);
            attachCardTableCell(state->cardTable, COL_PROTOCOL, row, "", FALSE, FALSE);
            attachCardTableCell(state->cardTable, COL_BITRATE, row, "", FALSE, FALSE);

            g_string_truncate(text, 0);
            if (!appendCardId(text, &entry->card))
                g_string_append(text, "-");
            attachCardTableCell(state->cardTable, COL_ID, row, text->str, FALSE, TRUE);
            attachCardTableCell(state->cardTable, COL_DETECTIONS, row, "", FALSE, FALSE);
            attachCardTableCell(state->cardTable, COL_DETAILS, row, "", FALSE, TRUE);
        }

        g_string_truncate(text, 0);
        if (ptx_RUL_CardProtocolType_Undefined != entry->activeProtocol)
            g_string_append(text, getProtocolShortString(entry->activeProtocol));
        else
            appendSupportedProtocols(text, entry->card.Protocols);
        setCardTableCellText(state->cardTable, COL_PROTOCOL, row, text->str);

        g_string_truncate(text, 0);
        appendCardBitRate(text, &entry->card);
        setCardTableCellText(state->cardTable, COL_BITRATE, row, text->str);

        g_string_printf(text, "%u", entry->detections);
        setCardTableCellText(state->cardTable, COL_DETECTIONS, row, text->str);

        g_string_truncate(text, 0);
        appendCardDetails(text, &entry->card);
        setCardTableCellText(state->cardTable, COL_DETAILS, row, text->str);

        g_string_truncate(text, 0);
        appendCardRawResponse(text, &entry->card);
        setCardTableCellTooltip(state->cardTable, COL_DETAILS, row, text->str);
    }
    state->cardRowsShown = MAX(state->cardRowsShown, cardsCnt);

    g_string_printf(text, "%u card%s detected since start", cardsCnt, (1u == cardsCnt) ? "" : "s");
    if (listFull)
        g_string_append_printf(text, " - list full (max. %u cards)", PTX_DEMO_APP_MAX_LISTED_CARDS);
    gtk_label_set_text(state->cardStatusLabel, text->str);
    g_string_free(text, TRUE);

    return FALSE;
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

    lockSharedState();
    ptx_Status_t status = sSharedState.ptxStatus;
    unlockSharedState();

    if(status == PTX_STATUS_SUCCESS)
    {
        gtk_label_set_text(state->cardStatusLabel, "System initialized. Press START to begin discovery!");
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
        gtk_label_set_text(state->cardStatusLabel, content);
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
    sSharedState.listedCardsCnt = 0;
    sSharedState.listFull = FALSE;
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
        clearCardTable(state);
        gtk_label_set_text(state->cardStatusLabel, "Waiting for cards ...");
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
 * - CLEAR: no NDEF read or write running.
 * - STOP:  NDEF read running and not being stopped already.
 * - WRITE: no NDEF read or write running and the message isn't empty.
 */
static void updateNdefButtons(AppState *state)
{
    gboolean idle = state->ptxReady && !state->ndefReadRunning && !state->ndefWriteRunning;

    gtk_widget_set_sensitive(GTK_WIDGET(state->ndefReadButton), idle);
    gtk_widget_set_sensitive(GTK_WIDGET(state->ndefReadClearButton), idle);
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

static void onNdefReadClear(GtkWidget *widget, gpointer userData)
{
    (void)widget;
    AppState *state = g_object_get_data(G_OBJECT(userData), "app-state");
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(state->ndefTextView), "", -1);
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

/* Setup function for about window: show the application version in the window and header bar title */
static void setupAboutWindow(GtkBuilder *builder, AppState *state)
{
    (void)state;

    const char *titleWithVersion = "PTX205R Discovery (" PTX_VERSION_STRING ")";

    GObject *window = gtk_builder_get_object(builder, "aboutWindow");
    if (GTK_IS_WINDOW(window)) {
        gtk_window_set_title(GTK_WINDOW(window), titleWithVersion);
    }

    GObject *headerbar = gtk_builder_get_object(builder, "headerbar");
    if (GTK_IS_HEADER_BAR(headerbar)) {
        GtkWidget *titleWidget = gtk_header_bar_get_title_widget(GTK_HEADER_BAR(headerbar));
        if (GTK_IS_LABEL(titleWidget)) {
            gtk_label_set_text(GTK_LABEL(titleWidget), titleWithVersion);
        }
    }
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
    clearCardTable(state);

    lockSharedState();
    gboolean isRunning = sSharedState.doRunDiscovery;
    if(isRunning)
        sSharedState.doReset = true;
    sSharedState.activeCard = (ptx_RUL_CardParams_t){0};
    sSharedState.listedCardsCnt = 0;
    sSharedState.listFull = FALSE;
    unlockSharedState();

    gtk_label_set_text(state->cardStatusLabel, isRunning ? "Waiting for cards ..." : "");
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

/* Default discovery settings: all technologies polled in every cycle, LPCD disabled. */
static void getDefaultPollingValues(PollingValues *values)
{
    memset(values, 0, sizeof(*values));
    values->idleTime = PTX_DEMO_APP_DISCOVERY_IDLE_TIME_MS;
    values->lpcdMode = POLL_LPCD_DISABLED;
    values->lpcdCycles = ptx_RUL_PollLPCD_HybridMin;
    for (guint i = 0; i < POLL_TECH_COUNT; ++i)
    {
        values->tech[i].enabled = TRUE;
        values->tech[i].frequency = ptx_RUL_PollFreq_EveryCycle;
        values->tech[i].devices = PTX_DEMO_APP_DEVICE_LIMIT_PER_TECH;
    }
    values->f424 = FALSE;
}

/* Builds the RUL discovery configuration from the Polling tab settings. */
static void buildPollConfig(const PollingValues *values, ptx_RUL_DiscoverConfig_t *cfg)
{
    memset(cfg, 0, sizeof(*cfg));

    /* Fixed settings: RF-Discovery / Polling Loop in NFC Forum Poll-Mode, Listen-mode (Card Emulation) not used. */
    cfg->PollMode = ptx_RUL_PollMode_NfcForum;
    cfg->EnablePolling = 1u;
#if !defined(PTX_DISABLE_LISTEN_MODE)
    cfg->ListenMode = 0;
#endif /* PTX_DISABLE_LISTEN_MODE */

    /* System pauses / idles after every full polling loop-cycle. */
    cfg->IdleTime = values->idleTime;

    /* Low-power-card-detection: 0 = disabled, 1 = every cycle, 2-255 = regular polling in every n-th cycle only. */
    switch (values->lpcdMode)
    {
        case POLL_LPCD_EVERY_CYCLE:     cfg->EnableLPCD = ptx_RUL_PollLPCD_Enabled; break;
        case POLL_LPCD_EVERY_NTH_CYCLE: cfg->EnableLPCD = CLAMP(values->lpcdCycles, ptx_RUL_PollLPCD_HybridMin, ptx_RUL_PollLPCD_HybridMax); break;
        default:                        cfg->EnableLPCD = ptx_RUL_PollLPCD_Disabled; break;
    }

    /*
     * Per technology: poll frequency 1 = every cycle, 2-10 = every n-th cycle (0 = don't poll), and the max. number
     * of devices / cards resolved per cycle (application can define max. value).
     */
    #define POLL_FREQUENCY(i) ((values->tech[i].enabled) ? (uint8_t)CLAMP(values->tech[i].frequency, ptx_RUL_PollFreq_EveryCycle, ptx_RUL_PollFreq_CycleMax) : (uint8_t)ptx_RUL_PollFreq_Disabled)
    #define POLL_DEVICES(i)   ((uint8_t)CLAMP(values->tech[i].devices, 1u, PTX_DEMO_APP_DEVICE_LIMIT_PER_TECH))

    cfg->PollTypeA = values->tech[POLL_TECH_A].enabled ? 1u : 0u;
    cfg->PollTypeAPollFrequency = POLL_FREQUENCY(POLL_TECH_A);
    cfg->PollTypeADeviceLimit = POLL_DEVICES(POLL_TECH_A);

    cfg->PollTypeB = values->tech[POLL_TECH_B].enabled ? 1u : 0u;
    cfg->PollTypeBPollFrequency = POLL_FREQUENCY(POLL_TECH_B);
    cfg->PollTypeBDeviceLimit = POLL_DEVICES(POLL_TECH_B);

    /* Type-F is polled either at 212 or at 424 kbit/s */
    cfg->PollTypeF212 = (values->tech[POLL_TECH_F].enabled && !values->f424) ? 1u : 0u;
    cfg->PollTypeF424 = (values->tech[POLL_TECH_F].enabled && values->f424) ? 1u : 0u;
    cfg->PollTypeFPollFrequency = POLL_FREQUENCY(POLL_TECH_F);
    cfg->PollTypeFDeviceLimit = POLL_DEVICES(POLL_TECH_F);

    cfg->PollTypeV = values->tech[POLL_TECH_V].enabled ? 1u : 0u;
    cfg->PollTypeVPollFrequency = POLL_FREQUENCY(POLL_TECH_V);
    cfg->PollTypeVDeviceLimit = POLL_DEVICES(POLL_TECH_V);

    #undef POLL_FREQUENCY
    #undef POLL_DEVICES
}

static void readPollingUi(AppState *state, PollingValues *values)
{
    values->idleTime = (uint32_t)gtk_spin_button_get_value_as_int(state->pollIdleTime);
    values->lpcdMode = (uint8_t)MIN(gtk_drop_down_get_selected(state->pollLpcdMode), (guint)POLL_LPCD_EVERY_NTH_CYCLE);
    values->lpcdCycles = (uint8_t)gtk_spin_button_get_value_as_int(state->pollLpcdCycles);
    for (guint i = 0; i < POLL_TECH_COUNT; ++i)
    {
        values->tech[i].enabled = gtk_check_button_get_active(state->pollEnabled[i]);
        values->tech[i].frequency = (uint8_t)gtk_spin_button_get_value_as_int(state->pollFrequency[i]);
        values->tech[i].devices = (uint8_t)gtk_spin_button_get_value_as_int(state->pollDevices[i]);
    }
    values->f424 = gtk_check_button_get_active(state->pollF424);
}

/*
 * Applies the Polling tab to the discovery configuration and updates the widgets:
 * - frequency, devices and bit rate of a disabled technology are greyed out,
 * - at least one technology stays enabled,
 * - the LPCD cycles are only shown for "Use in every n-th cycle",
 * - LPCD requires an idle time of at least 1 ms.
 */
static void applyPollingUi(AppState *state, GtkWidget *changed)
{
    if (state->pollingUiLoading)
        return;

    PollingValues values;
    const char *note = "Changes apply when discovery is started next time.";

    state->pollingUiLoading = TRUE;
    readPollingUi(state, &values);

    guint enabledCount = 0;
    for (guint i = 0; i < POLL_TECH_COUNT; ++i)
    {
        if (values.tech[i].enabled)
            enabledCount++;
    }
    if ((0u == enabledCount) && GTK_IS_CHECK_BUTTON(changed))
    {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(changed), TRUE);
        readPollingUi(state, &values);
        note = "At least one technology must be enabled.";
    }

    if (POLL_LPCD_DISABLED != values.lpcdMode)
    {
        gtk_spin_button_set_range(state->pollIdleTime, 1, 10000);
        readPollingUi(state, &values);
        if (0 == g_strcmp0(note, "Changes apply when discovery is started next time."))
            note = "LPCD requires an idle time of at least 1 ms. Changes apply when discovery is started next time.";
    }
    else
    {
        gtk_spin_button_set_range(state->pollIdleTime, 0, 10000);
    }
    gtk_widget_set_visible(GTK_WIDGET(state->pollLpcdCycles), POLL_LPCD_EVERY_NTH_CYCLE == values.lpcdMode);

    for (guint i = 0; i < POLL_TECH_COUNT; ++i)
    {
        gtk_widget_set_sensitive(GTK_WIDGET(state->pollFrequency[i]), values.tech[i].enabled);
        gtk_widget_set_sensitive(GTK_WIDGET(state->pollDevices[i]), values.tech[i].enabled);
    }
    gtk_widget_set_sensitive(GTK_WIDGET(state->pollF212), values.tech[POLL_TECH_F].enabled);
    gtk_widget_set_sensitive(GTK_WIDGET(state->pollF424), values.tech[POLL_TECH_F].enabled);
    gtk_label_set_text(state->pollingNote, note);
    state->pollingUiLoading = FALSE;

    ptx_RUL_DiscoverConfig_t cfg;
    buildPollConfig(&values, &cfg);
    lockSharedState();
    sSharedState.pollConfig = cfg;
    unlockSharedState();
}

static void loadPollingUi(AppState *state, const PollingValues *values)
{
    state->pollingUiLoading = TRUE;
    gtk_spin_button_set_range(state->pollIdleTime, 0, 10000);
    gtk_spin_button_set_value(state->pollIdleTime, values->idleTime);
    gtk_drop_down_set_selected(state->pollLpcdMode, values->lpcdMode);
    gtk_spin_button_set_value(state->pollLpcdCycles, values->lpcdCycles);
    for (guint i = 0; i < POLL_TECH_COUNT; ++i)
    {
        gtk_check_button_set_active(state->pollEnabled[i], values->tech[i].enabled);
        gtk_spin_button_set_value(state->pollFrequency[i], values->tech[i].frequency);
        gtk_spin_button_set_value(state->pollDevices[i], values->tech[i].devices);
    }
    gtk_check_button_set_active(values->f424 ? state->pollF424 : state->pollF212, TRUE);
    state->pollingUiLoading = FALSE;

    applyPollingUi(state, NULL);
}

/* "toggled" of check buttons and "value-changed" of spin buttons */
static void onPollingChanged(GtkWidget *widget, gpointer userData)
{
    AppState *state = g_object_get_data(G_OBJECT(userData), "app-state");
    applyPollingUi(state, widget);
}

/* "notify::selected" of the LPCD drop-down */
static void onPollingLpcdChanged(GObject *object, GParamSpec *pspec, gpointer userData)
{
    (void)pspec;
    AppState *state = g_object_get_data(G_OBJECT(userData), "app-state");
    applyPollingUi(state, GTK_WIDGET(object));
}

static void onPollingRestoreDefaults(GtkWidget *widget, gpointer userData)
{
    (void)widget;
    AppState *state = g_object_get_data(G_OBJECT(userData), "app-state");
    PollingValues values;
    getDefaultPollingValues(&values);
    loadPollingUi(state, &values);
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

    obj = gtk_builder_get_object(windowBuilder, "cardStatus");
    if(obj == NULL) {
        g_warning("Failed to load 'cardStatus' from window.ui - cannot display card information");
        g_object_unref(windowBuilder);
        return;
    }
    state->cardStatusLabel = GTK_LABEL(obj);

    obj = gtk_builder_get_object(windowBuilder, "cardTable");
    if(obj == NULL) {
        g_warning("Failed to load 'cardTable' from window.ui - cannot display card information");
        g_object_unref(windowBuilder);
        return;
    }
    state->cardTable = GTK_GRID(obj);

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

    obj = gtk_builder_get_object(windowBuilder, "ndefReadClear");
    if(obj == NULL) {
        g_warning("Failed to load 'ndefReadClear' button from window.ui - application may not function correctly");
        g_object_unref(windowBuilder);
        return;
    }
    state->ndefReadClearButton = GTK_BUTTON(obj);
    gtk_widget_set_sensitive(GTK_WIDGET(state->ndefReadClearButton), FALSE);
    g_signal_connect(state->ndefReadClearButton, "clicked", G_CALLBACK(onNdefReadClear), app);

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

    /* Polling tab: discovery settings */
    static const char TECH_LETTERS[POLL_TECH_COUNT] = { 'A', 'B', 'F', 'V' };
    const char * const pollingIds[] = { "pollIdleTime", "pollLpcdMode", "pollLpcdCycles", "pollF212", "pollF424",
                                        "pollingNote", "pollingRestoreDefaults" };
    GObject *pollingObjects[G_N_ELEMENTS(pollingIds)];
    for (guint i = 0; i < G_N_ELEMENTS(pollingIds); ++i)
    {
        pollingObjects[i] = gtk_builder_get_object(windowBuilder, pollingIds[i]);
        if(pollingObjects[i] == NULL) {
            g_warning("Failed to load '%s' from window.ui - cannot configure polling", pollingIds[i]);
            g_object_unref(windowBuilder);
            return;
        }
    }
    state->pollIdleTime = GTK_SPIN_BUTTON(pollingObjects[0]);
    state->pollLpcdMode = GTK_DROP_DOWN(pollingObjects[1]);
    state->pollLpcdCycles = GTK_SPIN_BUTTON(pollingObjects[2]);
    state->pollF212 = GTK_CHECK_BUTTON(pollingObjects[3]);
    state->pollF424 = GTK_CHECK_BUTTON(pollingObjects[4]);
    state->pollingNote = GTK_LABEL(pollingObjects[5]);
    g_signal_connect(pollingObjects[6], "clicked", G_CALLBACK(onPollingRestoreDefaults), app);

    for (guint i = 0; i < POLL_TECH_COUNT; ++i)
    {
        char id[32];
        snprintf(id, sizeof(id), "poll%cEnabled", TECH_LETTERS[i]);
        GObject *enabled = gtk_builder_get_object(windowBuilder, id);
        snprintf(id, sizeof(id), "poll%cFrequency", TECH_LETTERS[i]);
        GObject *frequency = gtk_builder_get_object(windowBuilder, id);
        snprintf(id, sizeof(id), "poll%cDevices", TECH_LETTERS[i]);
        GObject *devices = gtk_builder_get_object(windowBuilder, id);
        if((enabled == NULL) || (frequency == NULL) || (devices == NULL)) {
            g_warning("Failed to load the Type-%c polling settings from window.ui - cannot configure polling", TECH_LETTERS[i]);
            g_object_unref(windowBuilder);
            return;
        }
        state->pollEnabled[i] = GTK_CHECK_BUTTON(enabled);
        state->pollFrequency[i] = GTK_SPIN_BUTTON(frequency);
        state->pollDevices[i] = GTK_SPIN_BUTTON(devices);
        gtk_spin_button_set_range(state->pollFrequency[i], ptx_RUL_PollFreq_EveryCycle, ptx_RUL_PollFreq_CycleMax);
        gtk_spin_button_set_range(state->pollDevices[i], 1, PTX_DEMO_APP_DEVICE_LIMIT_PER_TECH);
        g_signal_connect(enabled, "toggled", G_CALLBACK(onPollingChanged), app);
        g_signal_connect(frequency, "value-changed", G_CALLBACK(onPollingChanged), app);
        g_signal_connect(devices, "value-changed", G_CALLBACK(onPollingChanged), app);
    }
    gtk_spin_button_set_range(state->pollLpcdCycles, ptx_RUL_PollLPCD_HybridMin, ptx_RUL_PollLPCD_HybridMax);
    g_signal_connect(state->pollIdleTime, "value-changed", G_CALLBACK(onPollingChanged), app);
    g_signal_connect(state->pollLpcdCycles, "value-changed", G_CALLBACK(onPollingChanged), app);
    g_signal_connect(state->pollF424, "toggled", G_CALLBACK(onPollingChanged), app);
    g_signal_connect(state->pollLpcdMode, "notify::selected", G_CALLBACK(onPollingLpcdChanged), app);

    PollingValues pollingDefaults;
    getDefaultPollingValues(&pollingDefaults);
    loadPollingUi(state, &pollingDefaults);
    refreshWriteRecordList(state);
    updateWriteSummary(state);

    gtk_label_set_text(state->cardStatusLabel, "Initializing ...");

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
        /* RF-Discovery / Polling Loop settings of the Polling tab (see buildPollConfig) */
        lockSharedState();
        discParams = sSharedState.pollConfig;
        unlockSharedState();

        /* Fixed settings: NFC Forum Poll-Mode only, Listen-mode (Card Emulation) not used in this demo. */
        discParams.PollMode = ptx_RUL_PollMode_NfcForum;
        discParams.EnablePolling = 1u;
#if !defined(PTX_DISABLE_LISTEN_MODE)
        discParams.ListenMode = 0;
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
                    (gint64)NDEF_WRITE_MAX_POLLING_CYCLES * ((gint64)discParams.IdleTime + PTX_DEMO_APP_POLLING_TIME_MS) * 1000;
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

/*
 * Merges the cards of the card registry (found in this discovery cycle) into the list of the Discovery tab:
 * known cards get their detection counter incremented, new cards are appended. Cards are never removed here,
 * the list is cleared when discovery is restarted.
 */
static void publishCardList(const ptx_RUL_CardRegistry_t *cardRegistry, gboolean activated)
{
    uint32_t cardsCnt = MIN(cardRegistry->CardsCnt, PTX_RUL_MAX_SUPPORTED_DEVICES);
    gboolean postUpdate = FALSE;

    lockSharedState();
    for (uint32_t i = 0; i < cardsCnt; ++i)
    {
        const ptx_RUL_CardParams_t *card = &cardRegistry->Cards[i];
        ListedCard *entry = NULL;

        for (uint32_t j = 0; (j < sSharedState.listedCardsCnt) && (NULL == entry); ++j)
        {
            if (isSameCard(&sSharedState.listedCards[j].card, card))
                entry = &sSharedState.listedCards[j];
        }

        if (NULL == entry)
        {
            if (sSharedState.listedCardsCnt >= PTX_DEMO_APP_MAX_LISTED_CARDS)
            {
                sSharedState.listFull = TRUE;
                continue;
            }
            entry = &sSharedState.listedCards[sSharedState.listedCardsCnt++];
            entry->activeProtocol = ptx_RUL_CardProtocolType_Undefined;
            entry->detections = 0;
        }

        entry->card = *card;
        entry->detections++;
        if (activated && (i == cardRegistry->ActiveCardId))
            entry->activeProtocol = cardRegistry->ActiveProtocol;
    }

    /* Counters change in every cycle - queue at most one UI update at a time */
    if ((cardsCnt > 0u) && !sSharedState.cardUpdatePending)
    {
        sSharedState.cardUpdatePending = TRUE;
        postUpdate = TRUE;
    }
    unlockSharedState();

    if (postUpdate)
    {
        g_idle_add(updateCardInfo, NULL);
    }
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
                {
                    /* Multiple cards present, the internal Discovery-procedure is still busy resolving all cards. */
                    (void)ptx_RUL_Sleep(rul, PTX_DEMO_APP_CARD_RESOLVE_SLEEP_MS);
                    break;
                }

                case ptx_RUL_StatusInfoDiscover_CardsAvailable:
                case ptx_RUL_StatusInfoDiscover_MultiProtocol:
                {
                    /*
                     * Multiple cards (or a single card supporting multiple protocols) present, all cards are resolved
                     * and stored in the card registry. The Discovery tab displays all of them, the NDEF tab only
                     * supports a single card.
                     */
                    lockSharedState();
                    DiscoveryMode mode = sSharedState.mode;
                    unlockSharedState();

                    if (DiscoveryMode_CardInfo == mode)
                    {
                        publishCardList(cardRegistry, FALSE);
                    }
                    *demoState = demoStateRestartDiscovery;
                    break;
                }

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
        if (DiscoveryMode_CardInfo == mode)
        {
            publishCardList(cardRegistry, TRUE);
        }
        else
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
                g_idle_add(updateNdefInfo, NULL);
            }
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
