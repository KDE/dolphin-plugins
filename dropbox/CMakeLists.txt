project(fileviewdropboxplugin)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${LIBKONQ_INCLUDE_DIR} )

set(fileviewdropboxplugin_SRCS
    fileviewdropboxplugin.cpp
)

kde4_add_plugin(fileviewdropboxplugin  ${fileviewdropboxplugin_SRCS})
target_link_libraries(fileviewdropboxplugin ${KDE4_KIO_LIBS} ${LIBKONQ_LIBRARY})
install(FILES fileviewdropboxplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS fileviewdropboxplugin DESTINATION ${PLUGIN_INSTALL_DIR})