// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2022 apocelipes

#ifndef SETTINGPANEL_H
#define SETTINGPANEL_H

#include <QWidget>
#include <memory>

#include "utils.h"

class SettingPanel: public QWidget
{
    Q_OBJECT
public:
    explicit SettingPanel(QWidget *parent = nullptr) noexcept;
    ~SettingPanel() noexcept override;

    [[nodiscard]] bool isRecursiveSearching() const noexcept;
    [[nodiscard]] Utils::PHashDistance getSimilarDistance() const noexcept;
    [[nodiscard]] bool isUseTimerDialog() const noexcept;

private:
    friend struct SettingPanelPrivate;
    std::unique_ptr<struct SettingPanelPrivate> d;
};

#endif // SETTINGPANEL_H
