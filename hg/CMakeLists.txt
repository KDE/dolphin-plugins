project(fileviewhgplugin)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${LIBKONQ_INCLUDE_DIR} ${QIMAGEBLITZ_INCLUDES} )

set(fileviewhgplugin_SRCS
    fileviewhgplugin.cpp
    renamedialog.cpp
    hgwrapper.cpp
)

kde4_add_kcfg_files(fileviewhgplugin_SRCS fileviewhgpluginsettings.kcfgc)
kde4_add_plugin(fileviewhgplugin  ${fileviewhgplugin_SRCS})
target_link_libraries(fileviewhgplugin ${KDE4_KIO_LIBS} ${LIBKONQ_LIBRARY})
install(FILES fileviewhgplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES fileviewhgpluginsettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(TARGETS fileviewhgplugin DESTINATION ${PLUGIN_INSTALL_DIR})


