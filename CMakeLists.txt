cmake_minimum_required(VERSION 3.16)
project(MCModloaderInstaller VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Concurrent)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/SidebarWidget.cpp
    src/InstallerPage.cpp
    src/VersionFetcher.cpp
    src/VersionParser.cpp
    src/InstallerWorker.cpp
    src/ProgressDialog.cpp
    src/NoguiDialog.cpp
    resources/resources.qrc
)

set(HEADERS
    src/MainWindow.h
    src/SidebarWidget.h
    src/InstallerPage.h
    src/VersionFetcher.h
    src/VersionParser.h
    src/InstallerWorker.h
    src/ProgressDialog.h
    src/NoguiDialog.h
    src/ModloaderInfo.h
)

if(WIN32)
    list(APPEND SOURCES resources/app.rc)
endif()

add_executable(MCModloaderInstaller WIN32
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(MCModloaderInstaller PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Concurrent
)

set_target_properties(MCModloaderInstaller PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# Copy Python scripts to build output directory
configure_file(${CMAKE_SOURCE_DIR}/python/requestVer.py ${CMAKE_BINARY_DIR}/requestVer.py COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/python/OptiToken.py  ${CMAKE_BINARY_DIR}/OptiToken.py  COPYONLY)

# Install rules
install(TARGETS MCModloaderInstaller RUNTIME DESTINATION bin)
install(FILES
    ${CMAKE_SOURCE_DIR}/python/requestVer.py
    ${CMAKE_SOURCE_DIR}/python/OptiToken.py
    DESTINATION bin
)

message(STATUS "==============================================")
message(STATUS "  MCModloaderInstaller ${PROJECT_VERSION}")
message(STATUS "  Qt: ${Qt6_VERSION}")
message(STATUS "  Build: ${CMAKE_BUILD_TYPE}")
message(STATUS "==============================================")
