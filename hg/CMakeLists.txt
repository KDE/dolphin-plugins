find_package(KF${QT_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS
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
    KF${QT_MAJOR_VERSION}::KIOCore
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::CoreAddons
    KF${QT_MAJOR_VERSION}::WidgetsAddons
    KF${QT_MAJOR_VERSION}::TextEditor
    DolphinVcs
)

if (QT_MAJOR_VERSION EQUAL "6")
    target_link_libraries(
        fileviewhgplugin PRIVATE
        Qt6::Core5Compat
        KF6::TextWidgets
        KF6::Service
    )
endif()

install(FILES fileviewhgpluginsettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})


