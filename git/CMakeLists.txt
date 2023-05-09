add_definitions(-DTRANSLATION_DOMAIN=\"fileviewgitplugin\")

kcoreaddons_add_plugin(fileviewgitplugin INSTALL_NAMESPACE "dolphin/vcs")

target_sources(fileviewgitplugin PRIVATE
    fileviewgitplugin.cpp
    checkoutdialog.cpp
    commitdialog.cpp
    tagdialog.cpp
    pushdialog.cpp
    pulldialog.cpp
    gitwrapper.cpp

    fileviewgitplugin.h
    checkoutdialog.h
    commitdialog.h
    tagdialog.h
    pushdialog.h
    pulldialog.h
    gitwrapper.h
)

kconfig_add_kcfg_files(fileviewgitplugin
    fileviewgitpluginsettings.kcfgc
)

target_link_libraries(fileviewgitplugin PRIVATE
    Qt::Core
    Qt::Widgets
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::XmlGui
    KF${QT_MAJOR_VERSION}::KIOCore
    KF${QT_MAJOR_VERSION}::KIOFileWidgets
    KF${QT_MAJOR_VERSION}::TextWidgets
    DolphinVcs
)

if (QT_MAJOR_VERSION EQUAL "6")
    target_link_libraries(
        fileviewgitplugin PRIVATE
        Qt6::Core5Compat
    )
endif()

install(FILES fileviewgitpluginsettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
