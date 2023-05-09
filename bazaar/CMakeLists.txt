add_definitions(-DTRANSLATION_DOMAIN=\"fileviewbazaarplugin\")

kcoreaddons_add_plugin(fileviewbazaarplugin SOURCES fileviewbazaarplugin.cpp fileviewbazaarplugin.h INSTALL_NAMESPACE "dolphin/vcs")

target_link_libraries(fileviewbazaarplugin
    KF${QT_MAJOR_VERSION}::KIOCore
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::CoreAddons
    DolphinVcs
)
