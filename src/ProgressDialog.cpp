#include "ProgressDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

ProgressDialog::ProgressDialog(const QString &title, QWidget *parent)
    : QDialog(parent, Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
    setWindowTitle(title);
    setMinimumSize(580, 400);
    setModal(true);

    setStyleSheet(R"(
        QDialog {
            background: #0D1117;
        }
        QLabel {
            color: #E2E8F0;
            font-size: 13px;
            font-family: 'JetBrains Mono', monospace;
        }
        QProgressBar {
            background: #161B22;
            border: 1px solid #2D3748;
            border-radius: 6px;
            height: 12px;
            text-align: center;
            color: transparent;
        }
        QProgressBar::chunk {
            border-radius: 5px;
        }
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
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);

    m_statusLabel = new QLabel("Initializing...", this);
    m_statusLabel->setWordWrap(true);
    layout->addWidget(m_statusLabel);

    m_bar = new QProgressBar(this);
    m_bar->setRange(0, 100);
    m_bar->setValue(0);
    layout->addWidget(m_bar);

    m_log = new QTextEdit(this);
    m_log->setReadOnly(true);
    layout->addWidget(m_log);

    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();

    m_cancelBtn = new QPushButton("Cancel", this);
    m_cancelBtn->setStyleSheet(R"(
        QPushButton {
            background: #1E2D3D;
            color: #FC8181;
            border: 1px solid #742A2A;
            border-radius: 6px;
            padding: 8px 20px;
            font-family: monospace;
            font-weight: 700;
        }
        QPushButton:hover { background: #742A2A; color: white; }
    )");

    m_closeBtn = new QPushButton("Close", this);
    m_closeBtn->setVisible(false);
    m_closeBtn->setStyleSheet(R"(
        QPushButton {
            background: #1E3A2F;
            color: #68D391;
            border: 1px solid #276749;
            border-radius: 6px;
            padding: 8px 24px;
            font-family: monospace;
            font-weight: 700;
        }
        QPushButton:hover { background: #276749; color: white; }
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
    m_statusLabel->setText(msg.length() > 80 ? msg.left(77) + "..." : msg);
}

void ProgressDialog::setPercent(int percent)
{
    if (percent < 0) {
        m_bar->setRange(0, 0);  // indeterminate
    } else {
        m_bar->setRange(0, 100);
        m_bar->setValue(percent);
    }
}

void ProgressDialog::setFinished(bool success, const QString &summary)
{
    m_done = true;
    m_bar->setRange(0, 100);
    m_bar->setValue(success ? 100 : m_bar->value());

    if (success) {
        m_bar->setStyleSheet("QProgressBar::chunk { background: #48BB78; border-radius: 5px; }");
        m_statusLabel->setText(QString("✓ ") + summary);
        m_statusLabel->setStyleSheet("color: #68D391; font-size: 13px; font-family: monospace; font-weight: 700;");
    } else {
        m_bar->setStyleSheet("QProgressBar::chunk { background: #FC8181; border-radius: 5px; }");
        m_statusLabel->setText(QString("✗ ") + summary);
        m_statusLabel->setStyleSheet("color: #FC8181; font-size: 13px; font-family: monospace; font-weight: 700;");
    }

    m_log->append(QString("\n") + (success ? "=== INSTALLATION COMPLETE ===" : "=== INSTALLATION FAILED ==="));
    m_cancelBtn->setVisible(false);
    m_closeBtn->setVisible(true);
}
