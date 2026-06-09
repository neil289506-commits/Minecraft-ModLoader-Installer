cmake_minimum_required(VERSION 3.16)

project(MCModloaderInstaller VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    Concurrent
)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/VersionFetcher.cpp
    src/InstallerWorker.cpp
    src/VersionParser.cpp
    src/SidebarWidget.cpp
    src/InstallerPage.cpp
    src/ProgressDialog.cpp
)

set(HEADERS
    src/MainWindow.h
    src/VersionFetcher.h
    src/InstallerWorker.h
    src/VersionParser.h
    src/SidebarWidget.h
    src/InstallerPage.h
    src/ProgressDialog.h
    src/ModloaderInfo.h
)

# WIN32 = Windows GUI Application (不顯示終端機)
add_executable(MCModloaderInstaller WIN32
    ${SOURCES}
    ${HEADERS}
    resources/resources.qrc
    appicon.rc
)

target_link_libraries(MCModloaderInstaller PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
)

# 保險設定
set_target_properties(MCModloaderInstaller PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# Copy python script next to executable
configure_file(
    ${CMAKE_SOURCE_DIR}/python/requestVer.py
    ${CMAKE_BINARY_DIR}/requestVer.py
    COPYONLY
)

install(TARGETS MCModloaderInstaller
    RUNTIME DESTINATION bin
)
