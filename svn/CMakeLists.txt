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

    fileviewsvnplugin.h
    svncommands.h
    svncommitdialog.h
    svnlogdialog.h
    svncheckoutdialog.h
    svnprogressdialog.h
    svncleanupdialog.h
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
    Qt6::Core
    Qt6::Widgets
    KF6::I18n
    KF6::XmlGui
    KF6::KIOCore
    KF6::KIOWidgets
    DolphinVcs
)

install(FILES fileviewsvnpluginsettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
