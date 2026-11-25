set(PACKAGE notificationtool)

set_source_files_properties(${CMAKE_SOURCE_DIR}/src/notifications/notificationmanager.xml
    PROPERTIES
    INCLUDE "lipsticknotification.h"
    CLASSNAME "NotificationManagerProxy")

set(SOURCES notificationtool.cpp)
qt_add_dbus_interface(SOURCES ${CMAKE_SOURCE_DIR}/src/notifications/notificationmanager.xml notificationmanagerproxy)

add_executable(${PACKAGE} ${SOURCES})

target_link_libraries(${PACKAGE} PUBLIC
    Qt6::Core
    Qt6::DBus
    lipstick-qt6)

target_include_directories(${PACKAGE} PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ../../src
    ../../src/notifications)
