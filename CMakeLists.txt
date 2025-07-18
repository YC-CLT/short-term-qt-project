cmake_minimum_required(VERSION 3.16)

project(short-term-qt-project VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ZLIB_INCLUDE_DIR "D:/code/msys/ucrt64/include/")#你应该改成你的zlib目录，如D:/code/Qt/Tools/mingw1310_64/x86_64-w64-mingw32/include/
set(ZLIB_LIBRARY "D:/code/msys/ucrt64/lib/libz.a")#你应该改成你的zlib目录，如D:/code/Qt/Tools/mingw1310_64/x86_64-w64-mingw32/lib/libz.a

find_package(ZLIB REQUIRED)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Network Sql Svg Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Network Sql Svg Charts)

set(SOURCES_TO_TRANSLATE
        main.cpp
        src/mainwindow.cpp
        include/mainwindow.h
        include/mainwindow.ui  
        ${TS_FILES}
)
set(TS_FILES short-term-qt-project_zh_CN.ts)

qt6_create_translation(QM_FILES
    ${SOURCES_TO_TRANSLATE}
    ${TS_FILES}
)

set(PROJECT_SOURCES
        main.cpp
        src/mainwindow.cpp
        include/mainwindow.h
        include/mainwindow.ui
        ${TS_FILES}
        src/memmod.cpp
        include/memmod.h
        src/daymod.cpp
        include/daymod.h
        src/weathermod.cpp
        include/weathermod.h
        src/settingmod.cpp
        include/settingmod.h
        resources.qrc
        appicon.rc
        src/temperaturechart.cpp
        include/temperaturechart.h
        src/saying.cpp
        include/saying.h
        src/uisetting.cpp
        include/uisetting.h
        src/aiagent.cpp
        include/aiagent.h
)

# 添加根目录和生成目录到包含路径
# 确保在创建可执行目标之后设置包含目录（移动到qt_add_executable之后）

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
    find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
    qt_add_executable(short-term-qt-project
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET short-term-qt-project APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    target_include_directories(short-term-qt-project PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        include
    )

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(short-term-qt-project SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(short-term-qt-project
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(short-term-qt-project PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Sql ZLIB::ZLIB Qt${QT_VERSION_MAJOR}::Svg Qt${QT_VERSION_MAJOR}::Charts)
target_link_libraries(short-term-qt-project PRIVATE Qt6::Widgets)
target_link_libraries(short-term-qt-project PRIVATE Qt6::Core Qt6::Widgets)
target_link_libraries(short-term-qt-project PRIVATE Qt6::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.short-term-qt-project)
endif()
set_target_properties(short-term-qt-project PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS short-term-qt-project
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(short-term-qt-project)
endif()
