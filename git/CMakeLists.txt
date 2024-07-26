add_definitions(-DTRANSLATION_DOMAIN=\"fileviewgitplugin\")

kcoreaddons_add_plugin(fileviewgitplugin INSTALL_NAMESPACE "dolphin/vcs")

ecm_qt_declare_logging_category(fileviewgitplugin
    HEADER dolphingit_log.h
    IDENTIFIER DolphinGitLog
    CATEGORY_NAME org.kde.dolphin.plugins.git
    DESCRIPTION "Dolphin Git Plugin"
    EXPORT DolphinGit
)

target_sources(fileviewgitplugin PRIVATE
    fileviewgitplugin.cpp
    checkoutdialog.cpp
    clonedialog.cpp
    commitdialog.cpp
    tagdialog.cpp
    progressdialog.cpp
    pushdialog.cpp
    pulldialog.cpp
    gitwrapper.cpp

    fileviewgitplugin.h
    checkoutdialog.h
    clonedialog.h
    commitdialog.h
    tagdialog.h
    progressdialog.h
    pushdialog.h
    pulldialog.h
    gitwrapper.h
)

kconfig_add_kcfg_files(fileviewgitplugin
    fileviewgitpluginsettings.kcfgc
)

target_link_libraries(fileviewgitplugin PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Concurrent
    KF6::ConfigGui
    KF6::I18n
    KF6::KIOCore
    KF6::KIOFileWidgets
    KF6::TextWidgets
    DolphinVcs
)

install(FILES fileviewgitpluginsettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

ecm_qt_install_logging_categories(
    EXPORT DolphinGit
    FILE dolphingit.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
