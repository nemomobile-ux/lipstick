set(PACKAGE notificationtool)

set(HEADERS notificationmanagerproxy.h)
set(SOURCES notificationmanagerproxy.cpp
            notificationtool.cpp)

add_executable(${PACKAGE} ${SRC} ${SOURCES})

target_link_libraries(${PACKAGE} PUBLIC
    Qt6::Core
    Qt6::DBus
    -llipstick-qt6)

include_directories(../../src
    ../../src/notifications)
