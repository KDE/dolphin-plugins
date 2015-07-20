project(fileviewsvnplugin)

add_definitions(-DTRANSLATION_DOMAIN=\"fileviewsvnplugin\")

set(fileviewsvnplugin_SRCS
    fileviewsvnplugin.cpp
)

kconfig_add_kcfg_files(fileviewsvnplugin_SRCS
    fileviewsvnpluginsettings.kcfgc
)

add_library(fileviewsvnplugin MODULE ${fileviewsvnplugin_SRCS})
target_link_libraries(fileviewsvnplugin
    Qt5::Core
    Qt5::Widgets
    KF5::I18n
    KF5::XmlGui
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::KDELibs4Support
    DolphinVcs
)

install(FILES fileviewsvnplugin.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(FILES fileviewsvnpluginsettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(TARGETS fileviewsvnplugin DESTINATION ${KDE_INSTALL_PLUGINDIR})
