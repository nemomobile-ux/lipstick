set(TESTNAME volumecontrol)
set(SRC
    ut_${TESTNAME}.cpp
    ut_${TESTNAME}.h
    ../stubs/stubbase.cpp
    ../stubs/homewindow.cpp)

add_executable(ut_${TESTNAME} ${SRC})

target_link_libraries(ut_${TESTNAME}
    Qt6::Core
    Qt6::Quick
    Qt6::DBus
    Qt6::Gui
    Qt6::WaylandCompositor
    Qt6::Test
    PkgConfig::DBUS
    PkgConfig::MLITE
    PkgConfig::GLIB2
    PkgConfig::LIBRESOURCE
    PkgConfig::TIMED_QT
    -llipstick-qt6)

add_test(ut_${TESTNAME} ut_${TESTNAME})
