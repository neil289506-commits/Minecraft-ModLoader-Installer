#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QList>
#include "ModloaderInfo.h"

class SidebarButton : public QPushButton
{
    Q_OBJECT
public:
    explicit SidebarButton(ModloaderType type, QWidget *parent = nullptr);
    ModloaderType loaderType() const { return m_type; }
    void setActive(bool active);

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    ModloaderType m_type;
    bool m_active = false;
};


class SidebarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SidebarWidget(QWidget *parent = nullptr);

    void setActive(ModloaderType type);

signals:
    void loaderSelected(ModloaderType type);

private:
    QList<SidebarButton*> m_buttons;
};
