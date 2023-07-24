system(qdbusxml2cpp ../../src/notifications/notificationmanager.xml -p notificationmanagerproxy -c NotificationManagerProxy -i lipsticknotification.h)

TEMPLATE = app
TARGET = notificationtool

QT += core dbus
CONFIG += link_pkgconfig c++17
PKGCONFIG += mlite$${QT_MAJOR_VERSION} dsme_dbus_if thermalmanager_dbus_if usb_moded

INSTALLS = target
target.path = /usr/bin

DEPENDPATH += "../../src"
INCLUDEPATH += "../../src" "../../src/notifications" "../../src/devicestate"
QMAKE_LIBDIR = ../../src
LIBS = -llipstick-qt$${QT_MAJOR_VERSION}

HEADERS += \
     notificationmanagerproxy.h
SOURCES += \
     notificationtool.cpp \
     notificationmanagerproxy.cpp

QMAKE_CXXFLAGS += \
    -Werror \
    -g \
    -fvisibility=hidden \
    -fvisibility-inlines-hidden
