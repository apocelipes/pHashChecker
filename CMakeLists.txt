cmake_minimum_required(VERSION 3.5)

project(pHashChecker LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" OR ${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    add_compile_options(-Wall -Wextra)
endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

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

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(pHashChecker
        ${PROJECT_SOURCES}
    )
else()
    add_executable(pHashChecker
        ${PROJECT_SOURCES}
    )
endif()
add_dependencies(${PROJECT_NAME} create_all_qm_files)

target_link_libraries(pHashChecker PRIVATE Qt${QT_VERSION_MAJOR}::Widgets pHash)
