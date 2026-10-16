# Project Dependencies and Licenses

This document provides a comprehensive list of all dependencies used by the PTX GTK4 Discovery Application, their sources, and their respective licenses.

## Runtime Dependencies (Dynamically Linked)

### GTK4
- **Source**: https://gitlab.gnome.org/GNOME/gtk
- **License**: LGPL v2.1 or later
- **Purpose**: GUI toolkit and application interface
- **License File**: See `COPYING` or `COPYING.LESSER` in this repository
- **Modifications**: None (unmodified dependency)
- **Link Type**: Dynamic linking (libgtk-4.so)

### GLib
- **Source**: https://gitlab.gnome.org/GNOME/glib
- **License**: LGPL v2.1 or later
- **Purpose**: Core data structures and utilities (used directly by the application, e.g. threads, strings, NDEF message handling, and by GTK4)
- **Modifications**: None (unmodified dependency)
- **Link Type**: Dynamic linking (libglib-2.0.so)

### Gio
- **Source**: https://gitlab.gnome.org/GNOME/glib (part of GLib)
- **License**: LGPL v2.1 or later
- **Purpose**: I/O and settings management via GSettings (used directly by the application and by GTK4)
- **Modifications**: None (unmodified dependency)
- **Link Type**: Dynamic linking (libgio-2.0.so)

## Build-Time Dependencies

### CMake
- **Version**: 3.15 or later
- **Source**: https://cmake.org
- **License**: BSD-3-Clause
- **Purpose**: Build system configuration and compilation

### pkg-config
- **Source**: https://www.freedesktop.org/wiki/Software/pkg-config/
- **License**: GNU GPL v2 or later
- **Purpose**: Development tool for detecting libraries (build-time only)

## Embedded/Bundled Components

### zf_log 0.4.0
- **Location**: `SRC/COMPS/LOG/zf_log.c` and `zf_log.h`
- **Source**: https://github.com/wonder-mice/zf_log
- **License**: MIT License
- **Copyright**: Copyright 2017 wonder-mice
- **Purpose**: Logging functionality
- **Modifications**: None (unmodified)
- **License Text**: Embedded in source file header

## Proprietary/Renesas Components

### PTX2xxR/W RUL SDK 1.2.1
- **Source**: Renesas Electronics
- **License**: BSD-3-Clause
- **Copyright**: Copyright (c) 2026, Renesas Electronics Corporation and/or its affiliates
- **Purpose**: NFC reader hardware interface and protocol implementation
- **License File**: Embedded in source code headers
- **Usage**: Core SDK components, not modified

### PTX205R Firmware/uCode Image
- **Source**: Renesas Electronics
- **License**: Renesas Open License v1.0 (proprietary)
- **Copyright**: Renesas Electronics Corporation
- **License File**: `Renesas_Open_License_v1.txt`
- **Purpose**: Firmware for PTX205R NFC reader hardware
- **Restrictions**: 
  - Use only with Renesas integrated circuits
  - No reverse engineering or decompilation
  - Subject to Renesas licensing agreement

## Application License

### ptx-rul-gtk
- **License**: BSD-3-Clause
- **Copyright**: Copyright (c) 2026, Renesas Electronics Corporation and/or its affiliates
- **Source**: https://github.com/renesas/ptx-rul-gtk
- **Purpose**: GTK4 NFC Forum basic discovery demo application with NDEF read and write
- **License Text**: Embedded in all source file headers via SPDX-License-Identifier

## License Compliance Notes

### LGPL Compliance
This application links against GTK4 and related libraries (GLib, Gio) which are licensed under LGPL v2.1 or later. The following compliance measures are in place:

1. **License Distribution**: Full LGPL v2.1 license text is included with distributions (see `COPYING` and `COPYING.LESSER`)
2. **Source Availability**: Application source code is available for compilation and modification
3. **Dynamic Linking**: Libraries are dynamically linked (not statically embedded), allowing users to upgrade or replace GTK4 independently
4. **No Modifications**: GTK4 and related libraries are used unmodified
5. **Dependency Documentation**: This file documents all LGPL and other dependencies

### BSD-3-Clause Compliance
The application code and PTX SDK are licensed under BSD-3-Clause. License text is included in all source file headers via SPDX identifiers.

### Proprietary Component Restrictions
Users must comply with the Renesas Open License v1.0 for the PTX205R firmware. Use is restricted to Renesas integrated circuits and requires execution of a Renesas Electronics Licensing Agreement.

## Source Code Availability

For information on obtaining source code for this application and its dependencies:

- **This Application**: Source available at https://github.com/renesas/ptx-rul-gtk
- **GTK4**: https://gitlab.gnome.org/GNOME/gtk
- **GLib**: https://gitlab.gnome.org/GNOME/glib
- **zf_log**: https://github.com/wonder-mice/zf_log
- **Renesas Components**: Contact marketing_nfc_reader@lm.renesas.com

## Distribution Checklist

When distributing this application, ensure you include:

- [ ] `COPYING` file (LGPL v2.1 full text)
- [ ] `COPYING.LESSER` file (LGPL v2.1 clarification)
- [ ] `DEPENDENCIES.md` file (this file)
- [ ] `LGPL_COMPLIANCE.md` file (compliance statement)
- [ ] `README.md` (build and source information)
- [ ] `Renesas_Open_License_v1.txt` (proprietary firmware license)
- [ ] Application executable and required runtime dependencies

## Questions or Licensing Inquiries

For licensing questions or requests for source code of proprietary components:

- **Renesas Components**: marketing_nfc_reader@lm.renesas.com
- **Application Source**: See README.md for repository URL
- **LGPL Compliance**: See `LGPL_COMPLIANCE.md` for detailed compliance information
