project(fileviewdropboxplugin)

add_definitions(-DTRANSLATION_DOMAIN=\"fileviewdropboxplugin\")

set(fileviewdropboxplugin_SRCS
    fileviewdropboxplugin.cpp
)

add_library(fileviewdropboxplugin MODULE ${fileviewdropboxplugin_SRCS})
target_link_libraries(fileviewdropboxplugin
    Qt5::Core
    Qt5::Widgets
    Qt5::Network
    KF5::I18n
    KF5::XmlGui
    KF5::KIOCore
    DolphinVcs
)

install(FILES fileviewdropboxplugin.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS fileviewdropboxplugin DESTINATION ${KDE_INSTALL_PLUGINDIR})
