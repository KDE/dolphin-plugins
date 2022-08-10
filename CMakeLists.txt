cmake_minimum_required(VERSION 3.16)

# Release service version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "22")
set(RELEASE_SERVICE_VERSION_MINOR "08")
set(RELEASE_SERVICE_VERSION_MICRO "0")

set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(dolphin-plugins VERSION ${RELEASE_SERVICE_VERSION})

set(QT_MIN_VERSION "5.15.0")
set(KF5_MIN_VERSION "5.90.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.84")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMOptionalAddSubdirectory)
include(FeatureSummary)

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050f00)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055a00)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
    Network
    DBus
)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    XmlGui
    I18n
    KIO
    TextWidgets
    CoreAddons
    Solid
)

find_package(DolphinVcs)
set_package_properties(DolphinVcs PROPERTIES
    DESCRIPTION "the Dolphin version control plugin library"
    URL "https://kde.org/applications/system/org.kde.dolphin"
    TYPE REQUIRED
    PURPOSE "Provides the version control plugin interface."
)

add_definitions(-DQT_NO_URL_CAST_FROM_STRING -DQT_NO_FOREACH -DQT_NO_KEYWORDS)

ecm_optional_add_subdirectory(svn)
ecm_optional_add_subdirectory(git)
ecm_optional_add_subdirectory(bazaar)
ecm_optional_add_subdirectory(dropbox)
ecm_optional_add_subdirectory(hg)
if(UNIX AND NOT APPLE)
    ecm_optional_add_subdirectory(mountiso)
endif()

install(FILES org.kde.dolphin-plugins.metainfo.xml
        DESTINATION ${KDE_INSTALL_METAINFODIR})

ki18n_install(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
