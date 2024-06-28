// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2022 apocelipes

#include <QCheckBox>
#include <QLabel>
#include <QSlider>
#include <QString>
#include <QHBoxLayout>

#include "settingpanel.h"

struct SettingPanelPrivate
{
    SettingPanel *q = nullptr;
    QLabel *distanceLabel = nullptr;
    QLabel *valueLabel = nullptr;
    QSlider *distanceSlider = nullptr;
    QCheckBox *recursiveSearchChecker = nullptr;
    QCheckBox *timerDialogChecker = nullptr;

    void init(SettingPanel *q_ptr) noexcept;

private:
    [[nodiscard]] static QString getDistanceName(int index) noexcept
    {
        // i18n需要在main函数中运行，因此不能依赖静态成员的初始化
        static const QString infoes[4] = {
            QObject::tr("fuzzy"),
            QObject::tr("default"),
            QObject::tr("precise"),
            QObject::tr("strict")
        };
        if (index >= 4 || index < 0) {
            return QString{};
        }
        return infoes[index];
    }

    [[nodiscard]] static QString getDistanceToolTip(int index) noexcept {
        static const QString toolTips[4] = {
             QObject::tr("Fuzzy mode may produce more erroneous results."),
             QObject::tr("The default mode."),
             QObject::tr("This mode provides higher precision than the default mode."),
             QObject::tr("Strict mode provides the highest matching accuracy."),
        };
        if (index >= 4 || index < 0) {
            return QString{};
        }
        return toolTips[index];
    }
};

void SettingPanelPrivate::init(SettingPanel *q_ptr) noexcept
{
    q = q_ptr;
    distanceSlider = new QSlider{Qt::Horizontal, q};
    distanceLabel = new QLabel{QObject::tr("matching accuracy: "), q};
    valueLabel = new QLabel{getDistanceName(1), q};
    distanceSlider->setRange(0, 3);
    distanceSlider->setTickInterval(1);
    distanceSlider->setTickPosition(QSlider::TickPosition::TicksBelow);
    distanceSlider->setMinimumWidth(100);
    QObject::connect(distanceSlider, &QSlider::valueChanged, q, [this](int val) {
        q->setToolTip(getDistanceToolTip(val));
    });
    distanceSlider->setValue(1);
    QObject::connect(distanceSlider, &QSlider::valueChanged, [this](int value) {
        valueLabel->setText(getDistanceName(value));
    });

    recursiveSearchChecker = new QCheckBox{QObject::tr("recursive searching"), q};
    recursiveSearchChecker->setToolTip(QObject::tr("Recursively searches all images in the current directory and its subdirectories."));

    timerDialogChecker = new QCheckBox{QObject::tr("record calculation time"), q};
    timerDialogChecker->setToolTip(QObject::tr("Record the time spent on calculation."));
    timerDialogChecker->setChecked(true);

    auto settingsLayout = new QHBoxLayout;
    settingsLayout->addWidget(distanceLabel);
    settingsLayout->addWidget(distanceSlider);
    settingsLayout->addWidget(valueLabel);
    settingsLayout->addWidget(recursiveSearchChecker);
    settingsLayout->addWidget(timerDialogChecker);
    q->setLayout(settingsLayout);
}

SettingPanel::SettingPanel(QWidget *parent) noexcept
    :QWidget{parent}, d{new SettingPanelPrivate}
{
    d->init(this);
}

SettingPanel::~SettingPanel() noexcept = default;

bool SettingPanel::isRecursiveSearching() const noexcept
{
    return d->recursiveSearchChecker->isChecked();
}

Utils::PHashDistance SettingPanel::getSimilarDistance() const noexcept
{
    auto value = d->distanceSlider->value();
    switch (value) {
    case 0:
        return Utils::PHashDistance::FUZZY;
    case 1:
        return Utils::PHashDistance::DEFAULT;
    case 2:
        return Utils::PHashDistance::PRECISE;
    case 3:
        return Utils::PHashDistance::STRICT;
    default:
        qWarning() << tr("returning DEFAULT because of invalid value: ") << value;
        return Utils::PHashDistance::DEFAULT;
    }
}

bool SettingPanel::isUseTimerDialog() const noexcept
{
    return d->timerDialogChecker->isChecked();
}
