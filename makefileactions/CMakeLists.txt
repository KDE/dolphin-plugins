add_definitions(-DTRANSLATION_DOMAIN=\"makefileactions\")

kcoreaddons_add_plugin(makefileactions
    SOURCES makefileactions.cpp makefileactions.h
    INSTALL_NAMESPACE "kf${QT_MAJOR_VERSION}/kfileitemaction")

target_link_libraries(makefileactions
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::KIOWidgets)
