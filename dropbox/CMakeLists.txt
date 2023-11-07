add_definitions(-DTRANSLATION_DOMAIN=\"fileviewdropboxplugin\")

kcoreaddons_add_plugin(fileviewdropboxplugin SOURCES fileviewdropboxplugin.cpp fileviewdropboxplugin.h INSTALL_NAMESPACE "dolphin/vcs")

target_link_libraries(fileviewdropboxplugin
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    KF6::I18n
    KF6::XmlGui
    KF6::KIOCore
    DolphinVcs
)
