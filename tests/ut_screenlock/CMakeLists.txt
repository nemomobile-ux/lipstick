set(TESTNAME screenlock)
set(SRC
    ut_${TESTNAME}.cpp
    ut_${TESTNAME}.h
    ../stubs/stubbase.cpp
    ../stubs/homeapplication.cpp
    ${CMAKE_SOURCE_DIR}/src/screenlock/screenlock.cpp
    ${CMAKE_SOURCE_DIR}/src/touchscreen/touchscreen.cpp)

add_executable(ut_${TESTNAME} ${SRC})

target_link_libraries(ut_${TESTNAME}
    Qt6::Core
    Qt6::Quick
    Qt6::DBus
    Qt6::Test
    -llipstick-qt6)

add_test(ut_${TESTNAME} ut_${TESTNAME})
