project(fileviewhgplugin)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    TextEditor
    WidgetsAddons
)

add_definitions(-DTRANSLATION_DOMAIN=\"fileviewhgplugin\")

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
    dialogbase.cpp
    config-widgets/generalconfig.cpp
    config-widgets/pathconfig.cpp
    config-widgets/ignorewidget.cpp
    config-widgets/pluginsettings.cpp
)


kconfig_add_kcfg_files(fileviewhgplugin_SRCS
    fileviewhgpluginsettings.kcfgc
)

add_library(fileviewhgplugin MODULE ${fileviewhgplugin_SRCS})
target_link_libraries(fileviewhgplugin
    Qt5::Core
    Qt5::Widgets
    KF5::KIOCore
    KF5::I18n
    KF5::CoreAddons
    KF5::WidgetsAddons
    KF5::TextEditor
    DolphinVcs
)

install(FILES fileviewhgplugin.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(FILES fileviewhgpluginsettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(TARGETS fileviewhgplugin DESTINATION ${KDE_INSTALL_PLUGINDIR})


