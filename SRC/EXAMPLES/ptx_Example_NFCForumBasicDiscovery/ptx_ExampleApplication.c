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
    Module      : SDK Example (NFC Forum Basic Discovery)
    File        : EXAMPLES/ptx_Example_NFCForumBasicDiscovery/ptx_ExampleApplication.c

    Description : GTK4 application using the PTX205R and the RUL SDK's reference implementation
                  for the Raspberry Pi and SPI.
                  Polls for RF-technologies A, B, F, V. If a card is activated, card
                  inforamtion is displayed. If the card contains an NDEF message, it's
                  displayed as UTF-8 encoded text, without any parsing.
                  Only one single card in the field is supported.
                  When the app is launched, the PTX is initialized and discovery can be
                  started/stopped by clicking the START/STOP button. There's a menu offering
                  the following options:
                  - Reset - displayed card informaiton is cleared and siscovery is restarted.
                  - Preferences - enable/disable dark mode.
                  - About - display about information.
                  - Quit - close the applicatoin.

                  Settings are permanently stored in the gschemas file.
                  If run from the command line, messages will be printed there.
*/

#include "ptx_ExampleApplication.h"
#include "ptx_Status.h"
#include "ptx_RUL.h"
#include "ptx_Utils_AppParameters.h"
#include "ptx_Utils_Print.h"
#include "ptx_Utils_VersionInfo.h"
#include "ptx_Utils_CardInfo.h"
#include "ptx_NDEF.h"
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
#define ACCEL_RESET       "<Primary>r"

/* Work-buffer max. sizes for current demo example. */
#define NDEF_BUFFER_SIZE   256u

/* Consolidated shared state with single mutex protecting all shared data */
typedef struct {
    GMutex mutex;                              /* Single mutex protecting all shared state */

    /* Control flags */
    int gExit;                                 /* Exit flag (was volatile int) */
    gboolean doStopDiscovery;                  /* Was sDoStopDiscovery (volatile bool) */
    gboolean doRunDiscovery;                   /* Was sDoRunDiscovery (volatile bool) */
    gboolean doReset;                          /* Was sDoReset (volatile bool) */

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
} SharedState;

static SharedState sSharedState = {0};

/* Temporary NDEF work buffers (not shared across threads) */
static uint8_t sNdefRxBuffer[NDEF_BUFFER_SIZE];
static uint8_t sNdefTxBuffer[NDEF_BUFFER_SIZE];
static uint8_t sNdefWorkBuffer[NDEF_BUFFER_SIZE];

/* PTX device handle (protected by application synchronization, not by sSharedState mutex) */
static ptx_RUL_t sRul = {0};

typedef struct {
    gboolean darkMode;
    GtkTextView *contentTextView;
    GtkButton *startStopButton;
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
typedef GtkWindow* (*LoadWindowFunc)(GtkApplication *app, AppState *state, GtkWindow *parent);
static void presentWindow(GtkWindow **window, LoadWindowFunc loader, GtkApplication *app, AppState *state);
static ptx_Status_t runDemo();
static ptx_Status_t processStateStartDiscovery(ptx_RUL_t *rul, demoState_t *demoState);
static ptx_Status_t processStateWaitForDiscovery(ptx_RUL_t *rul, demoState_t *demoState, ptx_RUL_CardRegistry_t *cardRegistry);
static ptx_Status_t processStateDataExchange(ptx_RUL_t *rul, demoState_t *demoState, ptx_RUL_CardRegistry_t *cardRegistry);
static ptx_Status_t processStateRestartDiscovery(ptx_RUL_t *rul, demoState_t *demoState);
static ptx_Status_t processStateDeactivateReader(ptx_RUL_t *rul, demoState_t *demoState);

static void appendToTextBuffer(GtkTextBuffer *buffer, const char *fmt, ...)
{
    char content[256];
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
    char hexBuf[256];

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

static gboolean displayDiscoveryLoopError()
{
    GtkApplication *app = GTK_APPLICATION(g_application_get_default());
    AppState *state = g_object_get_data(G_OBJECT(app), "app-state");

    g_mutex_lock(&sSharedState.mutex);
    ptx_Status_t status = sSharedState.ptxStatus;
    g_mutex_unlock(&sSharedState.mutex);

    GtkTextBuffer * const buffer = gtk_text_view_get_buffer(state->contentTextView);
    appendToTextBuffer(buffer, "\nDiscovery loop ended with ERROR: 0x%04X\nPlease close the application and open it again!\n", status);

    return FALSE;
}

static gboolean updateCardInfo()
{
    g_mutex_lock(&sSharedState.mutex);
    gboolean isRunning = sSharedState.doRunDiscovery;
    g_mutex_unlock(&sSharedState.mutex);

    if(!isRunning)
        return FALSE;

    GtkApplication *app = GTK_APPLICATION(g_application_get_default());
    AppState *state = g_object_get_data(G_OBJECT(app), "app-state");
    GtkTextBuffer * const buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(state->contentTextView));

    gtk_text_buffer_set_text(buffer, "Card activated\n", 15);

    g_mutex_lock(&sSharedState.mutex);
    ptx_RUL_CardParams_t localCard = sSharedState.activeCard;
    ptx_RUL_CardProtocolType_t localProtocol = sSharedState.activeProtocol;
    gboolean isNdef = sSharedState.isNdefMessage;
    gboolean isNdefRead = sSharedState.isNdefMessageRead;
    uint32_t ndefSize = sSharedState.ndefMsgBufferSize;
    g_mutex_unlock(&sSharedState.mutex);

    displayCardTechInfo(buffer, &localCard);

    switch(localProtocol)
    {
        case ptx_RUL_CardProtocolType_T2T:
            appendToTextBuffer(buffer, "Protocol: T2T\n");
            break;
        case ptx_RUL_CardProtocolType_T3T:
            appendToTextBuffer(buffer, "Protocol: T3T\n");
            break;
        case ptx_RUL_CardProtocolType_T4T:
            appendToTextBuffer(buffer, "Protocol: T4T (ISO-DEP)\n");
            break;
        case ptx_RUL_CardProtocolType_NfcDep:
            appendToTextBuffer(buffer, "Protocol: P2P (NFC-DEP)\n");
            break;
        case ptx_RUL_CardProtocolType_T5T:
            appendToTextBuffer(buffer, "Protocol: T5T\n");
            break;
        default:
            appendToTextBuffer(buffer, "Protocol: Unknown / Unsupported\n");
            break;
    }

    if(isNdef)
    {
        if(isNdefRead)
        {
            appendToTextBuffer(buffer, "\nNDEF message: ");

            // Make a local copy of the NDEF buffer
            uint8_t ndefCopy[NDEF_BUFFER_SIZE] = {0};
            g_mutex_lock(&sSharedState.mutex);
            memcpy(ndefCopy, sSharedState.ndefMsgBuffer, ndefSize);
            g_mutex_unlock(&sSharedState.mutex);

            GError *error = NULL;
            gchar *utf8Str = g_convert((const char *)ndefCopy, ndefSize, "UTF-8", "ISO-8859-1", NULL, NULL, &error);

            if (utf8Str) {
                appendToTextBuffer(buffer, "%s\n", utf8Str);
                g_free(utf8Str);
            } else {
                appendToTextBuffer(buffer, "Conversion failed: %s\n", error->message);
                g_error_free(error);
            }
        }
        else
        {
            appendToTextBuffer(buffer, "\nCould not read NDEF message.");
        }
    }

    return FALSE;
}

static gint onInitDone(gpointer userData)
{
    GtkApplication *app = GTK_APPLICATION(userData);
    AppState *state = g_object_get_data(G_OBJECT(app), "app-state");
    GtkTextBuffer * buffer = gtk_text_view_get_buffer(state->contentTextView);

    g_mutex_lock(&sSharedState.mutex);
    ptx_Status_t status = sSharedState.ptxStatus;
    g_mutex_unlock(&sSharedState.mutex);

    if(status == PTX_STATUS_SUCCESS)
    {
        gtk_text_buffer_set_text(buffer, "System initialized. Press START to begin discovery!", 50);
        gtk_widget_set_sensitive(GTK_WIDGET(state->startStopButton), TRUE);
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
    ptx_Utils_AppParams_t app_params;

    /* Main API component */
    ptx_RUL_InitParams_t rul_init_params = {0};

    /* Initialize demo application parameters with default values. */
    (void)memcpy(&app_params.DeviceName[0], EXAMPLE_DEVICE_NAME, EXAMPLE_DEVICE_NAME_LEN);
    app_params.DeviceNameLen = EXAMPLE_DEVICE_NAME_LEN;

    /* Initialize Main RUL API Component.
     *
     * Note: The used Host-Interface i.e. the HAL-component and the Logging-component (if used) get selected via the Build-process (see User Manual)
     *       and get also initialized during the next call to "ptx_RUL_Init".
     */
    rul_init_params.InterfaceInitParams.InterfaceName = (uint8_t*)&app_params.DeviceName[0];
    rul_init_params.InterfaceInitParams.InterfaceNameLen = app_params.DeviceNameLen;
    rul_init_params.InterfaceInitParams.InterfaceType = EXAMPLE_INTERFACE_TYPE;
    rul_init_params.InterfaceInitParams.InterfacePortType = EXAMPLE_INTERFACE_PORT_TYPE;
    rul_init_params.InterfaceInitParams.InterfacePortID = EXAMPLE_INTERFACE_PORT_ID;
    rul_init_params.InterfaceInitParams.NumberOfTimers = EXAMPLE_NUMBER_OF_TIMERS;

    status = ptx_RUL_Init(&sRul, &rul_init_params);

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

    g_mutex_lock(&sSharedState.mutex);
    sSharedState.ptxStatus = status;
    g_mutex_unlock(&sSharedState.mutex);
    g_idle_add(onInitDone, userData);
    g_print("Init thread finished with status %04X\n", status);

    return NULL;
}

static void startStop(GtkWidget *widget, gpointer userData)
{
    (void)widget;
    AppState *state = g_object_get_data(G_OBJECT(userData), "app-state");

    g_mutex_lock(&sSharedState.mutex);
    gboolean isRunning = sSharedState.doRunDiscovery;
    g_mutex_unlock(&sSharedState.mutex);

    if(!isRunning)
    {
        GtkTextBuffer * buffer = gtk_text_view_get_buffer(state->contentTextView);
        gtk_text_buffer_set_text(buffer, "", 0);
        gtk_button_set_label(state->startStopButton, "STOP");

        g_mutex_lock(&sSharedState.mutex);
        sSharedState.doRunDiscovery = true;
        g_mutex_unlock(&sSharedState.mutex);

        if(state->workerThread == NULL)
        {
            state->workerThread = g_thread_new("worker", workerThread, NULL);
        }
    }
    else
    {
        gtk_button_set_label(state->startStopButton, "START");
        g_mutex_lock(&sSharedState.mutex);
        sSharedState.doStopDiscovery = true;
        g_mutex_unlock(&sSharedState.mutex);

        g_mutex_lock(&sSharedState.mutex);
        sSharedState.activeCard = (ptx_RUL_CardParams_t){0};
        g_mutex_unlock(&sSharedState.mutex);
    }
}

static void closeRequest(GtkApplication *app, gpointer userData)
{
    (void)app;
    g_mutex_lock(&sSharedState.mutex);
    sSharedState.doRunDiscovery = false;
    sSharedState.gExit = 1;
    g_mutex_unlock(&sSharedState.mutex);
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

static GtkWindow *loadPrefsWindow(GtkApplication *app, AppState *state, GtkWindow *parent)
{
    GtkBuilder *builder = gtk_builder_new_from_file("prefs.ui");

    GtkWidget *prefsWindow = GTK_WIDGET(gtk_builder_get_object(builder, "prefsWindow"));
    if (!prefsWindow) {
        g_error("Failed to load 'prefsWindow' from prefs.ui");
    }

    // Widgets
    state->darkModeSwitch = GTK_SWITCH(gtk_builder_get_object(builder, "darkModeSwitch"));

    // Initialize from current state (loaded from GSettings earlier)
    gtk_switch_set_active(state->darkModeSwitch, state->darkMode);

    // Set relationships
    gtk_window_set_application(GTK_WINDOW(prefsWindow), app);
    if (parent) gtk_window_set_transient_for(GTK_WINDOW(prefsWindow), parent);

    g_settings_bind(state->settings, "dark-mode",
                    state->darkModeSwitch, "active",
                    G_SETTINGS_BIND_DEFAULT);

    g_signal_connect(state->darkModeSwitch, "state-set", G_CALLBACK(onDarkModeSwitchSet), NULL);

    g_object_unref(builder);
    return GTK_WINDOW(prefsWindow);
}

static GtkWindow *loadAboutWindow(GtkApplication *app, AppState *state, GtkWindow *parent)
{
    (void)state;

    GtkBuilder *builder = gtk_builder_new_from_file("about.ui");

    GtkWidget *aboutWindow = GTK_WIDGET(gtk_builder_get_object(builder, "aboutWindow"));
    if (!aboutWindow) {
        g_error("Failed to load 'aboutWindow' from about.ui");
    }

    gtk_window_set_application(GTK_WINDOW(aboutWindow), app);
    if (parent) gtk_window_set_transient_for(GTK_WINDOW(aboutWindow), parent);

    g_object_unref(builder);
    return GTK_WINDOW(aboutWindow);
}

static void onReset(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    (void)action;
    (void)parameter;

    AppState *state = g_object_get_data(G_OBJECT(userData), "app-state");
    GtkTextBuffer * buffer = gtk_text_view_get_buffer(state->contentTextView);
    gtk_text_buffer_set_text(buffer, "", 0);

    g_mutex_lock(&sSharedState.mutex);
    gboolean isRunning = sSharedState.doRunDiscovery;
    if(isRunning)
        sSharedState.doReset = true;
    g_mutex_unlock(&sSharedState.mutex);

    g_mutex_lock(&sSharedState.mutex);
    sSharedState.activeCard = (ptx_RUL_CardParams_t){0};
    g_mutex_unlock(&sSharedState.mutex);
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

    g_mutex_lock(&sSharedState.mutex);
    sSharedState.doRunDiscovery = false;
    sSharedState.gExit = 1;
    g_mutex_unlock(&sSharedState.mutex);
    g_application_quit(G_APPLICATION(userData));
}

static void onActivate(GtkApplication *app, gpointer userData)
{
    (void)userData;

    // initialize mutexes for thread-safe access
    g_mutex_init(&sSharedState.mutex);

    // intialize app state
    AppState *state = g_new0(AppState, 1);
    g_object_set_data_full(G_OBJECT(app), "app-state", state, (GDestroyNotify)g_free);

    // set up settings
    g_setenv("GSETTINGS_SCHEMA_DIR", ".", FALSE); // compiled schema file is expected to be inside the current working directory
    char cwdPath[4096];
    gchar *cwd = g_get_current_dir();
    snprintf(cwdPath, sizeof(cwdPath), "%s/", cwd);
    state->settings = g_settings_new_with_path("app", cwdPath);
    g_free(cwd); // memeory must be freed manually
    state->darkMode = g_settings_get_boolean(state->settings, "dark-mode");
    g_object_set(gtk_settings_get_default(), "gtk-application-prefer-dark-theme", state->darkMode, NULL);

    // set up UI
    GtkBuilder *windowBuilder = gtk_builder_new_from_file("window.ui");
    /* Connect signal handlers to the constructed widgets. */
    GObject *window = gtk_builder_get_object(windowBuilder, "window");
    gtk_window_set_application(GTK_WINDOW(window), app);
    g_signal_connect(window, "close_request", G_CALLBACK(closeRequest), window);

    GObject *obj = gtk_builder_get_object(windowBuilder, "startstop");
    if(obj == NULL) {
        g_object_unref(windowBuilder);
        return;
    }
    state->startStopButton = GTK_BUTTON(obj);
    gtk_widget_set_sensitive(GTK_WIDGET(state->startStopButton), FALSE);
    g_signal_connect(state->startStopButton, "clicked", G_CALLBACK(startStop), app);

    obj = gtk_builder_get_object(windowBuilder, "cardInfoText");
    if(obj == NULL) {
        g_object_unref(windowBuilder);
        return;
    }
    state->contentTextView = GTK_TEXT_VIEW(obj);

    GtkTextBuffer * buffer = gtk_text_view_get_buffer(state->contentTextView);
    gtk_text_buffer_set_text(buffer, "Initializing ...", 16);

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
    gtk_application_set_accels_for_action(app, "app.reset", (const char *[]) { ACCEL_RESET, NULL });
    
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
            g_mutex_lock(&sSharedState.mutex);
            doRun = sSharedState.doRunDiscovery;
            doReset = sSharedState.doReset;
            doStop = sSharedState.doStopDiscovery;
            if(doReset)
                sSharedState.doReset = false;
            exitFlag = sSharedState.gExit;
            g_mutex_unlock(&sSharedState.mutex);

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
                        ptx_Utils_Print("System status ERROR: %d\n", statusInfo.Info.System);
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
                            ptx_Utils_Print("Invalid Application-State detected!\n");
                            demoState = demoStateExit;
                            break;
                    }
                }
            }
        }
        while ((PTX_STATUS_SUCCESS == status) && (demoStateExit != demoState) && (0 == exitFlag));
    }

    if(status != PTX_STATUS_SUCCESS)
    {
        g_mutex_lock(&sSharedState.mutex);
        sSharedState.ptxStatus = status;
        g_mutex_unlock(&sSharedState.mutex);
        g_idle_add(displayDiscoveryLoopError, NULL);
    }

    return status;
}

static ptx_Status_t processStateStartDiscovery(ptx_RUL_t *rul, demoState_t *demoState)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_RUL_DiscoverConfig_t disc_params = {0};

    if ((NULL != rul) && (NULL != demoState))
    {
        /* Initialize RF-Discovery / Polling Loop in NFC Forum Poll-Mode with standard parameters.*/
        disc_params.PollMode = ptx_RUL_PollMode_NfcForum;   /* Enable PolL-Mode only in this example (Listen-mode can be optionally enabled). */
        disc_params.EnablePolling = 1u;                     /* Enable Polling only (Listen-mode can be optionally enabled). */
        disc_params.IdleTime = 100u;                        /* System pauses / idles for 100ms after every full polling loop-cycle. */
        disc_params.PollTypeA = 1u;                         /* Poll for NFC-A Technology. */
        disc_params.PollTypeAPollFrequency = 1u;            /* Poll for NFC-A Technology during every Polling-Loop cycle (otherwise every n-th cycle). */
        disc_params.PollTypeADeviceLimit = 5u;              /* Limit the number of NFC-A devices / cards to max. 5 devices (application can define max. value). */
        disc_params.PollTypeB = 1u;                         /* Poll for NFC-B Technology. */
        disc_params.PollTypeBPollFrequency = 1u;            /* Poll for NFC-B Technology during every Polling-Loop cycle (otherwise every n-th cycle). */
        disc_params.PollTypeBDeviceLimit = 5u;              /* Limit the number of NFC-B devices / cards to max. 5 devices (application can define max. value). */
        disc_params.PollTypeF212 = 1u;                      /* Poll for NFC-F 212 kBit/s Technology. */
        disc_params.PollTypeFPollFrequency = 1u;            /* Poll for NFC-F Technology during every Polling-Loop cycle (otherwise every n-th cycle). */
        disc_params.PollTypeFDeviceLimit = 5u;              /* Limit the number of NFC-F devices / cards to max. 5 devices (application can define max. value). */
        disc_params.PollTypeV = 1u;                         /* Poll for NFC-V Technology. */
        disc_params.PollTypeVPollFrequency = 1u;            /* Poll for NFC-V Technology during every Polling-Loop cycle (otherwise every n-th cycle). */
        disc_params.PollTypeVDeviceLimit = 5u;              /* Limit the number of NFC-V devices / cards to max. 5 devices (application can define max. value). */
        disc_params.EnableLPCD = 0;                         /* Don't use low-power-card-detection (LPCD) in this demo. */
#if !defined(PTX_DISABLE_LISTEN_MODE)
        disc_params.ListenMode = 0;                         /* Don't use Listen-mode (Card Emulation) in this demo. */
#endif /* PTX_DISABLE_LISTEN_MODE */

        status = ptx_RUL_InitiateDiscovery(rul, &disc_params);

        if (PTX_STATUS_SUCCESS == status)
        {
            *demoState = demoStateWaitForDiscovery;
            ptx_Utils_Print("RF-Discovery successfully started - waiting for Cards ... \n");
        }
        else
        {
            *demoState = demoStateExit;
            ptx_Utils_Print("Starting RF-Discovery failed with Error-Status %04X!\n", status);
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
                    // not dealing wiht multiple cards, multiprotocol or card selection
                    *demoState = demoStateRestartDiscovery;
                    break;

                default:
                {
                    *demoState = demoStateExit;
                    ptx_Utils_Print("Invalid/Unsupported Application-State detected!\n");
                    break;
                }
            }
        }
        else
        {
            *demoState = demoStateExit;
            ptx_Utils_Print("Check status of RF-Discovery failed with Error-Status %04X!\n", status);
        }
    }

    return status;
}

static ptx_Status_t processStateDataExchange(ptx_RUL_t *rul, demoState_t *demoState, ptx_RUL_CardRegistry_t *cardRegistry)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    sSharedState.isNdefMessage = false;

    if ((NULL != rul) && (NULL != demoState) && (NULL != cardRegistry))
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
                g_mutex_lock(&sSharedState.mutex);
                sSharedState.isNdefMessage = true;
                g_mutex_unlock(&sSharedState.mutex);

                /* Tag is NDEF-formatted - let's try to read an NDEF message from it. */
                uint32_t bufferSize = (uint32_t)NDEF_BUFFER_SIZE;
                status = ptx_NDEF_ReadMessage(&ndefComp, sSharedState.ndefMsgBuffer, &bufferSize);

                g_mutex_lock(&sSharedState.mutex);
                sSharedState.ndefMsgBufferSize = bufferSize;
                g_mutex_unlock(&sSharedState.mutex);

                if(status == PTX_STATUS_SUCCESS)
                {
                    g_mutex_lock(&sSharedState.mutex);
                    sSharedState.isNdefMessageRead = true;
                    g_mutex_unlock(&sSharedState.mutex);
                }
                else
                {
                    /* Reading NDEF failed, let's move on. */
                    g_mutex_lock(&sSharedState.mutex);
                    sSharedState.isNdefMessageRead = false;
                    g_mutex_unlock(&sSharedState.mutex);
                    status = PTX_STATUS_SUCCESS;
                }
            }
            else
            {
                /* Activated Tag is probably not NDEF-formatted, let's move on. */
                status = PTX_STATUS_SUCCESS;
            }

            (void)ptx_NDEF_Close(&ndefComp);
        }

        g_mutex_lock(&sSharedState.mutex);
        ptx_RUL_CardParams_t activeCard = sSharedState.activeCard;
        g_mutex_unlock(&sSharedState.mutex);

        // only update UI if the card is different from the previous one
        if(0 != memcmp(&activeCard, cardRegistry->ActiveCard, sizeof(activeCard)))
        {
            g_mutex_lock(&sSharedState.mutex);
            sSharedState.activeCard = *cardRegistry->ActiveCard;
            sSharedState.activeProtocol = cardRegistry->ActiveProtocol;
            g_mutex_unlock(&sSharedState.mutex);
            g_idle_add(updateCardInfo, NULL);
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
            ptx_Utils_Print("Reseting RF-Field failed with Error-Status %04X!\n", status);
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
            g_mutex_lock(&sSharedState.mutex);
            sSharedState.doStopDiscovery = false;
            sSharedState.doRunDiscovery = false;
            g_mutex_unlock(&sSharedState.mutex);
            *demoState = demoStateStartDiscovery;
        }
        else
        {
            ptx_Utils_Print("Switching RF-Field off failed with Error-Status %04X!\n", status);
            *demoState = demoStateExit;
        }
    }

    return status;
}
