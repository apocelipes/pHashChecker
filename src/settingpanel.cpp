// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2022 apocelipes

#include <QCheckBox>
#include <QLabel>
#include <QSlider>
#include <QString>
#include <QHBoxLayout>
#include <QSettings>

#include "settingpanel.h"

#define SETTINGS_PREFIX "settingpanel/"

using PHashDistance = Utils::PHashDistance;

namespace {
    constexpr std::array distances = {
        PHashDistance::FUZZY,
        PHashDistance::DEFAULT,
        PHashDistance::PRECISE,
        PHashDistance::STRICT,
    };

    inline constexpr std::size_t distance2Index(const PHashDistance distance) noexcept
    {
        auto ret  = Utils::indexOf(distances, distance);
        return ret.value_or(1);
    }

    inline constexpr std::optional<PHashDistance> index2Distance(const int idx) noexcept
    {
        if (idx < 0 || static_cast<std::size_t>(idx) >= distances.size()) [[unlikely]] {
            return std::nullopt;
        }

        return distances[idx];
    }

    const char *pHashDistanceIndexKey = SETTINGS_PREFIX"pHashDistanceIndex";
    const char *recursiveSearchKey = SETTINGS_PREFIX"recursiveSearch";
    const char *useStopwatchDialogKey = SETTINGS_PREFIX"useStopwatchDialog";

    constexpr int defaultPHashDistanceIndex = static_cast<int>(distance2Index(PHashDistance::DEFAULT));
    constexpr bool defaultRecursiveSearch = false;
    constexpr bool defaultUseStopwatchDialog = false;
}

struct SettingPanelPrivate
{
    SettingPanel *q = nullptr;
    QLabel *distanceLabel = nullptr;
    QLabel *valueLabel = nullptr;
    QSlider *distanceSlider = nullptr;
    QCheckBox *recursiveSearchChecker = nullptr;
    QCheckBox *stopwatchDialogChecker = nullptr;
    QSettings settings;

    void init(SettingPanel *q_ptr) noexcept;

    SettingPanelPrivate() noexcept
    : settings{"apocelipes", "pHashChecker"}
    {}

    ~SettingPanelPrivate() noexcept
    {
        settings.setValue(pHashDistanceIndexKey, distanceSlider->value());
        settings.setValue(recursiveSearchKey, recursiveSearchChecker->isChecked());
        settings.setValue(useStopwatchDialogKey, stopwatchDialogChecker->isChecked());
        settings.sync();
    }

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
    valueLabel = new QLabel{q};
    distanceSlider->setRange(0, 3);
    distanceSlider->setTickInterval(1);
    distanceSlider->setTickPosition(QSlider::TickPosition::TicksBelow);
    distanceSlider->setMinimumWidth(100);
    QObject::connect(distanceSlider, &QSlider::valueChanged, q, [this](int val) noexcept {
        q->setToolTip(getDistanceToolTip(val));
    });
    QObject::connect(distanceSlider, &QSlider::valueChanged, valueLabel, [this](int value) noexcept {
        valueLabel->setText(getDistanceName(value));
    });
    const auto distance = settings.value(pHashDistanceIndexKey, defaultPHashDistanceIndex).toInt();
    distanceSlider->setValue(distance);
    valueLabel->setText(getDistanceName(distance));

    recursiveSearchChecker = new QCheckBox{QObject::tr("recursive searching"), q};
    recursiveSearchChecker->setChecked(settings.value(recursiveSearchKey, defaultRecursiveSearch).toBool());
    recursiveSearchChecker->setToolTip(QObject::tr("Recursively searches all images in the current directory and its subdirectories."));

    stopwatchDialogChecker = new QCheckBox{QObject::tr("record calculation time"), q};
    stopwatchDialogChecker->setToolTip(QObject::tr("Record the time spent on calculation."));
    stopwatchDialogChecker->setChecked(settings.value(useStopwatchDialogKey, defaultUseStopwatchDialog).toBool());

    auto settingsLayout = new QHBoxLayout;
    settingsLayout->addWidget(distanceLabel);
    settingsLayout->addWidget(distanceSlider);
    settingsLayout->addWidget(valueLabel);
    settingsLayout->addWidget(recursiveSearchChecker);
    settingsLayout->addWidget(stopwatchDialogChecker);
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
    const auto value = d->distanceSlider->value();
    const auto distance = index2Distance(value);
    if (!distance.has_value()) [[unlikely]] {
        qWarning() << tr("returning DEFAULT because of invalid value: ") << value;
    }

    return distance.value_or(PHashDistance::DEFAULT);
}

bool SettingPanel::isUseStopwatchDialog() const noexcept
{
    return d->stopwatchDialogChecker->isChecked();
}
