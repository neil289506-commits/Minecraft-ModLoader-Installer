#pragma once
#include <QDialog>
#include <QLabel>
#include <QProgressBar>
#include <QTextEdit>
#include <QPushButton>

class ProgressDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ProgressDialog(const QString &title, QWidget *parent = nullptr);

    void appendLog(const QString &msg);
    void setPercent(int percent);   // -1 = indeterminate
    void setFinished(bool success, const QString &summary);

signals:
    void cancelRequested();

private:
    QLabel       *m_statusLabel;
    QProgressBar *m_bar;
    QTextEdit    *m_log;
    QPushButton  *m_cancelBtn;
    QPushButton  *m_closeBtn;
    bool          m_done = false;
};
