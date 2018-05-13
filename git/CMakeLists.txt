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

add_library(fileviewgitplugin MODULE ${fileviewgitplugin_SRCS})
target_link_libraries(fileviewgitplugin
    Qt5::Core
    Qt5::Widgets
    KF5::I18n
    KF5::XmlGui
    KF5::KIOCore
    DolphinVcs
)

install(FILES fileviewgitplugin.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(FILES fileviewgitpluginsettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(TARGETS fileviewgitplugin DESTINATION ${KDE_INSTALL_PLUGINDIR})
