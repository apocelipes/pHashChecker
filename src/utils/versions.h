// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 apocelipes

#pragma once

#include <QString>
#include <QtVersion>

#include <CImg.h>
#include <cpp-sort/version.h>
#include <pHash.h>
#include <ankerl/unordered_dense.h>

#define PHASHCHECKER_VERSION_MAJOR 0
#define PHASHCHECKER_VERSION_MINOR 1
#define PHASHCHECKER_VERSION_PATCH 0

namespace {
    inline QString semVersion(int major, int minor, int patch) noexcept {
        return QStringLiteral(u"%1.%2.%3").arg(major)
                                           .arg(minor)
                                           .arg(patch);
    }
}

namespace Utils {
    inline QString getCImgVersion() noexcept {
        return semVersion(cimg_version / 100, (cimg_version / 10) % 10, cimg_version % 10);
    }

    inline QString getCppSortVersion() noexcept {
        return semVersion(CPPSORT_VERSION_MAJOR, CPPSORT_VERSION_MINOR, CPPSORT_VERSION_PATCH);
    }

    inline QString getPHashVersion() noexcept {
        return QString{ph_version()};
    }

    inline QString getAnkerlUnorderedDenseVersion() noexcept {
        return semVersion(ANKERL_UNORDERED_DENSE_VERSION_MAJOR, ANKERL_UNORDERED_DENSE_VERSION_MINOR, ANKERL_UNORDERED_DENSE_VERSION_PATCH);
    }

    inline QString getQtVersion() noexcept {
        return qVersion();
    }

    inline QString getPHashCheckerVersion() noexcept {
        return semVersion(PHASHCHECKER_VERSION_MAJOR, PHASHCHECKER_VERSION_MINOR, PHASHCHECKER_VERSION_PATCH);
    }
}
