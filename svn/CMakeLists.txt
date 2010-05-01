project(fileviewsvnplugin)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${LIBKONQ_INCLUDE_DIR} ${QIMAGEBLITZ_INCLUDES} )

set(fileviewsvnplugin_SRCS fileviewsvnplugin.cpp)
kde4_add_kcfg_files(fileviewsvnplugin_SRCS fileviewsvnpluginsettings.kcfgc)
kde4_add_plugin(fileviewsvnplugin  ${fileviewsvnplugin_SRCS})
target_link_libraries(fileviewsvnplugin ${KDE4_KIO_LIBS} ${LIBKONQ_LIBRARY})
install(FILES fileviewsvnplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES fileviewsvnpluginsettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(TARGETS fileviewsvnplugin DESTINATION ${PLUGIN_INSTALL_DIR})
