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

    Description : GTK Discovery example application
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

#include <gtk/gtk.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES / CONSTANTS
 * ####################################################################################################################
 */

/* Application specific Exit-Flag (controllable by the user on Windows-/Linux-platforms via Ctrl-C). */
static volatile int gExit = 0;

/* Delay/Sleep between checking RF-Discovery state and no device was found. */
static const uint32_t PTX_DEMO_APP_NO_CARD_SLEEP_MS = 10u;

/* Delay/Sleep between checking RF-Discovery state and Discovery-procedure is ongoing. */
//static const uint32_t PTX_DEMO_APP_CARD_RESOVLVE_SLEEP_MS = 5u;

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

/* Work-buffer max. sizes for current demo example. */
#define NDEF_BUFFER_SIZE   256u

static uint8_t sNdefRxBuffer[NDEF_BUFFER_SIZE];
static uint8_t sNdefTxBuffer[NDEF_BUFFER_SIZE];
static uint8_t sNdefWorkBuffer[NDEF_BUFFER_SIZE];
static uint8_t sNdefMsgBuffer[NDEF_BUFFER_SIZE];
static volatile uint32_t sNdefMsgBufferSize = (uint32_t)NDEF_BUFFER_SIZE;

static ptx_RUL_t sRul = {0};
static volatile bool sDoStopDiscovery = false;
static volatile bool sDoRunDiscovery = false;
static volatile bool sDoReset = false;
static volatile ptx_RUL_CardParams_t sActiveCard = {0};
static volatile ptx_RUL_CardProtocolType_t sActiveProtocol;
static volatile ptx_Status_t sPtxStatus;
static GObject *sText = NULL;
static GObject *sStartStopButton = NULL;
static GThread *sWorkerThread = NULL;
static GThread *sInitThread = NULL;
//static GSettings sSettings;

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
static ptx_Status_t runDemo();
static ptx_Status_t processStateStartDiscovery(ptx_RUL_t *rul, demoState_t *demoState);
static ptx_Status_t processStateWaitForDiscovery(ptx_RUL_t *rul, demoState_t *demoState, ptx_RUL_CardRegistry_t *cardRegistry);
static ptx_Status_t processStateDataExchange(ptx_RUL_t *rul, demoState_t *demoState, ptx_RUL_CardRegistry_t *cardRegistry);
static ptx_Status_t processStateRestartDiscovery(ptx_RUL_t *rul, demoState_t *demoState);
static ptx_Status_t processStateDeactivateReader(ptx_RUL_t *rul, demoState_t *demoState);

static gboolean displayNdefMessage()
{
    char content[NDEF_BUFFER_SIZE];
    const size_t maxSize = sizeof(content);
    GtkTextIter iter;
    snprintf(content, maxSize, "\nNDEF message: ");
    GtkTextBuffer * const buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (sText));
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));

    memset(content, 0U, maxSize);
    memcpy(content, sNdefMsgBuffer, sNdefMsgBufferSize);

    GError *error = NULL;

    // convert to UTF-8 with fallback
    gchar *utf8Str = g_convert(
        content, sNdefMsgBufferSize, // Input string and length
        "UTF-8", "ISO-8859-1",       // To and From encodings
        NULL, NULL,
        &error
    );

    if (utf8Str) {
        snprintf(content, maxSize, "%s\n", utf8Str);
        gtk_text_buffer_get_end_iter(buffer, &iter);
        gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));
        g_free(utf8Str); // Caller must free the result
    } else {
        memset(content, 0U, maxSize);
        snprintf(content, maxSize, "Conversion failed: %s\n", error->message);
        gtk_text_buffer_get_end_iter(buffer, &iter);
        gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));
        g_error_free(error);
    }
    
    return FALSE;
}

static gboolean displayDiscoveryLoopError()
{
    char content[128] = "";
    const size_t maxSize = sizeof(content);
    GtkTextIter iter;

    snprintf(content, maxSize, "\nDiscovery loop ended with ERROR: 0x%04X\nPlease close the application and open it again!\n", sPtxStatus);
    GtkTextBuffer * const buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (sText));
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));

    return FALSE;
}

static gboolean updateCardInfo()
{
    if(!sDoRunDiscovery)
        return 0;

    char content[256] = "";
    const size_t maxSize = sizeof(content);

    snprintf(content, maxSize, "Card activated\n");
    GtkTextBuffer * const buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (sText));
    gtk_text_buffer_set_text (buffer, content, (int)strlen(content));
    
    GtkTextIter iter;

    switch(sActiveCard.TechType)
    {
        case ptx_RUL_CardRfTechType_A:
        {
            snprintf(content, maxSize, "RF-technology: Type-A\n");
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));

            snprintf(content, maxSize, "SENS_RES: ");
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));

            int n = 0;
            for (uint8_t i = 0; i < sizeof(sActiveCard.TechParam.TypeA.SensRes); ++i)
            {
                n += sprintf(&content[n], "%02X", sActiveCard.TechParam.TypeA.SensRes[i]);
            }
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));

            snprintf(content, maxSize, "\nID length: %02X\n", sActiveCard.TechParam.TypeA.NfcId1Len);
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));

            snprintf(content, maxSize, "ID: ");
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));

            n = 0;
            for (uint8_t i = 0; i < sActiveCard.TechParam.TypeA.NfcId1Len; ++i)
            {
                n += sprintf(&content[n], "%02X", sActiveCard.TechParam.TypeA.NfcId1[i]);
            }
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));
            break;
        }

        case ptx_RUL_CardRfTechType_B:
        {
            snprintf(content, maxSize, "RF-technology: Type-B\n");
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));

            snprintf(content, maxSize, "SENSB_RES: ");
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));

            int n = 0;
            for (uint8_t i = 0; i < sizeof(sActiveCard.TechParam.TypeB.SensBRes); ++i)
            {
                n += sprintf(&content[n], "%02X", sActiveCard.TechParam.TypeB.SensBRes[i]);
            }
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));
            break;
        }

        case ptx_RUL_CardRfTechType_F:
        {
            snprintf(content, maxSize, "RF-technology: Type-F\n");
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));

            snprintf(content, maxSize, "SENSF_RES: ");
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));

            int n = 0;
            for (uint8_t i = 0; i < sizeof(sActiveCard.TechParam.TypeF.SensFRes); ++i)
            {
                n += sprintf(&content[n], "%02X", sActiveCard.TechParam.TypeF.SensFRes[i]);
            }
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));
            break;
        }

        case ptx_RUL_CardRfTechType_V:
        {
            snprintf(content, maxSize, "RF-technology: Type-V\n");
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));

            snprintf(content, maxSize, "INVENTORY_RES: ");
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));

            int n = 0;
            for (uint8_t i = 0; i < sizeof(sActiveCard.TechParam.TypeV.InventoryRes); ++i)
            {
                n += sprintf(&content[n], "%02X", sActiveCard.TechParam.TypeV.InventoryRes[i]);
            }
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));
            break;
        }

        default:
        {
            snprintf(content, maxSize, "RF-technology: Unsupported\n");
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));
            break;
        }
    }

    switch(sActiveProtocol)
    {
        case ptx_RUL_CardProtocolType_T2T:
        {
            snprintf(content, maxSize, "\nProtocol: T2T\n");
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));
            break;
        }

        case ptx_RUL_CardProtocolType_T3T:
        {
            snprintf(content, maxSize, "\nProtocol: T3T\n");
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));
            break;
        }

        case ptx_RUL_CardProtocolType_T4T:
        {
            snprintf(content, maxSize, "\nProtocol: T4T (ISO-DEP)\n");
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));
            break;
        }

        case ptx_RUL_CardProtocolType_NfcDep:
        {
            snprintf(content, maxSize, "\nProtocol: P2P (NFC-DEP)\n");
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));
            break;
        }

        case ptx_RUL_CardProtocolType_T5T:
        {
            snprintf(content, maxSize, "\nProtocol: T5T\n");
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));
            break;
        }

        default:
        {
            snprintf(content, maxSize, "\nProtocol: Unknown / Unsupported\n");
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, content, (int)strlen(content));
            break;
        }
    }

    return FALSE;
}

static gint onInitDone(gpointer userData)
{
    GtkApplication *app = GTK_APPLICATION(userData);

    if(sPtxStatus == PTX_STATUS_SUCCESS)
    {
        const char * content = "System initialized. Press START to begin discovery!";
        GtkTextBuffer * buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (sText));
        gtk_text_buffer_set_text (buffer, content, (int)strlen(content));

        gtk_widget_set_sensitive(GTK_WIDGET(sStartStopButton), TRUE);
        GAction* resetAction = g_action_map_lookup_action(G_ACTION_MAP(app), "reset");
        GAction* prefsAction = g_action_map_lookup_action(G_ACTION_MAP(app), "preferences");
        g_simple_action_set_enabled(G_SIMPLE_ACTION(resetAction), TRUE);
        g_simple_action_set_enabled(G_SIMPLE_ACTION(prefsAction), TRUE);
    }
    else
    {
        char content[128] = "";
        const size_t maxSize = sizeof(content);
        snprintf(content, maxSize, "System initialization ERROR: 0x%04X\nPlease check the system and restart the application!", sPtxStatus);
        GtkTextBuffer * buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (sText));
        gtk_text_buffer_set_text (buffer, content, (int)strlen(content));
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

    sPtxStatus = status;
    g_idle_add(onInitDone, userData);
    g_print("Init thread finished with status %04X\n", status);

    return NULL;
}

static void startStop(GtkWidget *widget, gpointer userData)
{
    (void)widget;
    (void)userData;

    if(!sDoRunDiscovery)
    {
        const char * content = "";
        GtkTextBuffer * buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (sText));
        gtk_text_buffer_set_text (buffer, content, (int)strlen(content));
        gtk_button_set_label(GTK_BUTTON(sStartStopButton), "STOP");
        sDoRunDiscovery = true;

        if(sWorkerThread == NULL)
        {
            sWorkerThread = g_thread_new("worker", workerThread, NULL);
        }
    }
    else
    {
        gtk_button_set_label(GTK_BUTTON(sStartStopButton), "START");
        sDoStopDiscovery = true;
    }
}

static void closeRequest(GtkApplication *app, gpointer userData)
{
    (void)app;
    sDoRunDiscovery = false;
    gExit = 1;
    gtk_window_destroy(GTK_WINDOW(userData));
}


static void onReset (GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    (void)action;
    (void)parameter;
    (void)userData;
    const char * content = "";
    GtkTextBuffer * buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (sText));
    gtk_text_buffer_set_text (buffer, content, (int)strlen(content));
    if(sDoRunDiscovery)
        sDoReset = true;

}

static void onPreferences (GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    (void)action;
    (void)parameter;

    GtkWindow *parent = gtk_application_get_active_window (GTK_APPLICATION (userData));
    GtkAlertDialog *dlg = gtk_alert_dialog_new ("Preferences selected");
    gtk_alert_dialog_show (dlg, parent);
    g_object_unref (dlg);
}

static void onQuit(GSimpleAction *action, GVariant *parameter, gpointer userData)
{
    (void)action;
    (void)parameter;
    (void)userData;

    sDoRunDiscovery = false;
    gExit = 1;
    g_application_quit(G_APPLICATION(userData));
}


static void onActivate(GtkApplication *app, gpointer userData)
{
    (void)userData;

    g_setenv("GSETTINGS_SCHEMA_DIR", ".", FALSE);
    char cwdPath[4096];
    gchar *cwd = g_get_current_dir();
    snprintf(cwdPath, sizeof(cwdPath), "%s/", cwd);
    GSettings *settings = g_settings_new_with_path("app", cwdPath);
    g_free(cwd);
    GtkSettings *defaulSettings = gtk_settings_get_default();
    g_object_set(defaulSettings, "gtk-application-prefer-dark-theme", g_settings_get_boolean(settings, "use-dark-theme"), NULL);

    GtkBuilder *windowBuilder = gtk_builder_new();
    gtk_builder_add_from_file(windowBuilder, "window.ui", NULL);

    /* Connect signal handlers to the constructed widgets. */
    GObject *window = gtk_builder_get_object (windowBuilder, "window");
    gtk_window_set_application (GTK_WINDOW (window), app);

    g_signal_connect(window, "close_request", G_CALLBACK (closeRequest), window);

    sStartStopButton = gtk_builder_get_object (windowBuilder, "startstop");
    gtk_widget_set_sensitive(GTK_WIDGET(sStartStopButton), FALSE);
    g_signal_connect(sStartStopButton, "clicked", G_CALLBACK (startStop), NULL);

    sText = gtk_builder_get_object(windowBuilder, "text");

    if(sText == NULL || sStartStopButton == NULL)
        return;

    const char * content = "Initializing ...";
    GtkTextBuffer * buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (sText));
    gtk_text_buffer_set_text (buffer, content, (int)strlen(content));

    GObject *menuButton = gtk_builder_get_object(windowBuilder, "menu");

    GtkBuilder *menuBuilder = gtk_builder_new();
    gtk_builder_add_from_file(menuBuilder, "menu.ui", NULL);
    GMenuModel *menuModel = G_MENU_MODEL(gtk_builder_get_object (menuBuilder, "menu"));

    gtk_menu_button_set_menu_model (GTK_MENU_BUTTON(menuButton), menuModel);

    const GActionEntry actions[] = {
        { .name = "reset", .activate = onReset },
        { .name = "preferences", .activate = onPreferences },
        { .name = "quit", .activate = onQuit },
    };

    g_action_map_add_action_entries (G_ACTION_MAP(app),
                                     actions,
                                     G_N_ELEMENTS(actions),
                                     app);
    
    /* Optional accelerator: Ctrl+Q */
    gtk_application_set_accels_for_action (app, "app.quit",
                                           (const char *[]) { "<Primary>q", NULL });
    
    GAction* resetAction = g_action_map_lookup_action(G_ACTION_MAP(app), "reset");
    GAction* prefsAction = g_action_map_lookup_action(G_ACTION_MAP(app), "preferences");

    g_simple_action_set_enabled(G_SIMPLE_ACTION(resetAction), FALSE);
    g_simple_action_set_enabled(G_SIMPLE_ACTION(prefsAction), FALSE);

    sInitThread = g_thread_new("init", initThread, app);

    gtk_widget_set_visible(GTK_WIDGET (window), TRUE);

    g_object_unref(windowBuilder);
    g_object_unref(menuBuilder);
}

static void onStartup(GtkApplication *app, gpointer userData)
{
    (void)app;
    (void)userData;
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
        gExit = 1;
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
    g_signal_connect(gtkApp, "startup", G_CALLBACK (onStartup), NULL);
    g_signal_connect(gtkApp, "activate", G_CALLBACK (onActivate), NULL);
    g_application_run (G_APPLICATION (gtkApp), argc, argv);

    if(sInitThread != NULL)
    {
        g_thread_join(sInitThread);
        g_thread_unref(sInitThread);
    }

    if(sWorkerThread != NULL)
    {
        g_thread_join(sWorkerThread);
        g_thread_unref(sWorkerThread);
    }

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
        do
        {
            if(sDoRunDiscovery)
            {   
                if(sDoReset)
                {
                    demoState = demoStateRestartDiscovery;
                    sDoReset = false;
                }

                if(sDoStopDiscovery)
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
                        demoState = demoStateExit;
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
        while ((PTX_STATUS_SUCCESS == status) && (demoStateExit != demoState) && (0 == gExit));
    }

    if(status != PTX_STATUS_SUCCESS)
    {
        sPtxStatus = status;
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

    ptx_RUL_Status_Params_t discoverStatus;

    if ((NULL != rul) && (NULL != demoState) && (NULL != cardRegistry))
    {
        /* Check the status of the Discovery-Loop */
        memset(&discoverStatus, 0x00, sizeof(ptx_RUL_Status_Params_t));
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
                    sActiveCard = *cardRegistry->ActiveCard;
                    sActiveProtocol = cardRegistry->ActiveProtocol;
                    g_idle_add(updateCardInfo, NULL);

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
                /* Tag is NDEF-formatted - let's try to read an NDEF message from it. */
                uint32_t bufferSize = (uint32_t)NDEF_BUFFER_SIZE;
                status = ptx_NDEF_ReadMessage(&ndefComp, sNdefMsgBuffer, &bufferSize);
                sNdefMsgBufferSize = bufferSize;
                if(status == PTX_STATUS_SUCCESS)
                    g_idle_add(displayNdefMessage, NULL);
            }
            else
            {
                /* Activated Tag is probably not NDEF-formatted, let's move on. */
                status = PTX_STATUS_SUCCESS;
            }

            (void)ptx_NDEF_Close(&ndefComp);
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
            //ptx_Utils_Print("Reset of RF-Field successful - waiting for Cards ... \n");
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
            sDoStopDiscovery = false;
            sDoRunDiscovery = false;
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
