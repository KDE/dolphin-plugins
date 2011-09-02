project(fileviewbazaarplugin)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${LIBKONQ_INCLUDE_DIR} )

set(fileviewbazaarplugin_SRCS fileviewbazaarplugin.cpp)
kde4_add_plugin(fileviewbazaarplugin  ${fileviewbazaarplugin_SRCS})
target_link_libraries(fileviewbazaarplugin ${KDE4_KIO_LIBS} ${LIBKONQ_LIBRARY})
install(FILES fileviewbazaarplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS fileviewbazaarplugin DESTINATION ${PLUGIN_INSTALL_DIR})
