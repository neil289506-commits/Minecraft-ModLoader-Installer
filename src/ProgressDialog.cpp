#include "ProgressDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

ProgressDialog::ProgressDialog(const QString &title, QWidget *parent)
    : QDialog(parent, Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
    setWindowTitle(title);
    setMinimumSize(620, 480);
    setModal(true);

    setStyleSheet(R"(
        QDialog { background: #0D1117; }
        QLabel {
            color: #E2E8F0;
            font-size: 12px;
            font-family: 'JetBrains Mono', monospace;
        }
        QProgressBar {
            background: #161B22;
            border: 1px solid #2D3748;
            border-radius: 5px;
            height: 10px;
            text-align: center;
            color: transparent;
        }
        QProgressBar::chunk { border-radius: 4px; background: #4299E1; }
        QTextEdit {
            background: #0A0E14;
            color: #7EC8A0;
            border: 1px solid #1E2D3D;
            border-radius: 8px;
            font-family: 'JetBrains Mono', 'Courier New', monospace;
            font-size: 11px;
            padding: 8px;
        }
    )");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 20);
    layout->setSpacing(10);

    // ── Stage bar ──────────────────────────────────────────
    m_stageLabel = new QLabel("Initializing...", this);
    m_stageLabel->setStyleSheet("color: #63B3ED; font-size: 12px; font-family: monospace; font-weight: 700;");
    layout->addWidget(m_stageLabel);

    m_stageBar = new QProgressBar(this);
    m_stageBar->setRange(0, 100);
    m_stageBar->setValue(0);
    m_stageBar->setFixedHeight(12);
    m_stageBar->setStyleSheet(R"(
        QProgressBar { background:#161B22; border:1px solid #2D3748; border-radius:5px; }
        QProgressBar::chunk { background:#4299E1; border-radius:4px; }
    )");
    layout->addWidget(m_stageBar);

    layout->addSpacing(4);

    // ── Overall bar ────────────────────────────────────────
    m_overallLabel = new QLabel("Overall  [0/0]", this);
    m_overallLabel->setStyleSheet("color: #718096; font-size: 11px; font-family: monospace;");
    layout->addWidget(m_overallLabel);

    m_overallBar = new QProgressBar(this);
    m_overallBar->setRange(0, 100);
    m_overallBar->setValue(0);
    m_overallBar->setFixedHeight(8);
    m_overallBar->setStyleSheet(R"(
        QProgressBar { background:#161B22; border:1px solid #2D3748; border-radius:4px; }
        QProgressBar::chunk { background:#805AD5; border-radius:3px; }
    )");
    layout->addWidget(m_overallBar);

    layout->addSpacing(6);

    // ── Log ────────────────────────────────────────────────
    m_log = new QTextEdit(this);
    m_log->setReadOnly(true);
    layout->addWidget(m_log, 1);

    // ── Status + buttons ───────────────────────────────────
    m_statusLabel = new QLabel("", this);
    m_statusLabel->setStyleSheet("color: #718096; font-size: 11px; font-family: monospace;");
    m_statusLabel->setWordWrap(true);
    layout->addWidget(m_statusLabel);

    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();

    m_cancelBtn = new QPushButton("Cancel", this);
    m_cancelBtn->setStyleSheet(R"(
        QPushButton {
            background:#1E2D3D; color:#FC8181;
            border:1px solid #742A2A; border-radius:6px;
            padding:8px 20px; font-family:monospace; font-weight:700;
        }
        QPushButton:hover { background:#742A2A; color:white; }
    )");

    m_closeBtn = new QPushButton("Close", this);
    m_closeBtn->setVisible(false);
    m_closeBtn->setStyleSheet(R"(
        QPushButton {
            background:#1E3A2F; color:#68D391;
            border:1px solid #276749; border-radius:6px;
            padding:8px 24px; font-family:monospace; font-weight:700;
        }
        QPushButton:hover { background:#276749; color:white; }
    )");

    btnRow->addWidget(m_cancelBtn);
    btnRow->addWidget(m_closeBtn);
    layout->addLayout(btnRow);

    connect(m_cancelBtn, &QPushButton::clicked, this, &ProgressDialog::cancelRequested);
    connect(m_closeBtn,  &QPushButton::clicked, this, &QDialog::accept);
}

void ProgressDialog::appendLog(const QString &msg)
{
    m_log->append(msg);
    QString short_ = msg.length() > 100 ? msg.left(97) + "..." : msg;
    m_statusLabel->setText(short_);
}

void ProgressDialog::setPercent(int percent)
{
    if (percent < 0) {
        m_stageBar->setRange(0, 0);  // indeterminate
    } else {
        m_stageBar->setRange(0, 100);
        m_stageBar->setValue(percent);
    }
}

void ProgressDialog::setStage(int current, int total)
{
    m_overallLabel->setText(QString("Overall  [%1/%2]").arg(current).arg(total));
    m_overallBar->setRange(0, total);
    m_overallBar->setValue(current);

    // Update stage bar label
    if (!m_done)
        m_stageLabel->setText(QString("[%1/%2] Processing...").arg(current).arg(total));
}

void ProgressDialog::setFinished(bool success, const QString &summary)
{
    m_done = true;
    m_stageBar->setRange(0, 100);
    m_stageBar->setValue(100);

    if (success) {
        m_stageBar->setStyleSheet(R"(
            QProgressBar { background:#161B22; border:1px solid #2D3748; border-radius:5px; }
            QProgressBar::chunk { background:#48BB78; border-radius:4px; }
        )");
        m_overallBar->setStyleSheet(R"(
            QProgressBar { background:#161B22; border:1px solid #2D3748; border-radius:4px; }
            QProgressBar::chunk { background:#68D391; border-radius:3px; }
        )");
        m_overallBar->setValue(m_overallBar->maximum());
        m_stageLabel->setText("[OK] " + summary.split('\n').first());
        m_stageLabel->setStyleSheet("color:#68D391; font-size:12px; font-family:monospace; font-weight:700;");
    } else {
        m_stageBar->setStyleSheet(R"(
            QProgressBar { background:#161B22; border:1px solid #2D3748; border-radius:5px; }
            QProgressBar::chunk { background:#FC8181; border-radius:4px; }
        )");
        m_stageLabel->setText("[FAILED] " + summary.split('\n').first());
        m_stageLabel->setStyleSheet("color:#FC8181; font-size:12px; font-family:monospace; font-weight:700;");
    }

    m_log->append("\n" + QString(success ? "=== INSTALLATION COMPLETE ===" : "=== INSTALLATION FAILED ==="));
    m_statusLabel->setText(summary.split('\n').first());
    m_cancelBtn->setVisible(false);
    m_closeBtn->setVisible(true);
}
