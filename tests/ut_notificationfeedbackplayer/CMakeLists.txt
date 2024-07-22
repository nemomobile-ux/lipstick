set(TESTNAME notificationfeedbackplayer)
set(SRC
    ut_${TESTNAME}.cpp
    ut_${TESTNAME}.h
    ../stubs/stubbase.cpp
    ${CMAKE_SOURCE_DIR}/src/notifications/lipsticknotification.cpp
    ${CMAKE_SOURCE_DIR}/src/notifications/notificationfeedbackplayer.cpp)

add_executable(ut_${TESTNAME} ${SRC})

target_link_libraries(ut_${TESTNAME}
    Qt6::Core
    Qt6::Quick
    Qt6::DBus
    Qt6::Sql
    Qt6::WaylandCompositor
    Qt6::Test
    PkgConfig::DBUS
    PkgConfig::MLITE
    PkgConfig::SYSTEMSETTINGS
    PkgConfig::NGF_QT6
    PkgConfig::TIMED_QT
    -llipstick-qt6)

add_test(ut_${TESTNAME} ut_${TESTNAME})
