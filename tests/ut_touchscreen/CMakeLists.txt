set(TESTNAME touchscreen)
set(SRC
    ut_${TESTNAME}.cpp
    ut_${TESTNAME}.h
    ${CMAKE_SOURCE_DIR}/src/touchscreen/touchscreen.cpp
    ../stubs/homeapplication.cpp
    ../stubs/stubbase.cpp)

add_executable(ut_${TESTNAME} ${SRC})

target_link_libraries(ut_${TESTNAME}
    Qt6::Core
    Qt6::Quick
    Qt6::DBus
    Qt6::Gui
    Qt6::Test
    PkgConfig::DBUS
    PkgConfig::MLITE
    PkgConfig::GLIB2
    PkgConfig::LIBRESOURCE
    PkgConfig::NEMODEVICELOCK
    PkgConfig::USB_MODED_QT
    -llipstick-qt6)

add_test(ut_${TESTNAME} ut_${TESTNAME})
