project(fileviewgitplugin)

add_definitions(-DTRANSLATION_DOMAIN=\"fileviewgitplugin\")

set(fileviewgitplugin_SRCS
    fileviewgitplugin.cpp
    checkoutdialog.cpp
    commitdialog.cpp
    tagdialog.cpp
    pushdialog.cpp
    pulldialog.cpp
    gitwrapper.cpp
)

kconfig_add_kcfg_files(fileviewgitplugin_SRCS
    fileviewgitpluginsettings.kcfgc
)

kcoreaddons_add_plugin(fileviewgitplugin SOURCES ${fileviewgitplugin_SRCS} INSTALL_NAMESPACE "dolphin/vcs")

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

kcoreaddons_desktop_to_json(fileviewgitplugin fileviewgitplugin.desktop)

install(FILES fileviewgitpluginsettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
