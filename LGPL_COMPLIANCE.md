# LGPL Compliance Statement

## Overview

The PTX GTK4 Discovery Application is a GTK4-based graphical application that links against GTK4, GLib, and Gio libraries, which are licensed under the LGPL v2.1 or later. This document outlines the compliance measures taken to meet LGPL licensing requirements.

## LGPL Compliance Measures

### 1. License Distribution ✓

The following files are included with all distributions of this application:

- **`COPYING`**: Contains the complete text of the GNU Lesser General Public License v2.1
- **`COPYING.LESSER`**: Explicitly clarifies use of the LESSER GPL variant
- **Build System Integration**: CMakeLists.txt automatically copies these files to the build output directory

**LGPL Requirement Met**: Users and distributors have access to the full license text without needing to obtain it separately.

### 2. Dependency Documentation ✓

A comprehensive `DEPENDENCIES.md` file lists:

- All dynamically linked LGPL dependencies (GTK4, GLib, Gio)
- Their source URLs (https://gitlab.gnome.org/GNOME/gtk, etc.)
- License information and license file locations
- Link type (dynamic, not static)
- Information about no modifications to LGPL code
- Contact information for licensing inquiries

**LGPL Requirement Met**: Full transparency about LGPL usage and dependency sources is provided.

### 3. Source Code Availability ✓

This application provides:

- **Source Code Repository**: Available at https://github.com/renesas/ptx-rul-gtk
- **Build Instructions**: Complete CMake-based build system with dependencies documented
- **Modification Capability**: Users can modify, compile, and redistribute with proper attribution
- **Developers Can Recompile**: Users have full access to source and build system to create modified versions

**LGPL Requirement Met**: Source code is available for users who wish to use modified versions of the application.

### 4. Link Type - Dynamic Linking ✓

The application uses **dynamic linking** to GTK4 and related libraries:

- GTK4 is linked via `pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)` in CMakeLists.txt
- Produces dynamic dependencies: `libgtk-4.so`, `libglib-2.0.so`, `libgio-2.0.so`
- Users can verify with: `ldd ./NFC_FORUM_BASIC_DISCOVERY`
- Users can replace/upgrade GTK4 independently without recompiling the application

**LGPL Advantage**: Dynamic linking means the application code itself is not subject to LGPL copyleft requirements. Users can use unmodified LGPL libraries without affecting the application's BSD-3-Clause license.

**LGPL Requirement Met**: Application is not a derivative work of LGPL code; it merely links to it dynamically.

### 5. No Modifications to LGPL Code ✓

The application:

- Uses GTK4, GLib, and Gio unmodified
- Does not embed or fork LGPL code
- Does not create derivative works of LGPL libraries
- Follows standard GTK4 API as provided by upstream projects

**LGPL Requirement Met**: No modifications means no distribution of modified LGPL code is required.

### 6. Application License - BSD-3-Clause ✓

The application code itself is licensed under BSD-3-Clause:

- All source files include SPDX-License-Identifier: BSD-3-Clause
- Copyright: Copyright (c) 2025, Renesas Electronics Corporation
- Full license text included in source file headers
- Users can use, modify, and distribute under BSD-3-Clause terms

**Dual License Clarity**: The application (BSD) and its dependencies (LGPL) are under separate licenses. This is explicitly documented and does not create a conflict.

## For Binary Distributors

If you are packaging or distributing binaries of this application:

### Required Files

Include the following in your distribution package:

```
/usr/bin/NFC_FORUM_BASIC_DISCOVERY          (executable)
/usr/share/applications/                     (desktop shortcut, if applicable)
/usr/share/doc/ptx-rul-gtk/
    ├── COPYING                              (LGPL v2.1 full text)
    ├── COPYING.LESSER                       (LGPL clarification)
    ├── DEPENDENCIES.md                      (dependency list)
    ├── LGPL_COMPLIANCE.md                   (this file)
    ├── README.md                            (build/source information)
    └── LICENSE                              (BSD-3-Clause - optional copy)
```

### For .deb Packages

- Place license files in `/usr/share/doc/ptx-rul-gtk/`
- Include `COPYING` and `COPYING.LESSER` in the copyright file or as separate files
- Run `dpkg-gencontrol` to ensure debian/copyright is properly formatted

### For .rpm Packages

- Place license files in `/usr/share/doc/ptx-rul-gtk/`
- Ensure spec file includes `%doc` entries for license files
- rpm will validate presence of COPYING files

### For Manual Distributions

- Include `COPYING` and `COPYING.LESSER` in the same directory as the executable
- Include `DEPENDENCIES.md` and `README.md` alongside executable
- Ensure source code access information is readily available (in README.md)

## For Developers Modifying This Application

If you modify and redistribute this application:

### What You Must Do

1. **Keep License Headers**: Maintain SPDX-License-Identifier: BSD-3-Clause in all source files
2. **Include COPYING Files**: Your distribution must include `COPYING` and `COPYING.LESSER`
3. **Update DEPENDENCIES.md**: If you add new dependencies, update this file
4. **Provide Source**: Make source code available (via repository or distribution package)
5. **Maintain Attribution**: Include original copyright notices

### What You CAN Do

- Modify the application code freely (BSD-3-Clause allows this)
- Use the modified version commercially (if Renesas proprietary components allow)
- Redistribute under BSD-3-Clause terms
- Link to updated versions of GTK4 without recompilation

### What You CANNOT Do

- Remove license headers from source files
- Redistribute without including `COPYING` and `COPYING.LESSER`
- Modify GTK4, GLib, or Gio and claim they are unmodified
- Change the license of the proprietary Renesas firmware components
- Use proprietary Renesas firmware on non-Renesas hardware

## LGPL Compliance Verification Checklist

After building and before distribution, verify:

- [ ] `COPYING` file exists at repository root
- [ ] `COPYING.LESSER` file exists at repository root
- [ ] `DEPENDENCIES.md` file exists and is up-to-date
- [ ] CMakeLists.txt includes configure_file() commands for COPYING files
- [ ] Build output includes COPYING and COPYING.LESSER in BUILD/ directory
- [ ] Application source code is accessible (repository, source distribution, etc.)
- [ ] Run `ldd ./NFC_FORUM_BASIC_DISCOVERY` shows dynamic linking to libgtk-4.so
- [ ] No GTK4 code is embedded or statically linked
- [ ] README.md includes licensing and source availability information
- [ ] No source files have been removed or renamed without documentation

## Compliance Framework in CMakeLists.txt

The build system enforces compliance:

```cmake
# Automatic LGPL license file copying
configure_file(
  "${CMAKE_SOURCE_DIR}/../../../COPYING"
  "${CMAKE_BINARY_DIR}/COPYING"
  COPYONLY
)
configure_file(
  "${CMAKE_SOURCE_DIR}/../../../COPYING.LESSER"
  "${CMAKE_BINARY_DIR}/COPYING.LESSER"
  COPYONLY
)
configure_file(
  "${CMAKE_SOURCE_DIR}/../../../DEPENDENCIES.md"
  "${CMAKE_BINARY_DIR}/DEPENDENCIES.md"
  COPYONLY
)
```

This ensures compliance is automatic and cannot be accidentally overlooked during packaging.

## Licensing Questions and Inquiries

### For Licensing Questions
- **LGPL Compliance**: See this document and `DEPENDENCIES.md`
- **Application License (BSD-3-Clause)**: See source file headers and `README.md`
- **Proprietary Renesas Components**: Contact marketing_nfc_reader@lm.renesas.com

### For Source Code Requests
- **Application Source**: https://github.com/renesas/ptx-rul-gtk
- **GTK4 Source**: https://gitlab.gnome.org/GNOME/gtk
- **GLib Source**: https://gitlab.gnome.org/GNOME/glib
- **Renesas Components**: Contact marketing_nfc_reader@lm.renesas.com

### For Commercial or Redistribution Inquiries
- **Application Licensing**: See `README.md` for repository contact
- **Renesas Proprietary Components**: marketing_nfc_reader@lm.renesas.com

## Conclusion

This application fully complies with LGPL v2.1 requirements through:

1. Distribution of complete LGPL license text
2. Comprehensive dependency documentation
3. Availability of unmodified source code
4. Dynamic (not static) linking to LGPL libraries
5. Clear separation between application license (BSD) and dependency licenses (LGPL)
6. Automated build-system enforcement of compliance

Users and distributors can confidently use and distribute this application while respecting all applicable open source licenses.

---

**Last Updated**: 2026-10-12
**Compliance Level**: Full LGPL v2.1+ Compliance
