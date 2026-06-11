#include "NoguiDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

NoguiDialog::NoguiDialog(const QString &modloaderName, QWidget *parent)
    : QDialog(parent, Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
    setWindowTitle("Server Configuration");
    setMinimumWidth(440);
    setModal(true);

    setStyleSheet(R"(
        QDialog  { background: #0D1117; }
        QLabel   { color: #E2E8F0; font-family: 'Segoe UI', sans-serif; }
        QCheckBox{ color: #A0AEC0; font-family: monospace; font-size: 11px; }
        QCheckBox::indicator { width:14px; height:14px; border:1px solid #4A5568;
                               border-radius:3px; background:#161B22; }
        QCheckBox::indicator:checked { background:#4299E1; border-color:#4299E1; }
    )");

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(16);

    // Title
    auto *title = new QLabel("Configure " + modloaderName + " Server", this);
    title->setStyleSheet("color:white; font-size:16px; font-weight:700;");
    root->addWidget(title);

    // Description
    auto *desc = new QLabel(
        "Start with nogui mode? (Recommended for headless / remote servers)\n\n"
        "  ✓  nogui  — terminal-only, no Swing GUI overhead\n"
        "  ✗  GUI    — shows the built-in server control panel\n\n"
        "A start_server script will be created in your install directory.\n"
        "You can change this later by editing the script.",
        this);
    desc->setStyleSheet("color:#A0AEC0; font-size:12px; line-height:1.6;");
    desc->setWordWrap(true);
    root->addWidget(desc);

    // Remember checkbox
    auto *rememberChk = new QCheckBox("Remember for this session", this);
    root->addWidget(rememberChk);

    root->addSpacing(4);

    // Buttons
    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();

    auto *yesBtn = new QPushButton("✓  Enable nogui", this);
    yesBtn->setStyleSheet(R"(
        QPushButton { background:#276749; color:white; border:none; border-radius:6px;
                      padding:10px 22px; font-family:monospace; font-weight:700; }
        QPushButton:hover { background:#2F855A; }
    )");

    auto *noBtn = new QPushButton("✗  GUI mode", this);
    noBtn->setStyleSheet(R"(
        QPushButton { background:#2C5282; color:white; border:none; border-radius:6px;
                      padding:10px 22px; font-family:monospace; font-weight:700; }
        QPushButton:hover { background:#2B6CB0; }
    )");

    btnRow->addWidget(yesBtn);
    btnRow->addWidget(noBtn);
    root->addLayout(btnRow);

    connect(yesBtn, &QPushButton::clicked, this, [this, rememberChk]() {
        m_useNogui = true;
        m_remember = rememberChk->isChecked();
        accept();
    });
    connect(noBtn,  &QPushButton::clicked, this, [this, rememberChk]() {
        m_useNogui = false;
        m_remember = rememberChk->isChecked();
        accept();
    });
}
