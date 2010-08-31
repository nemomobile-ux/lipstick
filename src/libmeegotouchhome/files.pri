# Input
HEADERS += homeapplication.h \
    windowinfo.h \
    home.h \
    desktop.h \
    desktopmodel.h \
    desktopview.h \
    desktopstyle.h \
    extendedbuttonview.h \
    extendedbuttonstyle.h \
    mainwindow.h \
    launcher.h \
    launcherview.h \
    launchermodel.h \
    launcherstyle.h \
    launcheraction.h \
    launcherbutton.h \
    launcherbuttonview.h \
    launcherbuttonmodel.h \
    launcherbuttonstyle.h \
    pagedpanning.h \
    pagedviewport.h \
    pagedviewportstyle.h \
    pagedviewportview.h \
    quicklaunchbar.h \
    quicklaunchbarmodel.h \
    quicklaunchbarview.h \
    quicklaunchbarstyle.h \
    switcher.h \
    switchermodel.h \
    switcherview.h \
    switcherstyle.h \
    switcherbutton.h \
    switcherbuttonmodel.h \
    switcherbuttonview.h \
    switcherbuttonstyle.h \
    switcherbuttonwithtitlebarview.h \
    switcherbuttonwithtitlebarstyle.h \
    transformlayoutanimation.h \
    transformlayoutanimationstyle.h \
    x11wrapper.h \
    launcherpage.h \
    launcherpageview.h \
    launcherpagestyle.h \
    launcherpagemodel.h \
    launcherdatastore.h \
    pagepositionindicatorview.h \
    pagepositionindicatorstyle.h \
    pagepositionindicatormodel.h \
    pagepositionindicator.h \
    homescreenservice.h \
    homescreenadaptor.h \
    homewindowmonitor.h \
    windowmonitor.h \
    applicationpackagemonitor.h \
    applicationpackagemonitorlistener.h \
    xeventlistener.h \
    launcherbuttonprogressindicatorview.h \
    launcherbuttonprogressindicatorstyle.h \
    launcherbuttonprogressindicator.h \
    launcherbuttonprogressindicatormodel.h
SOURCES += homeapplication.cpp \
    windowinfo.cpp \
    home.cpp \
    desktop.cpp \
    desktopview.cpp \
    extendedbuttonview.cpp \
    mainwindow.cpp \
    launcher.cpp \
    launcherview.cpp \
    launcheraction.cpp \
    launcherbutton.cpp \
    launcherbuttonview.cpp \
    pagedpanning.cpp \
    pagedviewport.cpp \
    pagedviewportview.cpp \
    quicklaunchbar.cpp \
    quicklaunchbarview.cpp \
    switcher.cpp \
    switcherview.cpp \
    switcherbutton.cpp \
    switcherbuttonview.cpp \
    switcherbuttonwithtitlebarview.cpp \
    transformlayoutanimation.cpp \
    x11wrapper.cpp \
    launcherpage.cpp \
    launcherpageview.cpp \
    launcherdatastore.cpp \
    pagepositionindicatorview.cpp \
    pagepositionindicator.cpp \
    homescreenservice.cpp \
    homescreenadaptor.cpp \
    homewindowmonitor.cpp \
    applicationpackagemonitor.cpp \
    applicationpackagemonitorlistener.cpp \
    xeventlistener.cpp \
    launcherbuttonprogressindicatorview.cpp \
    launcherbuttonprogressindicator.cpp
MODEL_HEADERS += desktopmodel.h \
    launcherbuttonmodel.h \
    launchermodel.h \
    quicklaunchbarmodel.h \
    switcherbuttonmodel.h \
    switchermodel.h \
    launcherpagemodel.h \
    pagepositionindicatormodel.h \
    launcherbuttonprogressindicatormodel.h
STYLE_HEADERS += desktopstyle.h \
    extendedbuttonstyle.h \
    launcherstyle.h \
    launcherbuttonstyle.h \
    launcherpagestyle.h \
    quicklaunchbarstyle.h \
    switcherbuttonstyle.h \
    switcherbuttonwithtitlebarstyle.h \
    switcherstyle.h \
    pagepositionindicatorstyle.h \
    pagedviewportstyle.h \
    transformlayoutanimationstyle.h \
    launcherbuttonprogressindicatorstyle.h
INSTALL_HEADERS += $$HEADERS \
    mdesktopbackgroundextensioninterface.h
