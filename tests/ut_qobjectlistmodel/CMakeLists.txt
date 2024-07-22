set(TESTNAME qobjectlistmodel)
set(SRC
    ut_${TESTNAME}.cpp
    ${CMAKE_SOURCE_DIR}/src/utilities/qobjectlistmodel.cpp)

add_executable(ut_${TESTNAME} ${SRC})

target_link_libraries(ut_${TESTNAME}
    Qt6::Core
    Qt6::Quick
    Qt6::Test
    -llipstick-qt6)

add_test(ut_${TESTNAME} ut_${TESTNAME})
