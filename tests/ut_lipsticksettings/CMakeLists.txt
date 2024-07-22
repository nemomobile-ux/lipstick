set(TESTNAME lipsticksettings)
set(SRC
    ut_${TESTNAME}.cpp
    ut_${TESTNAME}.h
    ../stubs/stubbase.cpp
    ${CMAKE_SOURCE_DIR}/src/lipsticksettings.cpp)

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
    -llipstick-qt6)

add_test(ut_${TESTNAME} ut_${TESTNAME})
