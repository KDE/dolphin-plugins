add_definitions(-DTRANSLATION_DOMAIN=\"fileviewgitplugin\")

kcoreaddons_add_plugin(fileviewgitplugin INSTALL_NAMESPACE "dolphin/vcs")

target_sources(fileviewgitplugin PRIVATE
    fileviewgitplugin.cpp
    checkoutdialog.cpp
    commitdialog.cpp
    tagdialog.cpp
    pushdialog.cpp
    pulldialog.cpp
    gitwrapper.cpp

    fileviewgitplugin.h
    checkoutdialog.h
    commitdialog.h
    tagdialog.h
    pushdialog.h
    pulldialog.h
    gitwrapper.h
)

kconfig_add_kcfg_files(fileviewgitplugin
    fileviewgitpluginsettings.kcfgc
)

target_link_libraries(fileviewgitplugin PRIVATE
    Qt6::Core
    Qt6::Core5Compat
    Qt6::Widgets
    KF6::ConfigGui
    KF6::I18n
    KF6::KIOCore
    KF6::KIOFileWidgets
    KF6::TextWidgets
    DolphinVcs
)

install(FILES fileviewgitpluginsettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
