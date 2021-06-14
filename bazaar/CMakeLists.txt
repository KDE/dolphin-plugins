add_definitions(-DTRANSLATION_DOMAIN=\"fileviewbazaarplugin\")

kcoreaddons_add_plugin(fileviewbazaarplugin SOURCES fileviewbazaarplugin.cpp INSTALL_NAMESPACE "dolphin/vcs")

target_link_libraries(fileviewbazaarplugin
    KF5::KIOCore
    KF5::I18n
    KF5::CoreAddons
    DolphinVcs
)

kcoreaddons_desktop_to_json(fileviewbazaarplugin fileviewbazaarplugin.desktop)
