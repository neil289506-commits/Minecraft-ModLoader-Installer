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
    void setPercent(int percent);            // stage bar: 0-100, -1 = indeterminate
    void setStage(int current, int total);   // overall bar
    void setFinished(bool success, const QString &summary);

signals:
    void cancelRequested();

private:
    QLabel       *m_stageLabel;   // "[2/3] Downloading..."
    QProgressBar *m_stageBar;     // per-step progress
    QLabel       *m_overallLabel; // "Overall"
    QProgressBar *m_overallBar;   // overall stages
    QTextEdit    *m_log;
    QLabel       *m_statusLabel;
    QPushButton  *m_cancelBtn;
    QPushButton  *m_closeBtn;
    bool          m_done = false;
};
