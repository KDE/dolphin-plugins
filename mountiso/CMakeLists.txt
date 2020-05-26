project(mountisoplugin)

kcoreaddons_add_plugin(
    mountisoaction
    SOURCES mountisoaction.cpp
    JSON mountisoaction.json
    INSTALL_NAMESPACE "kf5/kfileitemaction")

target_link_libraries(mountisoaction
    Qt5::DBus
    KF5::I18n
    KF5::KIOWidgets
    KF5::Solid)
