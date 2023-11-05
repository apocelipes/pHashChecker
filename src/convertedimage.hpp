// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 apocelipes

#ifndef CONVERTEDIMAGE_H
#define CONVERTEDIMAGE_H

#include <optional>

#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QProcess>

#include "utils.h"

class ConvertedImage {
    std::optional<QString> path;
    bool isKeepAlive;

    [[nodiscard]] static QString getConvertedFullName(const QString &srcPath, const int width, const int height) noexcept {
        const auto &tmpPath = Utils::getTempDirPath();
        const auto fileName = QFileInfo{srcPath}.baseName();
        const auto &name = QString("%1_%2x%3.jpg").arg(fileName).arg(width).arg(height);
        return QDir::cleanPath(tmpPath + QDir::separator() + name);
    }

    void clear() noexcept {
        if (!isKeepAlive && path && *path != "") {
            QFile::remove(*path);
        }
    }
public:
    ConvertedImage(const QString &srcPath, const int width, const int height, bool keepAlive = false) noexcept
    : isKeepAlive{keepAlive} {
        if (!QFile::exists(srcPath)) [[unlikely]] {
            return;
        }
        const auto &newPath = getConvertedFullName(srcPath, width, height);
        path.emplace(newPath);
        // maybe some TOCTOU problems, but no impacts on this code
        if (QFile::exists(newPath) && isKeepAlive) {
            return;
        }
        QStringList arguments;
        arguments << srcPath
                  << "-quality" << "75"
                  << "-resize" << QString("%1x%2!").arg(width).arg(height)
                  << newPath;
        QProcess cmd;
        cmd.start("magick", arguments);
        if (!cmd.waitForFinished()) {
            qFatal() << QObject::tr("create converted image failed");
        }
        if (cmd.exitCode() != 0) {
            qFatal() << QObject::tr("call magick failed:") << QString{cmd.readAllStandardError()};
        }
    }

    ~ConvertedImage() noexcept {
        clear();
    }

    ConvertedImage(const ConvertedImage&) = delete;
    ConvertedImage& operator=(const ConvertedImage&) = delete;

    ConvertedImage(ConvertedImage &&other) noexcept: path{std::move(other.path)} {}
    ConvertedImage& operator=(ConvertedImage &&other) noexcept {
        clear();
        path = std::move(other.path);
        return *this;
    }

    [[nodiscard]] QString getImagePath() const noexcept {
        return path ? *path : "";
    }
};

#endif // CONVERTEDIMAGE_H
