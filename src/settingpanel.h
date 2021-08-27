#ifndef SETTINGPANEL_H
#define SETTINGPANEL_H

#include <QWidget>
#include <memory>

class SettingPanel: public QWidget
{
    Q_OBJECT
public:
    explicit SettingPanel(QWidget *parent = nullptr);
    ~SettingPanel();

private:
    friend struct SettingPanelPrivate;
    std::unique_ptr<struct SettingPanelPrivate> d;
};

#endif // SETTINGPANEL_H
