# GTK4 Discovery Application

GTK4 application using the PTX205R and the RUL SDK 1.2.1 reference implementation for the Raspberry Pi and SPI.

## Modificatons made to the SDK example

The project is based on the SDK's `NFC_FORUM_BASIC_DISCOVERY` example.
`CMakeLists.txt` under `PROJECTS/PTX2xxR/NFC_FORUM_BASIC_DISCOVERY` was modified to work with GTK and NDEF reading.
The source and header files under `SRC/EXAMPLES/ptx_Example_NFCForumBasicDiscovery` were modified and unnecessary pieces of application code were removed.
The basic structure of the SDK deliverable was kept, only unnecessary project and example files and directories were removed.

## Hardware requiriements
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

The appliactoin permanently stores preferences using `GSettings`.
The settings are defined in `app.gschema.xml`, and actual settings are stored in `gschemas.compiled`.
Both files are located in the `BUILD` directory.

Each time settings are modified, the settings file must be compiled again, by runnig the following command from the `BUILD` directory:
```
glib-compile-schemas .
```
For the sake of demonstration, there's one setting implemented, which is a switch to enable or disable dark mode.
It can be accessed by clicking the menu button and *Preferences*.
The state of the swithc is bound to the setting stored in `gschemas.compiled`.
When the switch is changed, `gschemas.compiled` is updated.
Upon launching the application, the setting is retrieved from `gschemas.compiled` and applied.
When opening the *Preferences* window, the state of the swithc is set according to the setting in `gschemas.compiled`.
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

The appliactoin can also be launched by double-clinking the executable.
If the appliactoin is run from the command line, information will be printed there.

## Necessary files for running the application

All necessary files for running the application are inside the `BUILD` directory, from where the it must be launched.

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

The settings schema file `app.gschemas.xml` is only reqired for compiling the settings file once, not for running the application.

## Under the hood

When the application is launched, UI files and settings are loaded, and the PTX is initilized in a separate thread.
If initialization was successful, discovery can be started by clicking the START button.
When discovery is started, the START button becomes the STOP button, and vica-versa.
The discovery loop runs in a separate thread and it polls for RF-technologies A, B, F, V.
If a card is activated, the UI thread is updated and card inforamtion is displayed.
If the card contains an NDEF message, it's displayed as UTF-8 encoded text, without any parsing.
Only one single card in the field is supported.

If the initialization fails, an error message will be displayed and the application must be closed.
If discovery fails to start or the discovery loop ends with an error, an error message will be displayed and the application must be closed.
Some menu items are disabled until the initialization is successfully finished.
If the appliaction is closed, the PTX is de-initialized - if discovery was running, it will stop.

## The UI

The mian window displays the START/STOP button, a text box for the card information, and a menu button.
There's also a window for the preferences and the about information, which can be opened from the menu.

Menu items:
- Reset (Ctrl+R) - displayed card informaiton is cleared and discovery is restarted.
- Preferences (Ctrl+P) - access the settings.
- About - show information about the application.
- Quit (Ctrl+Q) - close the applicatoin.

## Licensing and Compliance

### Application License
The PTX GTK4 Discovery Application is licensed under the **BSD-3-Clause License**.

### Third-Party Dependencies
This application links against **GTK4** and related libraries (**GLib**, **Gio**), which are licensed under the **LGPL v2.1 or later**.

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
