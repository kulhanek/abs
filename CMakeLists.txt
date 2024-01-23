# ==============================================================================
# ABS CMake File
# ==============================================================================

CMAKE_MINIMUM_REQUIRED(VERSION 3.10 FATAL_ERROR)
#SET(CMAKE_VERBOSE_MAKEFILE ON)
PROJECT(ABS)

# ==============================================================================
# conditional setup ------------------------------------------------------------
# ==============================================================================

SET(ABS_CORE_ONLY   OFF     CACHE BOOL "Only ABS core library?")

# ==============================================================================
# project setup ----------------------------------------------------------------
# ==============================================================================

# define project name ----------------------------
SET(ABS_VERSION 4.x)

# what languages we use --------------------------
ENABLE_LANGUAGE(CXX)

# ==============================================================================
# external dependencies --------------------------------------------------------
# ==============================================================================

IF(NOT DEFINED COMPOSITE_PROJECT)
    SET(DEVELOPMENT_ROOT "$ENV{DEVELOPMENT_ROOT}")
ENDIF(NOT DEFINED COMPOSITE_PROJECT)

IF(NOT DEVELOPMENT_ROOT)
    MESSAGE(FATAL_ERROR "DEVELOPMENT_ROOT variable is not set!")
ENDIF(NOT DEVELOPMENT_ROOT)

# ==============================================================================
# external libraries -----------------------------------------------------------
# ==============================================================================

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${DEVELOPMENT_ROOT}/projects/cmake-modules/1.0/share/modules")

# boost =======================
FIND_PACKAGE(MyBoost REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS} SYSTEM)

# HIPOLY -----------------------------------------
SET(HIPOLY_ROOT ${DEVELOPMENT_ROOT}/projects/hipoly/1.0)
INCLUDE_DIRECTORIES(${HIPOLY_ROOT}/src/lib/hipoly SYSTEM)
INCLUDE_DIRECTORIES(${HIPOLY_ROOT}/src/lib/hipoly/compress SYSTEM)
INCLUDE_DIRECTORIES(${HIPOLY_ROOT}/src/lib/hipoly/error SYSTEM)
INCLUDE_DIRECTORIES(${HIPOLY_ROOT}/src/lib/hipoly/event SYSTEM)
INCLUDE_DIRECTORIES(${HIPOLY_ROOT}/src/lib/hipoly/file SYSTEM)
INCLUDE_DIRECTORIES(${HIPOLY_ROOT}/src/lib/hipoly/list SYSTEM)
INCLUDE_DIRECTORIES(${HIPOLY_ROOT}/src/lib/hipoly/option SYSTEM)
INCLUDE_DIRECTORIES(${HIPOLY_ROOT}/src/lib/hipoly/string SYSTEM)
INCLUDE_DIRECTORIES(${HIPOLY_ROOT}/src/lib/hipoly/stream SYSTEM)
INCLUDE_DIRECTORIES(${HIPOLY_ROOT}/src/lib/hipoly/thread SYSTEM)
INCLUDE_DIRECTORIES(${HIPOLY_ROOT}/src/lib/hipoly/time SYSTEM)
INCLUDE_DIRECTORIES(${HIPOLY_ROOT}/src/lib/hipoly/xml SYSTEM)
LINK_DIRECTORIES(${HIPOLY_ROOT}/lib)
SET(HIPOLY_LIB_NAME hipoly)

# AMS -----------------------------------------
SET(AMS_ROOT ${DEVELOPMENT_ROOT}/projects/ams/9.0)
INCLUDE_DIRECTORIES(${AMS_ROOT}/src/lib/ams SYSTEM)
INCLUDE_DIRECTORIES(${AMS_ROOT}/src/lib/ams/base SYSTEM)
INCLUDE_DIRECTORIES(${AMS_ROOT}/src/lib/ams/host SYSTEM)
INCLUDE_DIRECTORIES(${AMS_ROOT}/src/lib/ams/host/components SYSTEM)
INCLUDE_DIRECTORIES(${AMS_ROOT}/src/lib/ams/user SYSTEM)
INCLUDE_DIRECTORIES(${AMS_ROOT}/src/lib/ams/site SYSTEM)
INCLUDE_DIRECTORIES(${AMS_ROOT}/src/lib/ams/mods SYSTEM)
LINK_DIRECTORIES(${AMS_ROOT}/lib)
SET(AMS_LIB_NAME ams)

# all libs ---------------------------------------
SET(ABS_EXT_LIBS
        plusula
        ${AMS_LIB_NAME}
        ${HIPOLY_LIB_NAME}
        ${SYSTEM_LIBS}
        )

SET(ABS_LIBS
        abs
        ${ABS_EXT_LIBS}
        )

# architecture -----------------------------------
SET(UNIX 1)
ADD_DEFINITIONS(-DUNIX)

# policies ---------------------------------------
IF(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

# ==============================================================================
# output directories -----------------------------------------------------------
# ==============================================================================

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/lib)

# ==============================================================================
# rpaths -----------------------------------------------------------------------
# ==============================================================================

IF(NOT DEFINED COMPOSITE_PROJECT)
    # use, i.e. don't skip the full RPATH for the build tree
    SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

    # when building, don't use the install RPATH already
    # (but later on when installing)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

    # the RPATH to be used when installing
    SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

    # add the automatically determined parts of the RPATH
    # which point to directories outside the build tree to the install RPATH
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
ENDIF(NOT DEFINED COMPOSITE_PROJECT)

# ==============================================================================
# compiler setups --------------------------------------------------------------
# ==============================================================================

IF(NOT DEFINED COMPOSITE_PROJECT)
    SET(CMAKE_CXX_FLAGS_DEBUG "-g -std=c++11 -Wall -Werror -pedantic-errors -Wundef -Wno-long-long")
    SET(CMAKE_CXX_FLAGS_RELEASE "-O2 -std=c++11")

    #SET(CMAKE_BUILD_TYPE Release)
    SET(CMAKE_BUILD_TYPE Debug)
ENDIF(NOT DEFINED COMPOSITE_PROJECT)

# ==============================================================================
# project subdirectories  ------------------------------------------------------
# ==============================================================================

IF(ABS_CORE_ONLY)
    ADD_SUBDIRECTORY(src)
ELSE(ABS_CORE_ONLY)
    ADD_SUBDIRECTORY(src)
    ADD_SUBDIRECTORY(etc)
    ADD_SUBDIRECTORY(share)
    ADD_SUBDIRECTORY(bin)
ENDIF(ABS_CORE_ONLY)




