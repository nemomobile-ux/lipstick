set(TESTNAME launchermodel)
set(SRC
    ut_${TESTNAME}.cpp
    ut_${TESTNAME}.h
    ../stubs/stubbase.cpp
    ${CMAKE_SOURCE_DIR}/src/components/launchermodel.cpp
    ${CMAKE_SOURCE_DIR}/src/components/launchermonitor.cpp
    ${CMAKE_SOURCE_DIR}/src/components/launcheritem.cpp
    ${CMAKE_SOURCE_DIR}/src/components/launcherdbus.cpp
    ${CMAKE_SOURCE_DIR}/src/utilities/qobjectlistmodel.cpp
    ${CMAKE_SOURCE_DIR}/src/logging.cpp)

add_executable(ut_${TESTNAME} ${SRC})
add_dependencies(ut_${TESTNAME} lipstick-qt6)
target_link_libraries(ut_${TESTNAME}
    Qt6::Core
    Qt6::Quick
    Qt6::DBus
    Qt6::Sql
    Qt6::WaylandCompositor
    Qt6::Test
    PkgConfig::DBUS
    PkgConfig::GIO_UNIX
    PkgConfig::MLITE
    PkgConfig::SYSTEMSETTINGS
    -llipstick-qt6 -L../../src)

add_test(ut_${TESTNAME} ut_${TESTNAME})
