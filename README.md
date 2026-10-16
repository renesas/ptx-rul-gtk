# GTK4 Discovery Application

GTK4 application using the PTX205R and the RUL SDK 1.2.1 reference implementation for the Raspberry Pi and SPI.

## Modifications made to the SDK example

The project is based on the SDK's `NFC_FORUM_BASIC_DISCOVERY` example.
`CMakeLists.txt` under `PROJECTS/PTX2xxR/NFC_FORUM_BASIC_DISCOVERY` was modified to work with GTK and NDEF reading/writing. In addition to the NDEF component of the SDK, it also builds the SDK's NDEF record sources (`SRC/API/NDEF/RECORDS`).
The source and header files under `SRC/EXAMPLES/ptx_Example_NFCForumBasicDiscovery` were modified and unnecessary pieces of application code were removed.
`ptx_ExampleNdefRecords.c` and `ptx_ExampleNdefRecords.h` were added there, they contain the definitions of the standard NDEF record types that can be written.
The basic structure of the SDK deliverable was kept, only unnecessary project and example files and directories were removed.
The SDK sources themselves are not modified.

## Hardware requirements
Raspberry Pi 5 with SPI enabled.

On the PTX205R board solder the jumpers for:
- MCU OFF
- PMOD_VDD TO VDD_IO

Connect the PTX to the Raspberry:

| PTX205R PMOD | Raspberry Pi 5 |
| ------------ | -------------- |
| MOSI         | GPIO 10        |
| MISO         | GPIO 9         |
| SCK          | GPIO 11        |
| NSS          | GPIO 7         |
| IRQ          | GPIO 19        |
| SEN          | GPIO 21        |

Connect the PTX board via USB-C for power.

## Software requirements

- RUL SDK 1.2.1
- GTK4 - install the `libgtk-4-dev` package
- cmake

## Application settings

The application permanently stores preferences using `GSettings`.
The settings are defined in `app.gschema.xml`, and actual settings are stored in `gschemas.compiled`.
Both files are located in the `BUILD` directory.

Each time settings are modified, the settings file must be compiled again, by running the following command from the `BUILD` directory:
```
glib-compile-schemas .
```
For the sake of demonstration, there's one setting implemented, which is a switch to enable or disable dark mode.
It can be accessed by clicking the menu button and *Preferences*.
The state of the switch is bound to the setting stored in `gschemas.compiled`.
When the switch is changed, `gschemas.compiled` is updated.
Upon launching the application, the setting is retrieved from `gschemas.compiled` and applied.
When opening the *Preferences* window, the state of the switch is set according to the setting in `gschemas.compiled`.
The presence of a compiled settings file with the expected settings is necessary for running the application.

## Build and run

Change to the build directory: `PROJECTS/PTX2xxR/NFC_FORUM_BASIC_DISCOVERY/BUILD`.

Configure cmake for Linux, SPI and the Console for printing log messages and File for internal logging:
```
cmake .. -DPTX_BUILD_LIN=ON -DPTX_ENABLE_SPI=ON -DPTX_UTILITY_PRINTING=Console -DPTX_LOGGING=File
```

Build:
```
cmake --build .
```

Run:
```
./NFC_FORUM_BASIC_DISCOVERY
```

The application can also be launched by double-clicking the executable.
If the application is run from the command line, information will be printed there.

## Necessary files for running the application

All necessary files for running the application are inside the `BUILD` directory, from where it must be launched.

For running the executable outside the `BUILD` directory, the following files must be copied and they must be inside the same directory as the executable:
- `libRUL.so` - RUL shared library - need to run `export LD_LIBRARY_PATH=$(pwd):$LD_LIBRARY_PATH` from `BUILD` once in order to add the path
- `window.ui` - main window file
- `prefs.ui` - preferences window file
- `menu.ui` - menu file
- `about.ui` - about window file
- `gschemas.compiled` - the compiled settings file, where the actual settings are stored

When distributing the application, the following license files must also be included alongside the executable (they are copied to the `BUILD` directory by cmake):
- `COPYING` - LGPL v2.1 full text
- `COPYING.LESSER` - LGPL v2.1 clarification
- `DEPENDENCIES.md` - list of dependencies and their licenses

The settings schema file `app.gschema.xml` is only required for compiling the settings file once, not for running the application.

## Under the hood

When the application is launched, UI files and settings are loaded, and the PTX is initialized in a separate thread.
If initialization was successful, discovery can be started from the *Discovery* tab, and NDEF messages can be read and written on the *NDEF* tab. The discovery settings are configured on the *Polling* tab.
Discovery and the NDEF operations share one discovery loop, which runs in a separate thread and polls for RF-technologies A, B, F, V.
The *Discovery* tab supports multiple cards in the field, the *NDEF* tab supports only one single card.
While discovery is running, the tabs cannot be switched.

If the initialization fails, an error message will be displayed and the application must be closed.
If discovery fails to start or the discovery loop ends with an error, an error message will be displayed and the application must be closed.
Some menu items are disabled until the initialization is successfully finished.
If the application is closed, the PTX is de-initialized - if discovery was running, it will stop.

### Discovery

Discovery is started by clicking the START button on the *Discovery* tab.
When discovery is started, the START button becomes the STOP button, and vice versa.
Multiple cards in the field are supported: up to 5 cards per RF-technology (A, B, F, V) and discovery cycle, 20 cards in total, as in the SDK's example application. The limit per technology can be lowered on the *Polling* tab.
The SDK's card registry holds up to 32 cards (`PTX_RUL_MAX_SUPPORTED_DEVICES`), the per-technology limit is set by `PTX_DEMO_APP_DEVICE_LIMIT_PER_TECH`.

When cards are found, the UI thread is updated and the cards are displayed in a table, one row per card:

| Column   | Content |
| -------- | ------- |
| #        | Number of the card |
| RF-Tech  | Type-A, Type-B, Type-F or Type-V |
| Protocol | The protocol the card was activated with (a card is only activated when it is alone in the field), otherwise the protocols supported by the card |
| Bit rate | Nominal RF bit rate (TX / RX if they differ). Discovered cards report the default of their RF-technology (Type-A/B 106, Type-F 212 or 424, Type-V 26 kbit/s), higher bit rates are only reported after an activation negotiating them, which this application does not enable |
| ID       | NFCID1 (Type-A), NFCID0 (Type-B), NFCID2 (Type-F) or UID (Type-V, most significant byte first) |
| Detections | Number of discovery cycles in which the card was found |
| Details  | Technology-specific data that is not part of the ID: ATQA and SAK (Type-A), application data and protocol info of SENSB_RES (Type-B), PMm and - if present - system code of SENSF_RES (Type-F), DSFID (Type-V). The complete SENS_RES/SEL_RES, SENSB_RES, SENSF_RES or INVENTORY_RES is shown as tooltip |

Above the table, the number of cards detected since discovery was started is displayed.
Not every card is resolved in every discovery cycle when multiple cards are in the field, so the cards are collected: once a card has been detected, it stays in the list - also after it was removed from the field - until discovery is restarted (STOP and START, or RESET).
Cards are identified by their RF-technology and ID, and keep their position in the list in the order of their first detection.
The Detections column shows how often each card was found, it stops increasing when a card is removed.
The list holds up to 50 cards (`PTX_DEMO_APP_MAX_LISTED_CARDS`), further cards are not listed and the status line reports that the list is full.
Cards using a random ID (e.g. some mobile phones) get a new entry for every new ID.
The RESET button clears the displayed card information and restarts discovery.

### NDEF read

NDEF messages are read when discovery is started with the READ button on the *NDEF* tab, and reading is stopped with the STOP button. The CLEAR button clears the output.
If a card is activated, its NDEF message is read and the output starts with a one-line card summary:
```
Card: RF-Tech <technology>, Protocol <protocol>, ID <hex>
```
The ID is NFCID1 for Type-A, NFCID0 for Type-B, NFCID2 for Type-F and the UID for Type-V (shown most significant byte first).
The card summary is followed by the NDEF records and the raw bytes of the message, displayed as continuous hex digits.

The message is parsed into its records by a bounds-checked parser in the application.
The SDK's own message parser is not used, because it decodes the payload length of long records in host byte order and does not check records against the end of the buffer.
The following record types are decoded:
- URI - the URI identifier code is expanded to the full URI
- Text - language code and text (UTF-8 or UTF-16)
- Smart Poster - the nested records are displayed indented
- MIME media types - displayed as text if the content is printable, as hex otherwise
- absolute URI, external type and empty records

The payload of any other record is displayed as hex.
At most 16 records are parsed per message and Smart Posters can be nested two levels deep.
Chunked records are not reassembled.
If the message is malformed, the records parsed so far are displayed together with a warning.

### NDEF write

The NDEF message to write is composed with the record type picker and the buttons next to the WRITE button, and the fields below them:
1. Pick a record type in the drop-down list and fill in its fields (required fields are marked with `*`).
2. Click ADD to add the record to the message. Repeat to add more records.
3. The records are listed with the size of the message and its raw bytes. REMOVE deletes the selected record, CLEAR deletes all records.

The list initially contains a URI record: `https://www.renesas.com/en/products/ptx205r`.
The message cannot be larger than 256 bytes. A record that would exceed this limit is not added.

Clicking WRITE starts discovery and writes the message to the first activated card, then stops discovery.
If no card is activated within 10 polling cycles, discovery stops and a message is displayed.
The RUL does not report polling cycles, so the limit is a time budget of 10 times the polling-loop cycle time: the idle time set on the *Polling* tab plus an estimated 50 ms of polling (about 1.5 s with the default idle time of 100 ms).
The status label below displays the result: success, no card found, card is not NDEF-formatted, or the error code of a failed write.
The card has to be NDEF-formatted, formatting cards is not supported.

While discovery is running, the READ button is disabled; while no discovery is running, the STOP button is disabled.
After STOP is clicked, READ is enabled again once the reader has been switched off.
CLEAR is only enabled while no NDEF read or write is running.
While a write is in progress, the record editor and the READ, STOP and CLEAR buttons are disabled.
While an NDEF read is running, the WRITE button is disabled.

The following record types are available:

| Type         | Fields                                               | Encoding |
| ------------ | ---------------------------------------------------- | -------- |
| URI          | URI                                                  | URI record, the longest matching URI identifier code is used (case-sensitive) |
| Text         | Language, Text                                       | Text record, UTF-8 |
| Smart Poster | URI, Title, Language                                 | Smart Poster record containing a URI and an optional Text record |
| Phone        | Number                                               | URI record `tel:<number>` |
| Email        | Address, Subject, Body                               | URI record `mailto:<address>?subject=...&body=...` |
| SMS          | Number, Message                                      | URI record `sms:<number>?body=...` |
| Geo location | Latitude, Longitude                                  | URI record `geo:<latitude>,<longitude>` |
| vCard        | Name, Phone, Email, Organization                     | MIME record `text/vcard`, vCard 3.0 |
| MIME         | MIME type, Content                                   | MIME record with the given type and text content |
| Wi-Fi        | SSID, Authentication (Open, WPA, WPA2), Network key  | MIME record `application/vnd.wfa.wsc` (Wi-Fi Simple Configuration) |
| Android app  | Package name                                         | External type record `android.com:pkg` |

Input is validated before a record is added, and problems are reported in the status label.

The SDK's helpers for creating URI and Text records are not used, because they do not check the size of the input before copying it.
The application assembles the payloads itself and uses the SDK to create the records and the message.

#### Adding a record type

Record types are defined by a table in `ptx_ExampleNdefRecords.c`.
To add a new type, define its fields (`NdefFieldDef` array) and its encoder function, which turns the field values into an NDEF record, and add a row to the `RECORD_TYPES` table.
The record type picker and the input form of the *NDEF* tab are generated from the table, no UI changes are needed.

### Polling

The *Polling* tab exposes the RF-discovery settings (`ptx_RUL_DiscoverConfig_t`) used by the application:

| Setting | Range | Meaning |
| ------- | ----- | ------- |
| Idle time between polling cycles | 0 - 10000 ms (default 100) | `IdleTime`, the system idles after every full polling-loop cycle |
| Low power card detection (LPCD) | Disable, Use in every polling cycle, Use in every n-th cycle (n = 2 - 255) | `EnableLPCD`: 0, 1 or n - with n, regular polling is used in every n-th cycle and LPCD in the other cycles |
| Enabled (per technology) | on / off | Poll for Type-A, -B, -F, -V (`PollTypeA`, `PollTypeB`, `PollTypeF212`/`PollTypeF424`, `PollTypeV`) |
| Poll every n-th cycle (per technology) | 1 - 10 (default 1) | `PollType*PollFrequency`, 1 = poll in every cycle |
| Max. devices (per technology) | 1 - 5 (default 5) | `PollType*DeviceLimit`, max. number of cards resolved per cycle, up to `PTX_DEMO_APP_DEVICE_LIMIT_PER_TECH` |
| Bit rate (Type-F) | 212 or 424 kbit/s (default 212) | `PollTypeF212` or `PollTypeF424`, mutually exclusive |

By default, all technologies are polled in every cycle with up to 5 devices each, and LPCD is disabled.
The poll mode is always NFC Forum (`ptx_RUL_PollMode_NfcForum`), polling is always enabled and Listen-mode (card emulation) is always disabled, these settings are not shown.
At least one technology has to stay enabled, and LPCD requires an idle time of at least 1 ms (the SDK rejects LPCD without idle time).
RESTORE DEFAULTS resets all settings.

The settings are used when discovery is started the next time (START, READ or WRITE); they cannot be changed while discovery is running, as the tabs are locked.
The settings are not stored, the defaults are used whenever the application is launched.

## The UI

The main window has three tabs, *Discovery*, *NDEF* and *Polling*, and a menu button.
The *Discovery* tab has the START/STOP and RESET buttons, the number of detected cards and a table with the information of all detected cards.
The *NDEF* tab has a read section with the READ, STOP and CLEAR buttons on top and a text box for the NDEF message, and a write section with the WRITE button, the record type picker and the ADD, REMOVE and CLEAR buttons on top.
The write section also has the input fields of the selected record type, the list of records, the raw bytes of the message and a status label.
The *Polling* tab has the general discovery settings (idle time, LPCD), a table with the settings per RF-technology and the RESTORE DEFAULTS button.
There's also a window for the preferences and the about information, which can be opened from the menu.

Menu items:
- Preferences (Ctrl+P) - access the settings.
- About - show information about the application.
- Quit (Ctrl+Q) - close the application.

## Licensing and Compliance

### Application License
The PTX GTK4 Discovery Application is licensed under the **BSD-3-Clause License**.

### Third-Party Dependencies
This application links against **GTK4** and related libraries (**GLib**, **Gio**), which are licensed under the **LGPL v2.1 or later**.
The NDEF reading and writing features do not introduce additional third-party dependencies.

### License Files
- **`COPYING`**: Contains the full LGPL v2.1 license text
- **`COPYING.LESSER`**: Clarifies the use of the LESSER GPL variant
- **`DEPENDENCIES.md`**: Complete list of all dependencies and their licenses
- **`LGPL_COMPLIANCE.md`**: Detailed compliance statement

### Source Code Availability
The source code for this application is available at:
https://github.com/renesas/ptx-rul-gtk

For source code of LGPL dependencies:
- **GTK4**: https://gitlab.gnome.org/GNOME/gtk
- **GLib/Gio**: https://gitlab.gnome.org/GNOME/glib

### Licensing Questions
For licensing inquiries or questions about proprietary components:
- **Renesas NFC Components**: marketing_nfc_reader@lm.renesas.com
- **LGPL Compliance**: See `LGPL_COMPLIANCE.md` for detailed information

See `license.txt` for a complete summary of all licenses.
