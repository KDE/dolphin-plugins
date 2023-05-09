cmake_minimum_required(VERSION 3.16)

# Release service version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "23")
set(RELEASE_SERVICE_VERSION_MINOR "07")
set(RELEASE_SERVICE_VERSION_MICRO "70")

set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(dolphin-plugins VERSION ${RELEASE_SERVICE_VERSION})

if (DEFINED BUILD_WITH_QT6)
    set(QT_MIN_VERSION "6.4.0")
    set(KF_MIN_VERSION "5.240.0")
else()
    set(QT_MIN_VERSION "5.15.2")
    set(KF_MIN_VERSION "5.90.0")
endif()

set(KDE_COMPILERSETTINGS_LEVEL "5.84")

# ECM setup
find_package(ECM ${KF_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

include(QtVersionOption)
include(ECMSetupVersion)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMOptionalAddSubdirectory)
include(FeatureSummary)

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050f00)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055a00)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
    Network
    DBus
)
if (QT_MAJOR_VERSION EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Core5Compat)
endif()

find_package(KF${QT_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS
    XmlGui
    I18n
    KIO
    TextWidgets
    CoreAddons
    Solid
)

find_package(DolphinVcs REQUIRED)
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
