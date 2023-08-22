cmake_minimum_required(VERSION 3.6.0)

project(lipstick
    VERSION 1.1
    DESCRIPTION "QML toolkit for homescreen creation")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(FindPkgConfig)
include(FeatureSummary)
include(GNUInstallDirs)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(QT_MIN_VERSION "6.2.0")
set(CMAKE_INSTALL_PREFIX /usr)

### Sets QT_INSTALL_QML to the directory where QML Plugins should be installed
function(FindQtInstallQml)
    find_program(QMAKE NAMES qmake6 qmake)
    if(NOT QMAKE)
        message(FATAL_ERROR "qmake not found")
    endif()
    execute_process(
        COMMAND ${QMAKE} -query QT_INSTALL_QML
        OUTPUT_VARIABLE PROC_RESULT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(QT_INSTALL_QML ${PROC_RESULT} PARENT_SCOPE)
endfunction()

FindQtInstallQml()

find_package(PkgConfig REQUIRED)
find_package(Qt6 ${QT_MIN_VERSION} REQUIRED
    Core
    DBus
    Gui
    Qml
    Quick
    Sql
    Sensors
    WaylandCompositor
    LinguistTools)
find_package(KF6BluezQt REQUIRED)

pkg_check_modules(DBUS dbus-1 REQUIRED IMPORTED_TARGET)
pkg_check_modules(DSME_DBUS dsme_dbus_if REQUIRED IMPORTED_TARGET)
pkg_check_modules(GLIB2 glib-2.0 REQUIRED IMPORTED_TARGET)
pkg_check_modules(GIO_UNIX gio-unix-2.0 REQUIRED IMPORTED_TARGET)
pkg_check_modules(KEEPALIVE keepalive REQUIRED IMPORTED_TARGET)
pkg_check_modules(LIBRESOURCE libresourceqt6 REQUIRED IMPORTED_TARGET)
pkg_check_modules(LIBSYSTEMD libsystemd REQUIRED IMPORTED_TARGET)
pkg_check_modules(MLITE mlite6 REQUIRED IMPORTED_TARGET)
pkg_check_modules(MCE mce REQUIRED IMPORTED_TARGET)
pkg_check_modules(MCE_QT mce-qt6 REQUIRED IMPORTED_TARGET)
pkg_check_modules(NEMODEVICELOCK nemodevicelock REQUIRED IMPORTED_TARGET)
pkg_check_modules(NGF_QT6 ngf-qt6 REQUIRED IMPORTED_TARGET)
pkg_check_modules(THERMALMANAGER_DBUS thermalmanager_dbus_if REQUIRED IMPORTED_TARGET)
pkg_check_modules(SYSTEMSETTINGS systemsettings-qt6 REQUIRED IMPORTED_TARGET)
pkg_check_modules(TIMED_QT timed-qt6 REQUIRED IMPORTED_TARGET)
pkg_check_modules(WAYLAND_SERVER wayland-server REQUIRED IMPORTED_TARGET)
pkg_check_modules(USB_MODED_QT usb-moded-qt6 REQUIRED IMPORTED_TARGET)

add_subdirectory(src)
add_subdirectory(plugin)

add_dependencies(lipstickplugin lipstick-qt6)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
