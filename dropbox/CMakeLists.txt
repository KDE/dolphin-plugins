add_definitions(-DTRANSLATION_DOMAIN=\"fileviewdropboxplugin\")

kcoreaddons_add_plugin(fileviewdropboxplugin SOURCES fileviewdropboxplugin.cpp INSTALL_NAMESPACE "dolphin/vcs")

target_link_libraries(fileviewdropboxplugin
    Qt::Core
    Qt::Widgets
    Qt::Network
    KF5::I18n
    KF5::XmlGui
    KF5::KIOCore
    DolphinVcs
)
