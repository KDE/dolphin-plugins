project(fileviewgitplugin)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${QIMAGEBLITZ_INCLUDES} )

set(fileviewgitplugin_SRCS
    fileviewgitplugin.cpp
)

kde4_add_plugin(fileviewgitplugin  ${fileviewgitplugin_SRCS})
target_link_libraries(fileviewgitplugin ${KDE4_KIO_LIBS} konq)
install(FILES fileviewgitplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS fileviewgitplugin DESTINATION ${PLUGIN_INSTALL_DIR})
