// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2022 apocelipes

#include <QDebug>
#include <QLabel>
#include <QSlider>
#include <QString>
#include <QHBoxLayout>

#include "hashworker.h"
#include "settingpanel.h"

struct SettingPanelPrivate
{
    SettingPanel *q = nullptr;
    QLabel *distanceLabel = nullptr;
    QLabel *valueLabel = nullptr;
    QSlider *distanceSlider = nullptr;

    void init(SettingPanel *q_ptr);

private:
    static QString getDistanceName(int index) noexcept
    {
        // i18n需要在main函数中运行，因此不能依赖静态成员的初始化
        static const QString infoes[4] = {
            QObject::tr("fuzzy"),
            QObject::tr("default"),
            QObject::tr("precise"),
            QObject::tr("strict")
        };
        return infoes[index];
    }
};

void SettingPanelPrivate::init(SettingPanel *q_ptr)
{
    q = q_ptr;
    distanceSlider = new QSlider{Qt::Horizontal, q};
    distanceLabel = new QLabel{QObject::tr("matching accuracy: "), q};
    valueLabel = new QLabel{getDistanceName(1), q};
    distanceSlider->setRange(0, 3);
    distanceSlider->setTickInterval(1);
    distanceSlider->setTickPosition(QSlider::TickPosition::TicksBelow);
    distanceSlider->setValue(1);
    QObject::connect(distanceSlider, &QSlider::valueChanged, [this](int value) {
        valueLabel->setText(getDistanceName(value));
        switch (value) {
        case 0:
            HashWorker::similar_distance = PHashDistance::FUZZY;
            break;
        case 1:
            HashWorker::similar_distance = PHashDistance::DEFAULT;
            break;
        case 2:
            HashWorker::similar_distance = PHashDistance::PRECISE;
            break;
        case 3:
            HashWorker::similar_distance = PHashDistance::STRICT;
            break;
        default:
            qWarning() << "invalid value: " << value;
        }
    });
    auto settingsLayout = new QHBoxLayout;
    settingsLayout->addWidget(distanceLabel);
    settingsLayout->addWidget(distanceSlider);
    settingsLayout->addWidget(valueLabel);
    settingsLayout->addStretch();
    q->setLayout(settingsLayout);
}

SettingPanel::SettingPanel(QWidget *parent)
    :QWidget{parent}, d{new SettingPanelPrivate}
{
    d->init(this);
}

SettingPanel::~SettingPanel() = default;
