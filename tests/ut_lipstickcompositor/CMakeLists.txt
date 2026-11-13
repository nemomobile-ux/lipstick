set(TESTNAME lipstickcompositor)
set(SRC
    ut_${TESTNAME}.cpp
    ut_${TESTNAME}.h
    ../stubs/homeapplication.cpp
    ../stubs/stubbase.cpp
    ../stubs/displaystate_stub.h
    ../stubs/mdconfitem_stub.h
    ${CMAKE_SOURCE_DIR}/src/touchscreen/touchscreen.cpp)

add_executable(ut_${TESTNAME} ${SRC})
add_dependencies(ut_${TESTNAME} lipstick-qt6)
target_link_libraries(ut_${TESTNAME}
    Qt6::Core
    Qt6::Quick
    Qt6::DBus
    Qt6::Gui
    Qt6::WaylandCompositor
    Qt6::Test
    Qt6::Sensors
    Qt6::Sql
    PkgConfig::DBUS
    PkgConfig::LIBRESOURCE
    PkgConfig::MLITE
    PkgConfig::MCE
    PkgConfig::MCE_QT
    PkgConfig::NEMODEVICELOCK
    PkgConfig::SYSTEMSETTINGS
    PkgConfig::TIMED_QT
    PkgConfig::LIBSYSTEMD
    lipstick-qt6
)

add_test(ut_${TESTNAME} ut_${TESTNAME})
