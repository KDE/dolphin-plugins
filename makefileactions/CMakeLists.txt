add_definitions(-DTRANSLATION_DOMAIN=\"makefileactions\")

kcoreaddons_add_plugin(makefileactions
    SOURCES makefileactions.cpp makefileactions.h
    INSTALL_NAMESPACE "kf6/kfileitemaction")

target_link_libraries(makefileactions
    KF6::I18n
    KF6::KIOWidgets
)
