set(TESTNAME qobjectlistmodel)
set(SRC
    ut_${TESTNAME}.cpp
    ${CMAKE_SOURCE_DIR}/src/utilities/qobjectlistmodel.cpp)

add_executable(ut_${TESTNAME} ${SRC})
add_dependencies(ut_${TESTNAME} lipstick-qt6)
target_link_libraries(ut_${TESTNAME}
    Qt6::Core
    Qt6::Quick
    Qt6::Test
    -llipstick-qt6 -L../../src)

add_test(ut_${TESTNAME} ut_${TESTNAME})
