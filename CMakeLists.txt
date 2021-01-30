cmake_minimum_required(VERSION 3.5)

project(pHashChecker LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package( ...) calls below.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools REQUIRED)

set(TS_FILES src/lang/zh_CN.ts src/lang/ja_JP.ts)

aux_source_directory(src PROJECT_SOURCES)

set(PROJECT_SOURCES ${PROJECT_SOURCES} ${TS_FILES} ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc)

configure_file(translations.qrc ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES})
    qt_add_executable(pHashChecker
        ${PROJECT_SOURCES}
        ${QM_FILES}
    )
else()
    qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES})
    if(ANDROID)
        add_library(pHashChecker SHARED
            ${PROJECT_SOURCES}
            ${QM_FILES}
        )
    else()
        add_executable(pHashChecker
            ${PROJECT_SOURCES}
            ${QM_FILES}
        )
    endif()
endif()

target_link_libraries(pHashChecker PRIVATE Qt${QT_VERSION_MAJOR}::Widgets pHash)
