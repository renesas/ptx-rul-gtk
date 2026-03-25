# GTK4 Discovery Application

GTK4 application using the PTX205R and the RUL SDK 1.2.0 reference implementation for the Raspberry Pi and SPI.

## Modificatons made to the SDK example

The project is based on the SDK's `NFC_FORUM_BASIC_DISCOVERY` example.
`CMakeLists.txt` under `PROJECTS/PTX2xxR/NFC_FORUM_BASIC_DISCOVERY` was modified to work with GTK and NDEF reading.
The source and header file under `SRC/EXAMPLES/ptx_Example_NFCForumBasicDiscovery` were modified and unnecessary pieces of application code were removed.
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

Install GTK4 on the Raspberry Pi by installing the `libgtk-4-dev` package.

## Application settings

The app permanently stores preferences using `GSettings`.
The settings are defined in `app.gschema.xml`, and actual settings are stored in `gschemas.compiled`.

Each time new settings are added or settings are removed, the settings file must be compiled again.
In order to do that, run the following command from inside `PROJECTS/PTX2xxR/NFC_FORUM_BASIC_DISCOVERY/BUILD`:
```
glib-compile-schemas .
```

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

## Necessary files for the GTK application

All necessary files for running the application are inside the `BUILD` directory, from where the application must be launched.

- `window.ui` - main window file
- `prefs.ui` - preferences window file
- `menu.ui` - menu file
- `about.ui` - about window file
- `app.gschemas.xml` - settings schema file
- `gschemas.compiled` - the compiled settings file, where the actual settings are stored

## Under the hood

When the app is launched, UI files and settings are loaded, and the PTX is initilized in a separate thread.
If initialization was successful, discovery can be started by clicking the START button.
When discovery is started, the START button changes into the STOP button, and vica-versa.
The discovery loop runs in a separate thread and it polls for RF-technologies A, B, F, V.
If a card is activated, the UI thread is updated and card inforamtion is displayed.
If the card contains an NDEF message, it's displayed as UTF-8 encoded text, without any parsing.
Only one single card in the field is supported.

If the initialization fails, an error message will be displayed and the application must be closed.
If discovery fails to start or the discovery loop ends with an error, an error message will be displayed and the application must be closed.
Some menu items are disabled until the initialization is successfully finished.
If the appliaction is closed, the PTX is de-initialized (if discovery was running, it will stop).

## The UI

The mian window displays the START/STOP button, a text box for the card information, and a menu button.
There's also a window for the preferences and the about information, which can be opened from the menu.

Menu items:
- Reset - displayed card informaiton is cleared and discovery is restarted.
- Preferences - enable/disable dark mode.
- About - display information about the application.
- Quit - close the applicatoin.

