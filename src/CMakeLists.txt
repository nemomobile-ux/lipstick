set(PACKAGE lipstick-qt6)

set(PUBLICHEADERS
    utilities/qobjectlistmodel.h
    utilities/closeeventeater.h
    touchscreen/touchscreen.h
    homeapplication.h
    homewindow.h
    lipstickglobal.h
    lipsticksettings.h
    lipstickdbus.h
    lipstickqmlpath.h
    components/launcheritem.h
    components/launchermodel.h
    components/launcherwatchermodel.h
    components/launchermonitor.h
    components/launcherdbus.h
    components/launcherfoldermodel.h
    compositor/lipstickcompositor.h
    compositor/lipstickcompositorwindow.h
    compositor/lipstickcompositorprocwindow.h
    compositor/windowmodel.h
    notifications/notificationmanager.h
    notifications/lipsticknotification.h
    notifications/notificationlistmodel.h
    notifications/notificationpreviewpresenter.h
    usbmodeselector.h
    localemanager.h
    shutdownscreen.h
    devicestate/displaystate.h
    devicestate/devicestate.h
    vpnagent.h)

set(HEADERS
    ${PUBLICHEADERS}
    3rdparty/synchronizelists.h
    3rdparty/dbus-gmain/dbus-gmain.h
    bluetooth/bluetoothagent.h
    bluetooth/bluetoothobexagent.h
    compositor/windowpixmapitem.h
    notifications/categorydefinitionstore.h
    notifications/batterynotifier.h
    notifications/notificationfeedbackplayer.h
    notifications/androidprioritystore.h
    screenlock/screenlock.h
    touchscreen/touchscreen_p.h
    volume/volumecontrol.h
    volume/pulseaudiocontrol.h
    lipstickapi.h
    lipstickqmlpath.h
    screenshotservice.h
    qdbusxml2cpp_dbus_types.h
    notifications/thermalnotifier.h
    devicestate/devicestate_p.h
    devicestate/displaystate_p.h
    logging.h)

set(SOURCES
    3rdparty/dbus-gmain/dbus-gmain.c
    bluetooth/bluetoothagent.cpp
    bluetooth/bluetoothobexagent.cpp
    homeapplication.cpp
    homewindow.cpp
    lipsticksettings.cpp
    lipstickqmlpath.cpp
    utilities/qobjectlistmodel.cpp
    utilities/closeeventeater.cpp
    components/launcheritem.cpp
    components/launchermodel.cpp
    components/launcherwatchermodel.cpp
    components/launchermonitor.cpp
    components/launcherdbus.cpp
    components/launcherfoldermodel.cpp
    compositor/lipstickcompositor.cpp
    compositor/lipstickcompositorwindow.cpp
    compositor/lipstickcompositorprocwindow.cpp
    compositor/windowmodel.cpp
    compositor/windowpixmapitem.cpp
    notifications/notificationmanager.cpp
    notifications/lipsticknotification.cpp
    notifications/categorydefinitionstore.cpp
    notifications/notificationlistmodel.cpp
    notifications/notificationpreviewpresenter.cpp
    notifications/batterynotifier.cpp
    notifications/androidprioritystore.cpp
    screenlock/screenlock.cpp
    touchscreen/touchscreen.cpp
    volume/volumecontrol.cpp
    volume/pulseaudiocontrol.cpp
    notifications/notificationfeedbackplayer.cpp
    usbmodeselector.cpp
    localemanager.cpp
    shutdownscreen.cpp
    vpnagent.cpp
    lipstickapi.cpp
    screenshotservice.cpp
    notifications/thermalnotifier.cpp
    devicestate/displaystate.cpp
    devicestate/devicestate.cpp
    logging.cpp
    connmanvpnagent.cpp)

add_definitions(-DLIPSTICK_BUILD_LIBRARY)
add_definitions(-DMESA_EGL_NO_X11_HEADERS)
add_definitions(-DQT_COMPOSITOR_QUICK)
add_definitions(-DEGL_NO_X11)
add_definitions(-Wfatal-errors)
add_definitions(-DVERSION="${CMAKE_PROJECT_VERSION}")

set_source_files_properties("${CMAKE_SOURCE_DIR}/src/net.connman.vpn.xml"
    PROPERTIES
    INCLUDE "qdbusxml2cpp_dbus_types.h"
    CLASSNAME "ConnmanVpnProxy")

qt_add_dbus_interface(SOURCES "${CMAKE_SOURCE_DIR}/src/net.connman.vpn.xml" "connmanvpnproxy")

qt_add_dbus_adaptor(SOURCES "${CMAKE_SOURCE_DIR}/src/notifications/notificationmanager.xml"
    "notifications/notificationmanager.h"
    "NotificationManager"
    "notifications/notificationmanageradaptor"
    "NotificationManagerAdaptor")
qt_add_dbus_adaptor(SOURCES "${CMAKE_SOURCE_DIR}/src/screenlock/screenlock.xml"
    "screenlock/screenlock.h"
    "ScreenLock"
    "screenlock/screenlockadaptor"
    "ScreenLockAdaptor")
qt_add_dbus_adaptor(SOURCES "${CMAKE_SOURCE_DIR}/src/screenshotservice.xml"
    "screenshotservice.h"
    "ScreenshotService"
    "screenshotserviceadaptor"
    "ScreenshotServiceAdaptor")
qt_add_dbus_adaptor(SOURCES "${CMAKE_SOURCE_DIR}/src/shutdownscreen.xml"
    "shutdownscreen.h"
    "ShutdownScreen"
    "shutdownscreenadaptor"
    "ShutdownScreenAdaptor")
qt_add_dbus_adaptor(SOURCES "${CMAKE_SOURCE_DIR}/src/net.connman.vpn.Agent.xml"
    "vpnagent.h"
    "VpnAgent"
    "connmanvpnagent"
    "ConnmanVpnAgentAdaptor")

qt_add_dbus_adaptor(SOURCES "${CMAKE_SOURCE_DIR}/src/compositor/compositor.xml"
    "lipstickcompositor.h"
    "LipstickCompositor"
    "lipstickcompositoradaptor"
    "LipstickCompositorAdaptor")
qt_add_dbus_adaptor(SOURCES "${CMAKE_SOURCE_DIR}/src/compositor/fileservice.xml" "fileserviceadaptor.h" "" "" "FileServiceAdaptor")

add_library(${PACKAGE} SHARED ${SOURCES} ${HEADERS})
set_target_properties(${PACKAGE}
    PROPERTIES VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION})

include_directories(utilities
    touchscreen
    components
    compositor
    3rdparty
    devicestate
    ${CMAKE_CURRENT_BINARY_DIR}/notifications
    ${CMAKE_CURRENT_BINARY_DIR}/screenlock
    ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(${PACKAGE} PUBLIC
    Qt6::DBus
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickPrivate
    Qt6::Sql
    Qt6::Gui
    Qt6::GuiPrivate
    Qt6::Sensors
    Qt6::WaylandCompositor
    PkgConfig::DBUS
    PkgConfig::DSME_DBUS
    PkgConfig::GLIB2
    PkgConfig::GIO_UNIX
    PkgConfig::KEEPALIVE
    PkgConfig::LIBRESOURCE
    PkgConfig::LIBSYSTEMD
    PkgConfig::MLITE
    PkgConfig::MCE
    PkgConfig::MCE_QT
    PkgConfig::NEMODEVICELOCK
    PkgConfig::NEMOCONNECTIVITY
    PkgConfig::NGF_QT6
    PkgConfig::THERMALMANAGER_DBUS
    PkgConfig::SYSTEMSETTINGS
    PkgConfig::TIMED_QT
    PkgConfig::WAYLAND_SERVER
    PkgConfig::USB_MODED_QT
    KF6::BluezQt
    -lrt -lEGL)

configure_file(${PACKAGE}.pc.in ${PACKAGE}.pc @ONLY)

# Translations
file(GLOB TS_FILES ../i18n/*.ts)
qt_add_lupdate(${PACKAGE} ${TS_FILES} no_check_exist)
qt6_add_translation(QM_FILES ${TS_FILES})
add_custom_target(translations DEPENDS ${QM_FILES})
add_dependencies(lipstick-qt6 translations)

install(TARGETS ${PACKAGE} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${PUBLICHEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lipstick-qt6)
install(FILES lipstick.conf DESTINATION /etc/dbus-1/system.d)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
install(DIRECTORY notificationcategories
    DESTINATION ${CMAKE_INSTALL_DATADIR}/lipstick/notificationcategories
    FILES_MATCHING PATTERN "*.conf")
install(FILES androidnotificationpriorities DESTINATION ${CMAKE_INSTALL_DATADIR}/lipstick)
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/translations)
