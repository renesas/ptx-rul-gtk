#    ---------------------------------------------------------------
#    SPDX-License-Identifier: BSD-3-Clause
#
#    Copyright (c) 2026, Renesas Electronics Corporation and/or its affiliates
#
#
#    Redistribution and use in source and binary forms, with or without modification,
#    are permitted provided that the following conditions are met:
#
#    1. Redistributions of source code must retain the above copyright notice, this
#       list of conditions and the following disclaimer.
#
#    2. Redistributions in binary form must reproduce the above copyright notice, this list of
#       conditions and the following disclaimer in the documentation and/or other
#       materials provided with the distribution.
#
#    3. Neither the name of Renesas nor the names of its
#       contributors may be used to endorse or promote products derived from this
#       software without specific prior written permission.
#
#
#
#    THIS SOFTWARE IS PROVIDED BY Renesas "AS IS" AND ANY EXPRESS
#    OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#    OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
#    DISCLAIMED. IN NO EVENT SHALL RENESAS OR CONTRIBUTORS BE
#    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
#    GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
#    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
#    OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#    ---------------------------------------------------------------

cmake_minimum_required(VERSION 3.15)

project("PTX205R_NFC_EXPLORER")

# Use pkg-config to locate GTK 4
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

# NOTE: GTK4 is LGPL v2.1 or later
# This dynamically-linked dependency requires LGPL compliance:
# - COPYING and COPYING.LESSER must be included in distributions
# - Source code availability must be provided or documented
# - See DEPENDENCIES.md and LGPL_COMPLIANCE.md for complete information

# General Configuration
# ---------------------

# Platform the sources are compiled for
#
# Currently there are two different platforms available:
#
# * LIN ... Compiles the target for Native Linux.
#
# * WIN ... Compiles the target for Native Windows.
#
# Only one platform is allowed during compilation! Please select the platform accordingly.
#
option(PTX_BUILD_WIN  "Native Windows build" OFF)
option(PTX_BUILD_LIN  "Native Linux build" OFF)

# Interface options for the PTX2xx project
#
# The project can be configured to communicate with the chip via the following interfaces:
#
# * SPI		... Compiles the target for SPI.
#
# * I2C		... Compiles the target for I2C.
#
# * UART	... Compiles the target for UART.
#
# Only one interface is allowed during compilation! Please select the interface accordingly.
#
option(PTX_ENABLE_SPI  "Enable SPI driver" OFF)
option(PTX_ENABLE_I2C  "Enable I2C driver" OFF)
option(PTX_ENABLE_UART "Enable UART driver" OFF)


# Logging configuration
#
# This configuration allows logging to be turned on and off. In the default configuration
# logging is turned off. Currently the logger supports the following options:
#
# * None 	... This is the default configuration, and logging is disabled for the project
#
# * Console ... Enables logging to the console where the program is run.
#               This logging option is only available if either PTX_BUILD_LIN or PTX_BUILD_WIN is set to ON!
#
# * File 	... Enables logging to a file where the program is run.
#               This logging option is only available if either PTX_BUILD_LIN or PTX_BUILD_WIN is set to ON!
#
set(PTX_LOGGING "None" CACHE STRING "Log Variant Definition")
set_property(CACHE PTX_LOGGING PROPERTY STRINGS None Console File)

# Utility Print configuration 
#
# This configuration allows to configure on which interface, the Utility Print module should direct its output. It is a 
# simplified Logging module to easily print formatted strings to an output. In default configuration, it is deactivated
# and set to None. Currently the utility print module supports the following options:
#
# * None 	... This is the default configuration, and printing to any output is disabled for the project.
#
# * Console ... Enables printing to the console where the program is run.
#               This printing option is only available if either PTX_BUILD_LIN or PTX_BUILD_WIN is set to ON!
#
set(PTX_UTILITY_PRINTING "None" CACHE STRING "Print Variant Definition")
set_property(CACHE PTX_UTILITY_PRINTING PROPERTY STRINGS None Console)

# Project Configuration
# ---------------------

# Logger configuration
if (${PTX_LOGGING} STREQUAL "None")
	set(PTX_LOGGING_DIR "NONE")
	set(PTX_LOGGING_FILE "None")
elseif(${PTX_LOGGING} STREQUAL "Console" AND (PTX_BUILD_LIN OR PTX_BUILD_WIN))
	set(PTX_LOGGING_DIR "CONSOLE")
	set(PTX_LOGGING_FILE "Console")
elseif(${PTX_LOGGING} STREQUAL "File" AND (PTX_BUILD_LIN OR PTX_BUILD_WIN))
	set(PTX_LOGGING_DIR "FILE")
	set(PTX_LOGGING_FILE "File")
else()
	message(FATAL_ERROR "Could not configure logger, due to invalid parameters! Please make sure that the parameters are set properly!")
endif()

# Utility Print configuration
if(${PTX_UTILITY_PRINTING} STREQUAL "Console" AND (PTX_BUILD_LIN OR PTX_BUILD_WIN))
	add_definitions(-DPTX_UTILITY_PRINTING_CONSOLE)
else()
	message(STATUS "No explicit Utility Print configuration was set! None variant is used and all printing is disabled!")
endif()

# Interface configuration
if (PTX_ENABLE_SPI)
	add_definitions(-DPTX_ENABLE_SPI)
	set(PTX_HAL_INTERFACE "SPI")
elseif(PTX_ENABLE_I2C)
	add_definitions(-DPTX_ENABLE_I2C)
	set(PTX_HAL_INTERFACE "I2C")
elseif(PTX_ENABLE_UART)
	add_definitions(-DPTX_ENABLE_UART)
	set(PTX_HAL_INTERFACE "UART")
else()
	message(FATAL_ERROR "No HAL driver selected! Please select the proper driver (SPI, I2C, or UART)!")
endif()


# To activate a build type configuration, add '-DCMAKE_BUILD_TYPE={Debug, Release, RelWithDebInfo, MinSizeRel}' as an argument to the cmake configuration command.
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Debug")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Platform configuration
if(PTX_BUILD_LIN)
	add_definitions(-DPTX_PLATFORM_OS)
    add_definitions("-DPTX_APP_DIVERSITY_LINUX")
	set(PTX_PLATFORM "LIN")
	   
    set(OBJECT_GEN_FLAGS "-fPIC -Wall -Werror -g -O0 -Wconversion -Wunused -Wuninitialized -Wvla -Wextra -Wmissing-declarations -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal")
    set(CMAKE_C_FLAGS   "${OBJECT_GEN_FLAGS} -std=gnu99 -MMD -MP" CACHE INTERNAL "C Compiler options")
    set(CMAKE_CXX_FLAGS "${OBJECT_GEN_FLAGS} -std=c++11 " CACHE INTERNAL "C++ Compiler options")
    set(CMAKE_EXE_LINKER_FLAGS "${OBJECT_GEN_FLAGS} -Wl,-Map=${CMAKE_PROJECT_NAME}.map" CACHE INTERNAL "Linker options")
	link_libraries(-lm)
	
	set(SOURCES ${SOURCES}
		../../../SRC/COMPS/OSAL/LIN/ptx_OSAL.c
	)
	
	set(HEADER_DIRS
		../../../SRC/COMPS/OSAL/
		../../../SRC/COMPS/OSAL/LIN/
	)

elseif(PTX_BUILD_WIN)
	add_definitions(-DPTX_PLATFORM_OS)
    add_definitions("-DPTX_APP_DIVERSITY_WIN")
	set(PTX_PLATFORM "WIN")
	
    set(OBJECT_GEN_FLAGS "-fPIC -Wall -Werror -g -O0 -Wconversion -Wunused -Wuninitialized -Wvla -Wextra -Wmissing-declarations -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal")
    set(CMAKE_C_FLAGS   "${OBJECT_GEN_FLAGS} -std=gnu99 -MMD -MP" CACHE INTERNAL "C Compiler options")
    set(CMAKE_CXX_FLAGS "${OBJECT_GEN_FLAGS} -std=c++11 " CACHE INTERNAL "C++ Compiler options")
    set(CMAKE_EXE_LINKER_FLAGS "${OBJECT_GEN_FLAGS} -Wl,-Map=${CMAKE_PROJECT_NAME}.map" CACHE INTERNAL "Linker options")

	set(SOURCES ${SOURCES}
		../../../SRC/COMPS/HAL/DRV/WIN/FTD2XX/ptx_HAL_Ftd2xx.c
		../../../SRC/COMPS/HAL/DRV/WIN/UTILS/ptx_HAL_WinUtils.c
		../../../SRC/COMPS/OSAL/WIN/ptx_OSAL.c
	)
	
	set(HEADER_DIRS
		../../../SRC/COMPS/HAL/DRV/WIN/FTD2XX
		../../../SRC/COMPS/HAL/DRV/WIN/UTILS
		../../../SRC/COMPS/OSAL/
		../../../SRC/COMPS/OSAL/WIN/
	)

else()
	message(FATAL_ERROR "No platform selected! Please select the proper platform (Linux or Windows)!")
endif()

# Configuration of source and header directories used for the project
set(SOURCES
${SOURCES}
	../../../SRC/COMPS/LOG/ptx_Log.c
	../../../SRC/COMPS/LOG/zf_log.c
	../../../SRC/COMPS/LOG/${PTX_LOGGING_DIR}/ptx_Log_${PTX_LOGGING_FILE}.c
	../../../SRC/COMPS/UTILS/ptx_RingBuffer.c
	../../../SRC/COMPS/HAL/DRV/${PTX_PLATFORM}/ptx_HAL_DRV_Timer.c
	../../../SRC/COMPS/HAL/DRV/${PTX_PLATFORM}/ptx_HAL_DRV_GPIO.c
	../../../SRC/COMPS/HAL/DRV/${PTX_PLATFORM}/ptx_HAL_DRV_MCU.c
	../../../SRC/COMPS/HAL/DRV/${PTX_PLATFORM}/ptx_HAL_DRV_Interface_${PTX_HAL_INTERFACE}.c
	../../../SRC/COMPS/HAL/ptx_HAL.c
	../../../SRC/COMPS/HAL/ptx_HAL_Interface.c
	../../../SRC/COMPS/HAL/ptx_HAL_Timer.c
	../../../SRC/COMPS/HAL/ptx_HAL_GPIO.c
	../../../SRC/COMPS/HAL/ptx_HAL_Interface_${PTX_HAL_INTERFACE}.c
	../../../SRC/COMPS/HIP/RUL/ptx_HIP_Frame.c
	../../../SRC/COMPS/HIP/ptx_HIP.c
	../../../SRC/COMPS/NSC/UCODE/ptx_NSC_uCODE.c
	../../../SRC/COMPS/NSC/ptx_NSC_StateMachine.c
	../../../SRC/COMPS/NSC/ptx_NSC.c
	../../../SRC/COMPS/NSC/ptx_NSC_CmdProcessor.c
	../../../SRC/COMPS/NSC/ptx_NSC_Data.c
	../../../SRC/COMPS/NSC/ptx_NSC_Firmware.c
	../../../SRC/COMPS/NSC/ptx_NSC_InitHw.c
	../../../SRC/COMPS/NSC/ptx_NSC_RfActivate.c
	../../../SRC/COMPS/NSC/ptx_NSC_RfConfig.c
	../../../SRC/COMPS/NSC/ptx_NSC_RfDeactivate.c
	../../../SRC/COMPS/NSC/ptx_NSC_RfDiscover.c
	../../../SRC/COMPS/NSC/ptx_NSC_RfDiscover_Poll.c
	../../../SRC/COMPS/NSC/ptx_NSC_RfSetParameter.c
	../../../SRC/COMPS/NSC/ptx_NSC_SystemConfig.c
	../../../SRC/COMPS/NSC/ptx_NSC_Error.c
	../../../SRC/COMPS/NSC/ptx_NSC_RfSetParameter.c
	../../../SRC/COMPS/NSC/ptx_NSC_RfTest.c
	../../../SRC/COMPS/NSC/ptx_NSC_RevisionInfo.c
	../../../SRC/COMPS/NSC/ptx_NSC_RfPresenceCheck.c
	../../../SRC/COMPS/NSC/ptx_NSC_Misc.c
	../../../SRC/COMPS/NSC/ptx_NSC_Standby.c
	../../../SRC/COMPS/NSC/ptx_NSC_Wakeup.c
	../../../SRC/COMPS/NSC/ptx_NSC_Wlc.c
	../../../SRC/COMPS/NSC/ptx_NSC_RfDiscover_Listen.c
	../../../SRC/COMPS/NSC/ptx_NSC_RfSetRoutingTable.c
	../../../SRC/COMPS/NSC/RUL/ptx_NSC_RUL.c
	../../../SRC/COMPS/NSC/RUL/ptx_NSC_RUL_RfConfig.c
	../../../SRC/COMPS/NSC/RUL/ptx_NSC_RUL_MsgProcessing.c
	../../../SRC/COMPS/NSC/RUL/ptx_NSC_RfConfigVal.c
	../../../SRC/API/RUL/ptx_RUL.c
	../../../SRC/API/RUL/ptx_RUL_Discovery.c
	../../../SRC/API/RUL/ptx_RUL_CardHandler.c
	../../../SRC/API/RUL/ptx_RUL_DataProcessor.c
	../../../SRC/API/RUL/ptx_RUL_Extension.c
	../../../SRC/API/RUL_HCE/ptx_RUL_HCE.c
	../../../SRC/EXAMPLES/Utils/ptx_Utils_Print.c
	../../../SRC/EXAMPLES/Utils/ptx_Utils_CardInfo.c
	../../../SRC/EXAMPLES/Utils/ptx_Utils_VersionInfo.c
	../../../SRC/EXAMPLES/Utils/ptx_Utils_AppParameters.c
	
	../../../SRC/API/NATIVE_TAG/T2T/ptx_NativeTag_T2T.c
	../../../SRC/API/NDEF/TAG_TYPES/NDEF_T2T/ptx_NDEF_T2T.c
	../../../SRC/API/NATIVE_TAG/T3T/ptx_NativeTag_T3T.c
	../../../SRC/API/NDEF/TAG_TYPES/NDEF_T3T/ptx_NDEF_T3T.c
	../../../SRC/API/NATIVE_TAG/T4T/ptx_NativeTag_T4T.c
	../../../SRC/API/NDEF/TAG_TYPES/NDEF_T4T/ptx_NDEF_T4T.c
	../../../SRC/API/NATIVE_TAG/T5T/ptx_NativeTag_T5T.c
	../../../SRC/API/NDEF/TAG_TYPES/NDEF_T5T/ptx_NDEF_T5T.c
	../../../SRC/API/NDEF/GENERIC/ptx_NDEF.c
	../../../SRC/API/NDEF/RECORDS/ptx_NDEFRecord.c
	../../../SRC/API/NDEF/RECORDS/ptx_NDEFRecord_URI.c
	../../../SRC/API/NDEF/RECORDS/ptx_NDEFMessage.c
)

set(HEADER_DIRS
${HEADER_DIRS}
	../../../SRC/COMPS/NSC
	../../../SRC/COMPS/NSC/UCODE
	../../../SRC/COMPS
	../../../SRC/COMPS/HAL/DRV
	../../../SRC/COMPS/HIP
	../../../SRC/API/RUL
	../../../SRC/COMPS/LOG
	../../../SRC/API
	../../../SRC/COMPS/NSC/RUL
	../../../SRC/COMPS/HAL
	../../../SRC/EXAMPLES
	../../../SRC/EXAMPLES/Utils
	../../../SRC/COMPS/HAL/DRV/${PTX_PLATFORM}
	../../../SRC/COMPS/LOG/${PTX_LOGGING_DIR}
	../../../SRC/COMPS/UTILS
	../../../SRC/COMPS/HIP/RUL

	../../../SRC/API/NATIVE_TAG
	../../../SRC/API/NDEF
	../../../SRC/API/NDEF/TAG_TYPES/NDEF_T3T
	../../../SRC/API/NATIVE_TAG/T4T
	../../../SRC/API/NDEF/TAG_TYPES/NDEF_T5T
	../../../SRC/API/NATIVE_TAG/T3T
	../../../SRC/API/NATIVE_TAG/T5T
	../../../SRC/API/NDEF/TAG_TYPES/NDEF_T4T
	../../../SRC/API/NDEF/TAG_TYPES
	../../../SRC/API/NDEF/TAG_TYPES/NDEF_T2T
	../../../SRC/API/NDEF/DEFINES
	../../../SRC/API/NATIVE_TAG/T2T
	../../../SRC/API/NDEF/GENERIC
	../../../SRC/API/NDEF/RECORDS
	../../../SRC/API/RUL_HCE
)

# Build Target Configuration
add_library(RUL SHARED)

target_sources(RUL PRIVATE ${HEADER_DIRS} ${SOURCES})
target_include_directories(RUL PUBLIC ${HEADER_DIRS})

set(TARGET_EXE_NAME "ptx205r-nfc-explorer")
add_executable(${TARGET_EXE_NAME})
target_sources(${TARGET_EXE_NAME} PRIVATE
	../../../SRC/EXAMPLES/ptx_NFCExplorer/ptx_ExampleApplication.c
	../../../SRC/EXAMPLES/ptx_NFCExplorer/ptx_ExampleNdefRecords.c
	../../../SRC/EXAMPLES/ptx_NFCExplorer/HCE_T4T/ptx_HCET4T.c
)

# link GTK
target_link_libraries(${TARGET_EXE_NAME} PRIVATE PkgConfig::GTK4)

# Application sources: UI files, GSettings schema, desktop integration
set(APP_DIR "${CMAKE_SOURCE_DIR}/../../../SRC/EXAMPLES/ptx_NFCExplorer")

# Embed the UI files (UI/*.ui) into the executable as a GResource
pkg_get_variable(GLIB_COMPILE_RESOURCES gio-2.0 glib_compile_resources)
pkg_get_variable(GLIB_COMPILE_SCHEMAS gio-2.0 glib_compile_schemas)
set(APP_RESOURCE_XML "${APP_DIR}/UI/ptx_App.gresource.xml")
set(APP_RESOURCE_SRC "${CMAKE_BINARY_DIR}/ptx_App_resources.c")
set(APP_UI_DIR "${APP_DIR}/UI")
set(APP_UI_FILES ${APP_UI_DIR}/window.ui ${APP_UI_DIR}/menu.ui ${APP_UI_DIR}/prefs.ui ${APP_UI_DIR}/about.ui)
add_custom_command(
	OUTPUT "${APP_RESOURCE_SRC}"
	COMMAND ${GLIB_COMPILE_RESOURCES} --target=${APP_RESOURCE_SRC} --sourcedir=${APP_UI_DIR} --generate-source ${APP_RESOURCE_XML}
	DEPENDS ${APP_RESOURCE_XML} ${APP_UI_FILES}
	COMMENT "Compiling UI resources"
)
target_sources(${TARGET_EXE_NAME} PRIVATE "${APP_RESOURCE_SRC}")

# Compile the GSettings schema into BUILD/schemas (the executable looks there at runtime)
set(APP_SCHEMA_XML "${APP_DIR}/DATA/com.renesas.nfc.gschema.xml")
set(APP_SCHEMA_DIR "${CMAKE_BINARY_DIR}/schemas")
add_custom_command(
	OUTPUT "${APP_SCHEMA_DIR}/gschemas.compiled"
	COMMAND ${CMAKE_COMMAND} -E make_directory ${APP_SCHEMA_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy ${APP_SCHEMA_XML} ${APP_SCHEMA_DIR}/
	COMMAND ${GLIB_COMPILE_SCHEMAS} ${APP_SCHEMA_DIR}
	DEPENDS ${APP_SCHEMA_XML}
	COMMENT "Compiling GSettings schema"
)
add_custom_target(app_schemas ALL DEPENDS "${APP_SCHEMA_DIR}/gschemas.compiled")
add_dependencies(${TARGET_EXE_NAME} app_schemas)

target_link_libraries(${TARGET_EXE_NAME} PRIVATE RUL)

# LGPL Compliance: Copy license files to build directory
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

# AppImage / install layout (see tools/appimage/build-appimage.sh)
set_target_properties(${TARGET_EXE_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN/../lib")
install(TARGETS ${TARGET_EXE_NAME} RUNTIME DESTINATION bin)
install(TARGETS RUL LIBRARY DESTINATION lib)
install(FILES "${APP_SCHEMA_XML}" DESTINATION share/glib-2.0/schemas)
install(FILES "${CMAKE_SOURCE_DIR}/../../../COPYING" "${CMAKE_SOURCE_DIR}/../../../COPYING.LESSER" "${CMAKE_SOURCE_DIR}/../../../DEPENDENCIES.md"
	DESTINATION share/doc/ptx205r-nfc-explorer)
install(FILES "${APP_DIR}/DATA/com.renesas.nfc.desktop" DESTINATION share/applications)
install(FILES "${APP_DIR}/DATA/com.renesas.nfc.png" DESTINATION share/icons/hicolor/256x256/apps)

target_include_directories(${TARGET_EXE_NAME} PUBLIC 
	../../../SRC/EXAMPLES
	../../../SRC/EXAMPLES/ptx_NFCExplorer
	../../../SRC/EXAMPLES/ptx_NFCExplorer/HCE_T4T

)

set_target_properties("${TARGET_EXE_NAME}"
	PROPERTIES
		OUTPUT_NAME "${TARGET_EXE_NAME}"
)
