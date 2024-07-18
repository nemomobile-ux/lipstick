set(TESTNAME shutdownscreen)
set(SRC
    ut_${TESTNAME}.cpp
    ut_${TESTNAME}.h
    ../stubs/stubbase.cpp
    ../stubs/homewindow.cpp
    ../stubs/homeapplication.cpp
    ${CMAKE_SOURCE_DIR}/src/shutdownscreen.cpp
    ${CMAKE_SOURCE_DIR}/src/devicestate/devicestate.cpp
    ${CMAKE_SOURCE_DIR}/src/devicestate/displaystate.cpp
    ${CMAKE_SOURCE_DIR}/src/notifications/lipsticknotification.cpp
    ${CMAKE_SOURCE_DIR}/src/notifications/thermalnotifier.cpp)

add_executable(ut_${TESTNAME} ${SRC})

target_link_libraries(ut_${TESTNAME}
    Qt6::Core
    Qt6::Quick
    Qt6::DBus
    Qt6::Gui
    Qt6::WaylandCompositor
    Qt6::Test
    PkgConfig::DSME_DBUS
    PkgConfig::MLITE
    PkgConfig::TIMED_QT
    PkgConfig::THERMALMANAGER_DBUS
    -llipstick-qt6)

add_test(ut_${TESTNAME} ut_${TESTNAME})
