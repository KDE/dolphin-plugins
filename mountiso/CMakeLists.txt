add_definitions(-DTRANSLATION_DOMAIN=\"mountisoaction\")

kcoreaddons_add_plugin(
    mountisoaction
    SOURCES mountisoaction.cpp mountisoaction.h
    INSTALL_NAMESPACE "kf6/kfileitemaction")

target_link_libraries(mountisoaction
    Qt6::DBus
    KF6::I18n
    KF6::KIOWidgets
    KF6::Solid
)
