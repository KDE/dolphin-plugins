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
)

kconfig_add_kcfg_files(fileviewgitplugin
    fileviewgitpluginsettings.kcfgc
)

target_link_libraries(fileviewgitplugin
    Qt5::Core
    Qt5::Widgets
    KF5::I18n
    KF5::XmlGui
    KF5::KIOCore
    KF5::KIOFileWidgets
    KF5::TextWidgets
    DolphinVcs
)

install(FILES fileviewgitpluginsettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
