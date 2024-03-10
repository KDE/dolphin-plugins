find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    TextEditor
    WidgetsAddons
)

add_definitions(-DTRANSLATION_DOMAIN=\"fileviewhgplugin\")

kcoreaddons_add_plugin(fileviewhgplugin INSTALL_NAMESPACE "dolphin/vcs")

target_sources(fileviewhgplugin PRIVATE
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
    dialogbase.cpp
    config-widgets/generalconfig.cpp
    config-widgets/pathconfig.cpp
    config-widgets/ignorewidget.cpp
    config-widgets/pluginsettings.cpp

    fileviewhgplugin.h
    renamedialog.h
    commitdialog.h
    hgwrapper.h
    statuslist.h
    branchdialog.h
    tagdialog.h
    updatedialog.h
    clonedialog.h
    createdialog.h
    syncdialogbase.h
    mergedialog.h
    pushdialog.h
    pulldialog.h
    hgconfig.h
    configdialog.h
    commititemdelegate.h
    commitinfowidget.h
    pathselector.h
    bundledialog.h
    exportdialog.h
    importdialog.h
    servedialog.h
    servewrapper.h
    backoutdialog.h
    dialogbase.h
    config-widgets/generalconfig.h
    config-widgets/pathconfig.h
    config-widgets/ignorewidget.h
    config-widgets/pluginsettings.h
)


kconfig_add_kcfg_files(fileviewhgplugin
    fileviewhgpluginsettings.kcfgc
)

target_link_libraries(fileviewhgplugin PRIVATE
    Qt::Core
    Qt::Widgets
    KF6::KIOCore
    KF6::I18n
    KF6::CoreAddons
    KF6::Service
    KF6::WidgetsAddons
    KF6::TextEditor
    KF6::TextWidgets
    DolphinVcs
)

install(FILES fileviewhgpluginsettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
