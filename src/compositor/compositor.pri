system(qdbusxml2cpp compositor.xml -a lipstickcompositoradaptor -c LipstickCompositorAdaptor -l LipstickCompositor -i lipstickcompositor.h)
system(qdbusxml2cpp fileservice.xml -a fileserviceadaptor -c FileServiceAdaptor)

INCLUDEPATH += $$PWD

PUBLICHEADERS += \
    $$PWD/lipstickcompositor.h \
    $$PWD/lipstickcompositorwindow.h \
    $$PWD/lipstickcompositorprocwindow.h \
    $$PWD/lipstickcompositoradaptor.h \
    $$PWD/fileserviceadaptor.h \
    $$PWD/windowmodel.h \

HEADERS += \
    $$PWD/windowpixmapitem.h \
    $$PWD/lipstickrecorder.h \
    $$PWD/hwcrenderstage.h \
    $$PWD/hwcimage.h \
    $$PWD/eglhybrisbuffer.h \
    $$PWD/eglhybrisfunctions.h

SOURCES += \
    $$PWD/lipstickcompositor.cpp \
    $$PWD/lipstickcompositorwindow.cpp \
    $$PWD/lipstickcompositorprocwindow.cpp \
    $$PWD/lipstickcompositoradaptor.cpp \
    $$PWD/fileserviceadaptor.cpp \
    $$PWD/windowmodel.cpp \
    $$PWD/windowpixmapitem.cpp \
    $$PWD/hwcrenderstage.cpp \
    $$PWD/hwcimage.cpp \
    $$PWD/eglhybrisbuffer.cpp \
    $$PWD/eglhybrisfunctions.cpp

DEFINES += QT_COMPOSITOR_QUICK

QT += waylandcompositor

# needed for hardware compositor
QT += quick-private gui-private core-private waylandcompositor-private qml-private

WAYLANDSERVERSOURCES += ../protocol/lipstick-recorder.xml \

OTHER_FILES += $$PWD/compositor.xml $$PWD/fileservice.xml
