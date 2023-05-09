add_definitions(-DTRANSLATION_DOMAIN=\"fileviewdropboxplugin\")

kcoreaddons_add_plugin(fileviewdropboxplugin SOURCES fileviewdropboxplugin.cpp fileviewdropboxplugin.h INSTALL_NAMESPACE "dolphin/vcs")

target_link_libraries(fileviewdropboxplugin
    Qt::Core
    Qt::Widgets
    Qt::Network
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::XmlGui
    KF${QT_MAJOR_VERSION}::KIOCore
    DolphinVcs
)
