add_definitions(-DTRANSLATION_DOMAIN=\"mountisoaction\")

kcoreaddons_add_plugin(
    mountisoaction
    SOURCES mountisoaction.cpp mountisoaction.h
    INSTALL_NAMESPACE "kf${QT_MAJOR_VERSION}/kfileitemaction")

target_link_libraries(mountisoaction
    Qt::DBus
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::KIOWidgets
    KF${QT_MAJOR_VERSION}::Solid)
