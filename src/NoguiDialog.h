#pragma once
#include <QDialog>
#include <QPushButton>
#include <QCheckBox>

class NoguiDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NoguiDialog(const QString &modloaderName, QWidget *parent = nullptr);
    bool useNogui()   const { return m_useNogui; }
    bool rememberIt() const { return m_remember; }

private:
    bool m_useNogui = true;
    bool m_remember = false;
};
