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

project("NFC_FORUM_BASIC_DISCOVERY")

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
# Currently there are three different platforms available:
#
# * MCU ... Compiles the target for the MCU Platform. The default platform is the RA4M2 platform.
#           Please make sure that all the Renesas FSP sources are generated before attempting to
#           configure and compile the project.
#
# * LIN ... Compiles the target for Native Linux.
#
# * WIN ... Compiles the target for Native Windows.
#
# Only one platform is allowed during compilation! Please select the platform accordingly.
#
option(PTX_BUILD_MCU  "Build for MCU (RA4M2) platform" OFF)
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
# Be aware, that PTX_LOGGING and PTX_UTILITY_PRINTING cannot be both set to "Serial", as both use the same UART interface 
# for the RA4M2 reference implementation.
#
# * None 	... This is the default configuration, and logging is disabled for the project
#
# * Serial 	... Enables serial logging. It is only available for MCU based projects.
#              	To enable serial logging make sure that PTX_BUILD_MCU is set to ON!
#
# * Console ... Enables logging to the console where the program is run.
#               This logging option is only available if either PTX_BUILD_LIN or PTX_BUILD_WIN is set to ON!
#
# * File 	... Enables logging to a file where the program is run.
#               This logging option is only available if either PTX_BUILD_LIN or PTX_BUILD_WIN is set to ON!
#
set(PTX_LOGGING "None" CACHE STRING "Log Variant Definition")
set_property(CACHE PTX_LOGGING PROPERTY STRINGS None Serial Console File)

# Utility Print configuration 
#
# This configuration allows to configure on which interface, the Utility Print module should direct its output. It is a 
# simplified Logging module to easily print formatted strings to an output. In default configuration, it is deactivated
# and set to None. Currently the utility print module supports the following options:
#
# Be aware, that PTX_LOGGING and PTX_UTILITY_PRINTING cannot be both set to "Serial", as both use the same UART interface 
# for the RA4M2 reference implementation.
#
# * None 	... This is the default configuration, and printing to any output is disabled for the project.
#
# * Serial 	... Enables serial printing. It is only available for MCU based projects.
#              	To enable serial printing make sure that PTX_BUILD_MCU is set to ON!
#
# * Console ... Enables printing to the console where the program is run.
#               This printing option is only available if either PTX_BUILD_LIN or PTX_BUILD_WIN is set to ON!
#
set(PTX_UTILITY_PRINTING "None" CACHE STRING "Print Variant Definition")
set_property(CACHE PTX_UTILITY_PRINTING PROPERTY STRINGS None Serial Console)

# Project Configuration
# ---------------------

# Board configuration
set(PTX_MCU_DIR "MCU/CUSTOM")

# Logger configuration
if (${PTX_LOGGING} STREQUAL "None")
	set(PTX_LOGGING_DIR "NONE")
	set(PTX_LOGGING_FILE "None")
elseif(${PTX_LOGGING} STREQUAL "Serial" AND PTX_BUILD_MCU AND (NOT (${PTX_UTILITY_PRINTING} STREQUAL "Serial")))
	set(PTX_LOGGING_DIR "SERIAL")
	set(PTX_LOGGING_FILE "Serial")
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
if(${PTX_UTILITY_PRINTING} STREQUAL "Serial" AND PTX_BUILD_MCU AND (NOT (${PTX_LOGGING} STREQUAL "Serial")))
	add_definitions(-DPTX_UTILITY_PRINTING_SERIAL)
elseif(${PTX_UTILITY_PRINTING} STREQUAL "Console" AND (PTX_BUILD_LIN OR PTX_BUILD_WIN))
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
if (PTX_BUILD_MCU)
	add_definitions(-DPTX_PLATFORM_MCU)
    add_definitions("-DPTX_APP_DIVERSITY_RA4M2")
	set(PTX_PLATFORM "MCU/RA4M2")
	
	set(OBJECT_GEN_FLAGS "-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -O2 -fmessage-length=0 -fsigned-char -Wunused -Wuninitialized -Wall -Wextra -Wvla -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Werror -Wlogical-op -Waggregate-return -Wfloat-equal -g -D_RENESAS_RA_ ")

	if (NOT (${CMAKE_BUILD_TYPE} STREQUAL "Debug"))
		set(OBJECT_GEN_FLAGS "${OBJECT_GEN_FLAGS} -ffunction-sections -fdata-sections")
	endif()

	set(CMAKE_C_FLAGS   "${OBJECT_GEN_FLAGS} -std=gnu99 -MMD -MP" CACHE INTERNAL "C Compiler options")
	set(CMAKE_CXX_FLAGS "${OBJECT_GEN_FLAGS} -std=c++11 " CACHE INTERNAL "C++ Compiler options")
	set(CMAKE_ASM_FLAGS "${OBJECT_GEN_FLAGS} -x assembler-with-cpp " CACHE INTERNAL "ASM Compiler options")

	set(CMAKE_EXE_LINKER_FLAGS "-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -O0 -fmessage-length=0 -fsigned-char -Wunused -Wuninitialized -Wall -Wextra -Wvla -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g -T fsp.ld -Xlinker --gc-sections -L${CMAKE_SOURCE_DIR}/MCU/CUSTOM/script/ -L${CMAKE_SOURCE_DIR}/MCU/CUSTOM/Debug-SPI-NONE -L${CMAKE_SOURCE_DIR}/MCU/CUSTOM/Debug-SPI-SERIAL -L${CMAKE_SOURCE_DIR}/MCU/CUSTOM/Release-SPI-NONE -L${CMAKE_SOURCE_DIR}/MCU/CUSTOM/Release-SPI-SERIAL -L${CMAKE_SOURCE_DIR}/MCU/CUSTOM/Debug-I2C-NONE -L${CMAKE_SOURCE_DIR}/MCU/CUSTOM/Debug-I2C-SERIAL -L${CMAKE_SOURCE_DIR}/MCU/CUSTOM/Release-I2C-NONE -L${CMAKE_SOURCE_DIR}/MCU/CUSTOM/Release-I2C-SERIAL -L${CMAKE_SOURCE_DIR}/MCU/CUSTOM/Debug-UART-NONE -L${CMAKE_SOURCE_DIR}/MCU/CUSTOM/Debug-UART-SERIAL -L${CMAKE_SOURCE_DIR}/MCU/CUSTOM/Release-UART-NONE -L${CMAKE_SOURCE_DIR}/MCU/CUSTOM/Release-UART-SERIAL -Wl,-Map=${CMAKE_PROJECT_NAME}.map --specs=rdimon.specs --specs=nano.specs " CACHE INTERNAL "Linker options")
	
	if (NOT (${CMAKE_BUILD_TYPE} STREQUAL "Debug"))
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ffunction-sections -fdata-sections")
	endif()

	macro(HEADER_DIRECTORIES dir return_list)
		file(GLOB_RECURSE new_list "${dir}/*.h")
		set(dir_list "")
		foreach(file_path ${new_list})
			get_filename_component(dir_path ${file_path} PATH)
			set(dir_list ${dir_list} ${dir_path})
		endforeach()
		list(REMOVE_DUPLICATES dir_list)
		set(${return_list} ${dir_list})
	endmacro()

	file(GLOB_RECURSE RA_SRCS "${PTX_MCU_DIR}/ra/*.c")
	file(GLOB_RECURSE RA_CFG_SRCS "${PTX_MCU_DIR}/ra_cfg/*.c")
	file(GLOB_RECURSE RA_GEN_SRCS "${PTX_MCU_DIR}/ra_gen/*.c")
	file(GLOB_RECURSE SRCS "${PTX_MCU_DIR}/src/*.c")

	file(GLOB_RECURSE RA_HDRS "${PTX_MCU_DIR}/ra/*.h")
	file(GLOB_RECURSE RA_CFG_HDRS "${PTX_MCU_DIR}/ra_cfg/*.h")
	file(GLOB_RECURSE RA_GEN_HDRS "${PTX_MCU_DIR}/ra_gen/*.h")
	file(GLOB_RECURSE HDRS "${PTX_MCU_DIR}/src/*.h")
	file(GLOB_RECURSE BUILD_HDRS "${PTX_MCU_DIR}/${CMAKE_BUILD_TYPE}-${PTX_HAL_INTERFACE}-${PTX_LOGGING_DIR}/bsp_linker_info.h")

	HEADER_DIRECTORIES("${PTX_MCU_DIR}/ra" RA_HDR_DIRS)
	HEADER_DIRECTORIES("${PTX_MCU_DIR}/ra_cfg" RA_CFG_HDR_DIRS)
	HEADER_DIRECTORIES("${PTX_MCU_DIR}/ra_gen" RA_GEN_HDR_DIRS)
	HEADER_DIRECTORIES("${PTX_MCU_DIR}/src" SRC_HDR_DIRS)
	HEADER_DIRECTORIES("${PTX_MCU_DIR}/${CMAKE_BUILD_TYPE}-${PTX_HAL_INTERFACE}-${PTX_LOGGING_DIR}" BUILD_HDR_DIRS)

	set(SOURCES ${SOURCES}
		
	)

	

elseif(PTX_BUILD_LIN)
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
	message(FATAL_ERROR "No platform selected! Please select the proper platform (RA4M2, Linux, or Windows)!")
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
	../../../SRC/COMPS/HAL/DRV/MCU

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
if (PTX_BUILD_MCU)
	add_library(RA4M2_NFC_FORUM_BASIC_DISCOVERY OBJECT ${RA_SRCS} ${RA_CFG_SRCS} ${RA_GEN_SRCS} ${SRCS})
	target_sources(RA4M2_NFC_FORUM_BASIC_DISCOVERY INTERFACE ${RA_HDRS} ${RA_CFG_HDRS} ${RA_GEN_HDRS} ${SRC_HDRS} ${BUILD_HDRS})
	target_include_directories(RA4M2_NFC_FORUM_BASIC_DISCOVERY PUBLIC ${RA_HDR_DIRS} ${RA_CFG_HDR_DIRS} ${RA_GEN_HDR_DIRS} ${SRC_HDR_DIRS} ${BUILD_HDR_DIRS})
	add_library(RUL STATIC)
else()
	add_library(RUL SHARED)
endif()

target_sources(RUL PRIVATE ${HEADER_DIRS} ${SOURCES})
target_include_directories(RUL PUBLIC ${HEADER_DIRS})

if (PTX_BUILD_MCU)
	target_link_libraries(RUL PRIVATE RA4M2_NFC_FORUM_BASIC_DISCOVERY)
endif()

set(TARGET_EXE_NAME "NFC_FORUM_BASIC_DISCOVERY")
add_executable(${TARGET_EXE_NAME})
target_sources(${TARGET_EXE_NAME} PRIVATE
	../../../SRC/EXAMPLES/ptx_Example_NFCForumBasicDiscovery/ptx_ExampleApplication.c
	../../../SRC/EXAMPLES/ptx_Example_NFCForumBasicDiscovery/ptx_ExampleNdefRecords.c
	../../../SRC/EXAMPLES/ptx_Example_NFCForumBasicDiscovery/HCE_T4T/ptx_HCET4T.c
)

# link GTK
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE PkgConfig::GTK4)

if (PTX_BUILD_MCU)
	target_link_libraries(${TARGET_EXE_NAME} PRIVATE RUL RA4M2_NFC_FORUM_BASIC_DISCOVERY)
else()
	target_link_libraries(${TARGET_EXE_NAME} PRIVATE RUL)
endif()

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

target_include_directories(${TARGET_EXE_NAME} PUBLIC 
	../../../SRC/EXAMPLES
	../../../SRC/EXAMPLES/ptx_Example_NFCForumBasicDiscovery
	../../../SRC/EXAMPLES/ptx_Example_NFCForumBasicDiscovery/HCE_T4T

)

if (PTX_BUILD_MCU)
	set_target_properties("${TARGET_EXE_NAME}"
		PROPERTIES
			OUTPUT_NAME "${TARGET_EXE_NAME}"
			SUFFIX ".elf"
	)
	
	add_custom_command(TARGET ${TARGET_EXE_NAME}
		POST_BUILD
		COMMAND ${CMAKE_SIZE} --format=berkeley ${TARGET_EXE_NAME}.elf
		COMMENT "Size Summary"
	)

	add_custom_command(TARGET ${TARGET_EXE_NAME}
		POST_BUILD
		COMMAND ${CMAKE_OBJCOPY} -O ihex ${TARGET_EXE_NAME}.elf ${TARGET_EXE_NAME}.hex
		COMMAND ${CMAKE_OBJCOPY} -O srec ${TARGET_EXE_NAME}.elf ${TARGET_EXE_NAME}.srec
		COMMAND ${CMAKE_OBJCOPY} -O binary ${TARGET_EXE_NAME}.elf ${TARGET_EXE_NAME}.bin
		COMMENT "Create object derivates hex, srec, and bin"
	)
else()
	set_target_properties("${TARGET_EXE_NAME}"
		PROPERTIES
			OUTPUT_NAME "${TARGET_EXE_NAME}"
	)

endif()

