cmake_minimum_required(VERSION 3.16)

# Release service version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "24")
set(RELEASE_SERVICE_VERSION_MINOR "07")
set(RELEASE_SERVICE_VERSION_MICRO "70")

set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(dolphin-plugins VERSION ${RELEASE_SERVICE_VERSION})

set(QT_MIN_VERSION "6.5.0")
set(KF_MIN_VERSION "5.240.0")

# ECM setup
find_package(ECM ${KF_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

include(QtVersionOption)
include(ECMSetupVersion)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMDeprecationSettings)
include(ECMOptionalAddSubdirectory)
include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Core
    Widgets
    Network
    DBus
    Concurrent
)

find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    XmlGui
    I18n
    KIO
    TextWidgets
    Config
    CoreAddons
    Solid
)

find_package(DolphinVcs REQUIRED)
set_package_properties(DolphinVcs PROPERTIES
    DESCRIPTION "the Dolphin version control plugin library"
    URL "https://commits.kde.org/dolphin"
    TYPE REQUIRED
    PURPOSE "Provides the version control plugin interface."
)

ecm_set_disabled_deprecation_versions(
    QT 6.5
    KF 5.240
)

ecm_qt_declare_logging_category(dolphin-plugins
    HEADER dolphinpluginsdebug.h
    IDENTIFIER GitPluginDebug
    CATEGORY_NAME org.kde.dolphin
    DESCRIPTION "dolphin-plugins"
    EXPORT DOLPHINPLUGINS
)
add_library(DolphinPluginsLogging SHARED)
target_sources(DolphinPluginsLogging PUBLIC ${PROJECT_BINARY_DIR}/dolphinpluginsdebug.h ${PROJECT_BINARY_DIR}/dolphinpluginsdebug.cpp)
target_link_libraries(DolphinPluginsLogging PRIVATE Qt6::Core)

ecm_optional_add_subdirectory(svn)
ecm_optional_add_subdirectory(git)
ecm_optional_add_subdirectory(bazaar)
ecm_optional_add_subdirectory(dropbox)
ecm_optional_add_subdirectory(hg)
if(UNIX)
    ecm_optional_add_subdirectory(makefileactions)
endif()
if(UNIX AND NOT APPLE)
    ecm_optional_add_subdirectory(mountiso)
endif()

install(FILES org.kde.dolphin-plugins.metainfo.xml
        DESTINATION ${KDE_INSTALL_METAINFODIR})

ki18n_install(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

include(KDEClangFormat)
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

include(KDEGitCommitHooks)
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
