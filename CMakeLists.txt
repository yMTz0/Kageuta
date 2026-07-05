cmake_minimum_required(VERSION 3.20)
project(Kageuta VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Multimedia MultimediaWidgets)
find_package(CURL REQUIRED)

set(WEBVIEW2_DIR ${CMAKE_SOURCE_DIR}/webview2/sdk/build/native)
set(WEBVIEW2_INCLUDE ${WEBVIEW2_DIR}/include)
set(WEBVIEW2_LIB ${WEBVIEW2_DIR}/x64/WebView2Loader.dll.lib)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/scraper.cpp
    src/player.cpp
    src/webview2widget.cpp
)

set(HEADERS
    src/mainwindow.h
    src/scraper.h
    src/player.h
    src/styles.h
    src/clickableframe.h
    src/webview2widget.h
)

set(RESOURCES
    assets/resources.qrc
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS} ${RESOURCES} assets/app.rc)

target_include_directories(${PROJECT_NAME} PRIVATE src ${CURL_INCLUDE_DIRS} ${WEBVIEW2_INCLUDE})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    ${CURL_LIBRARIES}
    ${WEBVIEW2_LIB}
    ole32
    oleaut32
)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
        LINK_FLAGS "-mwindows"
    )
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
