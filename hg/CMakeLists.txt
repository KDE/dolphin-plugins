project(fileviewhgplugin)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${LIBKONQ_INCLUDE_DIR} ${QIMAGEBLITZ_INCLUDES} )

set(fileviewhgplugin_SRCS
    fileviewhgplugin.cpp
    renamedialog.cpp
    commitdialog.cpp
    hgwrapper.cpp
    statuslist.cpp
    branchdialog.cpp
    tagdialog.cpp
    updatedialog.cpp
    clonedialog.cpp
    createdialog.cpp
    syncdialogbase.cpp
    mergedialog.cpp
    pushdialog.cpp
    pulldialog.cpp
    hgconfig.cpp
    configdialog.cpp
    commititemdelegate.cpp
    commitinfowidget.cpp
    pathselector.cpp
    bundledialog.cpp
    exportdialog.cpp
    importdialog.cpp
    servedialog.cpp
    servewrapper.cpp
    backoutdialog.cpp
    config-widgets/generalconfig.cpp
    config-widgets/pathconfig.cpp
    config-widgets/ignorewidget.cpp
    config-widgets/pluginsettings.cpp
)

kde4_add_kcfg_files(fileviewhgplugin_SRCS fileviewhgpluginsettings.kcfgc)
kde4_add_plugin(fileviewhgplugin  ${fileviewhgplugin_SRCS})
target_link_libraries(fileviewhgplugin ${KDE4_KIO_LIBS} ${LIBKONQ_LIBRARY} ${KDE4_KTEXTEDITOR_LIBS} )
install(FILES fileviewhgplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES fileviewhgpluginsettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(TARGETS fileviewhgplugin DESTINATION ${PLUGIN_INSTALL_DIR})


