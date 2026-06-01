# SPDX-FileCopyrightText: 2026 Kai Uwe Broulik <kde@broulik.de>
# SPDX-License-Identifier: BSD-3-Clause

kcoreaddons_add_plugin(
    mountedisooverlay
    SOURCES mountedisooverlay.cpp mountedisooverlay.h
    INSTALL_NAMESPACE "kf6/overlayicon"
)

ecm_qt_declare_logging_category(mountedisooverlay
    HEADER mountedisooverlay_debug.h
    IDENTIFIER lcMountedIsoOverlay
    CATEGORY_NAME org.kde.dolphin.plugins.mountedisooverlay
    DESCRIPTION "Dolphin Mounted Iso Overlay Plugin"
)

target_link_libraries(mountedisooverlay
    KF6::KIOCore
    KF6::Solid
)
