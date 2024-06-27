# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2022 apocelipes

cmake_minimum_required(VERSION 3.20)

project(pHashChecker LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" OR ${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    add_compile_options(-Wall -Wextra -fno-rtti -O2)
endif()

include_directories(third-party/CImg
    third-party/pHash/src
    third-party/unordered_dense/include
    third-party/cpp-sort/include
    src/utils
    src
)

add_definitions(-Dcimg_use_heif)

find_package(Qt6 REQUIRED COMPONENTS Core)
qt_standard_project_setup()
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

set(TS_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lang/zh_CN.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lang/ja_JP.ts)

aux_source_directory(src PROJECT_SOURCES)

set(PROJECT_SOURCES ${PROJECT_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc)

add_custom_target(update_all_ts_files ALL)
add_custom_target(create_all_qm_files ALL)
find_file(LUPDATE_PATH lupdate)
find_file(LRELEASE_PATH lrelease)
foreach(TS_FILE ${TS_FILES})
    get_filename_component(I18N_NAME ${TS_FILE} NAME_WE)
    set(TS_TARGET_NAME "update_ts_file_${I18N_NAME}")
    add_custom_target(${TS_TARGET_NAME}
            COMMAND ${LUPDATE_PATH} ${CMAKE_CURRENT_SOURCE_DIR} -ts ${TS_FILE}
            VERBATIM)
    add_dependencies(update_all_ts_files ${TS_TARGET_NAME})
    set(QM_TARGET_NAME "create_qm_file_${I18N_NAME}")
    set(QM_FILE "${CMAKE_CURRENT_BINARY_DIR}/${I18N_NAME}.qm")
    add_custom_target(${QM_TARGET_NAME}
            COMMAND ${LRELEASE_PATH} ${TS_FILE} -qm ${QM_FILE}
            VERBATIM)
    add_dependencies(${QM_TARGET_NAME} ${TS_TARGET_NAME})
    add_dependencies(create_all_qm_files ${QM_TARGET_NAME})
endforeach()

configure_file(translations.qrc ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

set(PHASH_DYNAMIC OFF CACHE BOOL "Build pHash dynamic library")
set(PHASH_STATIC ON CACHE BOOL "Build pHash static library")
add_subdirectory(third-party/pHash)

add_executable(pHashChecker
    ${PROJECT_SOURCES}
)

add_dependencies(${PROJECT_NAME} create_all_qm_files)

target_link_libraries(pHashChecker PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets pHash)

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT output LANGUAGES CXX)
if(ipo_supported)
    set_property(TARGET pHashChecker PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(WARNING "IPO is not supported: ${output}")
endif()
