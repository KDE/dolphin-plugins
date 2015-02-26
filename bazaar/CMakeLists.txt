project(fileviewbazaarplugin)

add_definitions(-DTRANSLATION_DOMAIN=\"fileviewbazaarplugin\")

set(fileviewbazaarplugin_SRCS fileviewbazaarplugin.cpp)
add_library(fileviewbazaarplugin MODULE ${fileviewbazaarplugin_SRCS})
target_link_libraries(fileviewbazaarplugin
    KF5::KIOCore
    KF5::I18n
    KF5::CoreAddons
    DolphinVcs
)

install(FILES fileviewbazaarplugin.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS fileviewbazaarplugin DESTINATION ${KDE_INSTALL_PLUGINDIR})
