add_definitions(-DTRANSLATION_DOMAIN=\"fileviewbazaarplugin\")

kcoreaddons_add_plugin(fileviewbazaarplugin SOURCES fileviewbazaarplugin.cpp fileviewbazaarplugin.h INSTALL_NAMESPACE "dolphin/vcs")

target_link_libraries(fileviewbazaarplugin
    KF6::KIOCore
    KF6::I18n
    KF6::CoreAddons
    DolphinVcs
)
