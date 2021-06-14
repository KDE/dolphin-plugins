add_definitions(-DTRANSLATION_DOMAIN=\"fileviewsvnplugin\")

kcoreaddons_add_plugin(fileviewsvnplugin INSTALL_NAMESPACE "dolphin/vcs")

target_sources(fileviewsvnplugin PRIVATE
    fileviewsvnplugin.cpp
    svncommands.cpp
    svncommitdialog.cpp
    svnlogdialog.cpp
    svncheckoutdialog.cpp
    svnprogressdialog.cpp
    svncleanupdialog.cpp
)

ki18n_wrap_ui(fileviewsvnplugin
    svnlogdialog.ui
    svncheckoutdialog.ui
    svnprogressdialog.ui
    svncleanupdialog.ui
)

kconfig_add_kcfg_files(fileviewsvnplugin
    fileviewsvnpluginsettings.kcfgc
)

target_link_libraries(fileviewsvnplugin
    Qt5::Core
    Qt5::Widgets
    KF5::I18n
    KF5::XmlGui
    KF5::KIOCore
    KF5::KIOWidgets
    DolphinVcs
)

kcoreaddons_desktop_to_json(fileviewsvnplugin fileviewsvnplugin.desktop)

install(FILES fileviewsvnpluginsettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
