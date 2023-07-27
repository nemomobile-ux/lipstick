system($$[QT_INSTALL_BINS]/qdbuscpp2xml compositor.xml -a lipstickcompositoradaptor -c LipstickCompositorAdaptor -l LipstickCompositor -i lipstickcompositor.h)
system($$[QT_INSTALL_BINS]/qdbuscpp2xml fileservice.xml -a fileserviceadaptor -c FileServiceAdaptor)

INCLUDEPATH += $$PWD

PUBLICHEADERS += \
    $$PWD/lipstickcompositor.h \
    $$PWD/lipstickcompositorwindow.h \
    $$PWD/lipstickcompositorprocwindow.h \
    $$PWD/lipstickcompositoradaptor.h \
    $$PWD/fileserviceadaptor.h \
    $$PWD/windowmodel.h \

HEADERS += \
    $$PWD/windowpixmapitem.h

SOURCES += \
    $$PWD/lipstickcompositor.cpp \
    $$PWD/lipstickcompositorwindow.cpp \
    $$PWD/lipstickcompositorprocwindow.cpp \
    $$PWD/lipstickcompositoradaptor.cpp \
    $$PWD/fileserviceadaptor.cpp \
    $$PWD/windowmodel.cpp \
    $$PWD/windowpixmapitem.cpp

DEFINES += QT_COMPOSITOR_QUICK

QT += waylandcompositor

# needed for hardware compositor
QT += quick-private gui-private core-private qml-private

OTHER_FILES += $$PWD/compositor.xml $$PWD/fileservice.xml
