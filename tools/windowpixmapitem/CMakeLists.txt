set(PACKAGE windowpixmapitem)

set(SRC main.cpp)

add_executable(${PACKAGE} ${SRC})
add_dependencies(${PACKAGE} lipstick-qt6 lipstickplugin)
target_link_libraries(${PACKAGE} PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Qml
    lipstick-qt6
)

target_compile_definitions(${PACKAGE} PRIVATE
    QML_FILE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/windowpixmapitem.qml")

set_target_properties(${PACKAGE} PROPERTIES
    BUILD_RPATH "${CMAKE_BINARY_DIR}/src;${CMAKE_BINARY_DIR}/plugin")

install(TARGETS ${PACKAGE} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES windowpixmapitem.qml DESTINATION ${CMAKE_INSTALL_DATADIR}/lipstick/windowpixmapitem)