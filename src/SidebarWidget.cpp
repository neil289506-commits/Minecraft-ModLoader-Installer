#include "SidebarWidget.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>

// ─────────────────────────── SidebarButton ────────────────────────────────

SidebarButton::SidebarButton(ModloaderType type, QWidget *parent)
    : QPushButton(parent), m_type(type)
{
    auto cap = capabilityOf(type);
    setText(modloaderName(type));
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    setFixedHeight(56);

    // 加入圖示
    QString iconPath;
    switch (type) {
        case ModloaderType::Vanilla:   iconPath = ":/icons/vanilla.png"; break;
        case ModloaderType::Fabric:    iconPath = ":/icons/fabric.png"; break;
        case ModloaderType::Quilt:     iconPath = ":/icons/quilt.png"; break;
        case ModloaderType::Forge:     iconPath = ":/icons/forge.png"; break;
        case ModloaderType::NeoForge:  iconPath = ":/icons/neoforge.png"; break;
        case ModloaderType::OptiFine:  iconPath = ":/icons/optifine.png"; break;
    }
    setIcon(QIcon(iconPath));
    setIconSize(QSize(20, 20)); // 調整圖示大小

    QString style = QString(R"(
        QPushButton {
            text-align: left;
            padding-left: 40px;   /* 留空間給圖示 */
            border: none;
            border-left: 3px solid transparent;
            background: transparent;
            color: #8A9BA8;
            font-size: 13px;
            font-family: 'JetBrains Mono', 'Courier New', monospace;
            font-weight: 600;
            letter-spacing: 1px;
        }
        QPushButton:hover {
            background: rgba(255,255,255,0.04);
            color: #CBD5E0;
            border-left: 3px solid %1;
        }
        QPushButton:checked {
            background: rgba(%2, 0.12);
            color: white;
            border-left: 3px solid %1;
        }
    )").arg(cap.color)
       .arg(QColor(cap.color).red())
       + QString(", %1, %2").arg(QColor(cap.color).green()).arg(QColor(cap.color).blue());

    setStyleSheet(style);
}

void SidebarButton::setActive(bool active)
{
    m_active = active;
    setChecked(active);
}

void SidebarButton::paintEvent(QPaintEvent *e)
{
    QPushButton::paintEvent(e);
}

// ─────────────────────────── SidebarWidget ────────────────────────────────

SidebarWidget::SidebarWidget(QWidget *parent) : QWidget(parent)
{
    setFixedWidth(200);
    setStyleSheet("background-color: #0D1117;");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    // Header / logo area
    auto *header = new QLabel("⛏ MC Installer", this);
    header->setFixedHeight(72);
    header->setAlignment(Qt::AlignCenter);
    header->setStyleSheet(R"(
        color: #E2E8F0;
        font-size: 14px;
        font-family: 'JetBrains Mono', monospace;
        font-weight: 700;
        letter-spacing: 2px;
        border-bottom: 1px solid #1E2D3D;
        background: #0A0E14;
    )");
    layout->addWidget(header);

    layout->addSpacing(12);

    // Category label
    auto *catLabel = new QLabel("  MODLOADERS", this);
    catLabel->setStyleSheet(R"(
        color: #4A5568;
        font-size: 10px;
        font-family: 'JetBrains Mono', monospace;
        font-weight: 700;
        letter-spacing: 2px;
        padding-left: 16px;
    )");
    catLabel->setFixedHeight(24);
    layout->addWidget(catLabel);

    const QList<ModloaderType> types = {
        ModloaderType::Vanilla,
        ModloaderType::Fabric,
        ModloaderType::Quilt,
        ModloaderType::Forge,
        ModloaderType::NeoForge,
        ModloaderType::OptiFine
    };

    for (auto t : types) {
        auto *btn = new SidebarButton(t, this);
        m_buttons.append(btn);
        layout->addWidget(btn);

        connect(btn, &QPushButton::clicked, this, [this, btn, t]() {
            for (auto *b : m_buttons) b->setActive(false);
            btn->setActive(true);
            emit loaderSelected(t);
        });
    }

    layout->addStretch();

    // Version info at bottom
    auto *verLabel = new QLabel("v1.0.0", this);
    verLabel->setAlignment(Qt::AlignCenter);
    verLabel->setFixedHeight(32);
    verLabel->setStyleSheet("color: #2D3748; font-size: 10px; font-family: monospace;");
    layout->addWidget(verLabel);

    // Default selection
    if (!m_buttons.isEmpty()) {
        m_buttons[0]->setActive(true);
    }
}

void SidebarWidget::setActive(ModloaderType type)
{
    for (auto *b : m_buttons) {
        b->setActive(b->loaderType() == type);
    }
}
