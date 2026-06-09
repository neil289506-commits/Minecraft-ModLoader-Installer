#include "InstallerPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QFrame>
#include <QScrollArea>

static QString badgeStyle(const QString &color) {
    return QString(R"(
        background: %1;
        color: white;
        border-radius: 3px;
        padding: 2px 8px;
        font-size: 10px;
        font-family: monospace;
        font-weight: 700;
        letter-spacing: 1px;
    )").arg(color);
}

static QString labelStyle() {
    return "color: #718096; font-size: 11px; font-family: monospace; "
           "font-weight: 600; letter-spacing: 1px;";
}

static QString comboStyle(const QString &accent) {
    return QString(R"(
        QComboBox {
            background: #161B22;
            color: #E2E8F0;
            border: 1px solid #2D3748;
            border-radius: 6px;
            padding: 8px 12px;
            font-size: 13px;
            font-family: 'JetBrains Mono', monospace;
            min-height: 20px;
        }
        QComboBox:hover  { border-color: %1; }
        QComboBox:focus  { border-color: %1; }
        QComboBox::drop-down { border: none; width: 24px; }
        QComboBox QAbstractItemView {
            background: #1A2233;
            color: #E2E8F0;
            border: 1px solid #2D3748;
            selection-background-color: %1;
            selection-color: white;
            outline: none;
        }
    )").arg(accent);
}

static QString lineEditStyle(const QString &accent) {
    return QString(R"(
        QLineEdit {
            background: #161B22;
            color: #E2E8F0;
            border: 1px solid #2D3748;
            border-radius: 6px;
            padding: 8px 12px;
            font-size: 12px;
            font-family: monospace;
            min-height: 20px;
        }
        QLineEdit:hover { border-color: %1; }
        QLineEdit:focus { border-color: %1; }
    )").arg(accent);
}

static QString btnPrimary(const QString &color) {
    return QString(R"(
        QPushButton {
            background: %1;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px 32px;
            font-size: 14px;
            font-family: 'JetBrains Mono', monospace;
            font-weight: 700;
            letter-spacing: 1px;
            min-height: 24px;
        }
        QPushButton:hover   { background: %2; }
        QPushButton:pressed { background: %3; }
        QPushButton:disabled { background: #2D3748; color: #4A5568; }
    )").arg(color)
       .arg(QColor(color).lighter(115).name())
       .arg(QColor(color).darker(115).name());
}

static QString radioStyle(const QString &accent) {
    return QString(R"(
        QRadioButton {
            color: #CBD5E0;
            font-size: 13px;
            font-family: 'JetBrains Mono', monospace;
            spacing: 8px;
        }
        QRadioButton::indicator {
            width: 16px; height: 16px;
            border-radius: 8px;
            border: 2px solid #4A5568;
            background: transparent;
        }
        QRadioButton::indicator:checked {
            border-color: %1;
            background: %1;
        }
        QRadioButton:hover { color: white; }
    )").arg(accent);
}

// ──────────────────────────────────────────────────────────────────────────

InstallerPage::InstallerPage(QWidget *parent) : QWidget(parent)
{
    setStyleSheet("background: #0F1419;");

    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("background: transparent;");

    auto *content = new QWidget;
    content->setStyleSheet("background: transparent;");

    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0,0,0,0);
    outerLayout->addWidget(scroll);
    scroll->setWidget(content);

    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(40, 36, 40, 36);
    layout->setSpacing(24);

    // ── Header ────────────────────────────────────────────
    auto *headerWidget = new QWidget;
    headerWidget->setStyleSheet(R"(
        background: #161B22;
        border-radius: 12px;
        border: 1px solid #1E2D3D;
    )");
    auto *headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setContentsMargins(28, 24, 28, 24);
    headerLayout->setSpacing(8);

    m_titleLabel = new QLabel("Vanilla", headerWidget);
    m_titleLabel->setStyleSheet(R"(
        color: white;
        font-size: 26px;
        font-family: 'JetBrains Mono', monospace;
        font-weight: 700;
        letter-spacing: 2px;
        background: transparent;
        border: none;
    )");

    m_descLabel = new QLabel("", headerWidget);
    m_descLabel->setStyleSheet(R"(
        color: #718096;
        font-size: 13px;
        font-family: sans-serif;
        background: transparent;
        border: none;
    )");
    m_descLabel->setWordWrap(true);

    auto *badgeRow = new QHBoxLayout;
    badgeRow->setSpacing(8);
    m_badgeClient = new QLabel("CLIENT", headerWidget);
    m_badgeServer = new QLabel("SERVER", headerWidget);
    badgeRow->addWidget(m_badgeClient);
    badgeRow->addWidget(m_badgeServer);
    badgeRow->addStretch();

    headerLayout->addWidget(m_titleLabel);
    headerLayout->addWidget(m_descLabel);
    headerLayout->addLayout(badgeRow);
    layout->addWidget(headerWidget);

    // ── Version Selection ──────────────────────────────────
    auto *verGroup = new QGroupBox("Version");
    verGroup->setStyleSheet(R"(
        QGroupBox {
            color: #718096;
            font-size: 11px;
            font-family: monospace;
            font-weight: 700;
            letter-spacing: 1px;
            border: 1px solid #1E2D3D;
            border-radius: 10px;
            margin-top: 8px;
            background: #161B22;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 16px;
            padding: 0 6px;
        }
    )");

    auto *verLayout = new QGridLayout(verGroup);
    verLayout->setContentsMargins(20, 20, 20, 20);
    verLayout->setHorizontalSpacing(20);
    verLayout->setVerticalSpacing(12);

    m_mcVersionLabel = new QLabel("MINECRAFT VERSION", verGroup);
    m_mcVersionLabel->setStyleSheet(labelStyle());
    m_mcVersionCombo = new QComboBox(verGroup);
    m_mcVersionCombo->addItem("– fetch versions first –");

    m_loaderVersionLabel = new QLabel("LOADER VERSION", verGroup);
    m_loaderVersionLabel->setStyleSheet(labelStyle());
    m_loaderVersionCombo = new QComboBox(verGroup);
    m_loaderVersionCombo->addItem("– fetch versions first –");

    verLayout->addWidget(m_mcVersionLabel,     0, 0);
    verLayout->addWidget(m_mcVersionCombo,     1, 0);
    verLayout->addWidget(m_loaderVersionLabel, 0, 1);
    verLayout->addWidget(m_loaderVersionCombo, 1, 1);

    layout->addWidget(verGroup);

    // ── Install Target ─────────────────────────────────────
    m_targetGroup = new QGroupBox("Install Target");
    m_targetGroup->setStyleSheet(verGroup->styleSheet());

    auto *targetLayout = new QHBoxLayout(m_targetGroup);
    targetLayout->setContentsMargins(20, 20, 20, 20);
    targetLayout->setSpacing(32);

    m_radioClient = new QRadioButton("Client", m_targetGroup);
    m_radioServer = new QRadioButton("Server", m_targetGroup);
    m_radioBoth   = new QRadioButton("Both",   m_targetGroup);
    m_radioClient->setChecked(true);

    targetLayout->addWidget(m_radioClient);
    targetLayout->addWidget(m_radioServer);
    targetLayout->addWidget(m_radioBoth);
    targetLayout->addStretch();
    layout->addWidget(m_targetGroup);

    // ── Options ────────────────────────────────────────────
    auto *optGroup = new QGroupBox("Options");
    optGroup->setStyleSheet(verGroup->styleSheet());
    auto *optLayout = new QGridLayout(optGroup);
    optLayout->setContentsMargins(20, 20, 20, 20);
    optLayout->setSpacing(12);

    auto *dirLabel = new QLabel("INSTALL DIRECTORY", optGroup);
    dirLabel->setStyleSheet(labelStyle());

    m_dirEdit = new QLineEdit(optGroup);
    m_dirEdit->setPlaceholderText("Select install directory...");
    QString defaultDir;
#ifdef Q_OS_WIN
    defaultDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                 + "/.minecraft";
#elif defined(Q_OS_MAC)
    defaultDir = QDir::homePath() + "/Library/Application Support/minecraft";
#else
    defaultDir = QDir::homePath() + "/.minecraft";
#endif
    m_dirEdit->setText(defaultDir);

    m_browseBtn = new QPushButton("Browse", optGroup);
    m_browseBtn->setFixedWidth(90);
    m_browseBtn->setStyleSheet(R"(
        QPushButton {
            background: #1E2D3D;
            color: #CBD5E0;
            border: 1px solid #2D3748;
            border-radius: 6px;
            padding: 8px 16px;
            font-size: 12px;
            font-family: monospace;
        }
        QPushButton:hover { background: #263548; color: white; }
    )");

    auto *javaLabel = new QLabel("JAVA EXECUTABLE  (blank = system java)", optGroup);
    javaLabel->setStyleSheet(labelStyle());
    m_javaEdit = new QLineEdit(optGroup);
    m_javaEdit->setPlaceholderText("java  /usr/bin/java  C:\\...\\java.exe");

    m_profileCheck = new QCheckBox("Create Minecraft launcher profile (client)", optGroup);
    m_profileCheck->setStyleSheet("color: #CBD5E0; font-size: 13px; font-family: monospace;");
    m_profileCheck->setChecked(true);

    auto *dirRow = new QHBoxLayout;
    dirRow->setSpacing(8);
    dirRow->addWidget(m_dirEdit);
    dirRow->addWidget(m_browseBtn);

    optLayout->addWidget(dirLabel,       0, 0);
    optLayout->addLayout(dirRow,         1, 0);
    optLayout->addWidget(javaLabel,      2, 0);
    optLayout->addWidget(m_javaEdit,     3, 0);
    optLayout->addWidget(m_profileCheck, 4, 0);

    layout->addWidget(optGroup);

    // ── Buttons ────────────────────────────────────────────
    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(12);

    m_fetchBtn = new QPushButton("↻  Fetch Versions", content);
    m_fetchBtn->setStyleSheet(R"(
        QPushButton {
            background: #1E2D3D;
            color: #63B3ED;
            border: 1px solid #2C5282;
            border-radius: 8px;
            padding: 12px 24px;
            font-size: 13px;
            font-family: 'JetBrains Mono', monospace;
            font-weight: 700;
            letter-spacing: 1px;
        }
        QPushButton:hover { background: #2C3E50; color: #90CDF4; }
        QPushButton:disabled { background: #1A2233; color: #2D3748; border-color: #1E2D3D; }
    )");

    m_installBtn = new QPushButton("⬇  Install", content);

    btnRow->addWidget(m_fetchBtn);
    btnRow->addStretch();
    btnRow->addWidget(m_installBtn);

    layout->addLayout(btnRow);

    // ── Status ──────────────────────────────────────────────
    m_statusLabel = new QLabel("", content);
    m_statusLabel->setStyleSheet("color: #718096; font-size: 12px; font-family: monospace;");
    m_statusLabel->setWordWrap(true);
    layout->addWidget(m_statusLabel);

    layout->addStretch();

    // ── Connections ─────────────────────────────────────────
    connect(m_browseBtn,  &QPushButton::clicked, this, &InstallerPage::onBrowseClicked);
    connect(m_installBtn, &QPushButton::clicked, this, &InstallerPage::onInstallClicked);
    connect(m_fetchBtn,   &QPushButton::clicked, this, &InstallerPage::fetchRequested);
    connect(m_mcVersionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &InstallerPage::onMcVersionChanged);

    // Initial theme
    setLoader(ModloaderType::Vanilla);
}

void InstallerPage::setLoader(ModloaderType type)
{
    m_currentType = type;
    applyLoaderTheme();
    updateTargetOptions();
    rebuildLoaderVersions();
}

void InstallerPage::setVersions(ModloaderType type, const QList<ModloaderVersion> &versions)
{
    m_versionData[type] = versions;
    if (type == m_currentType)
        rebuildLoaderVersions();

    m_statusLabel->setText(QString("✓ %1 versions loaded (%2 entries)")
                               .arg(modloaderName(type))
                               .arg(versions.size()));
}

void InstallerPage::applyLoaderTheme()
{
    auto cap = capabilityOf(m_currentType);
    QString accent = cap.color;

    m_titleLabel->setText(modloaderName(m_currentType));
    m_descLabel->setText(cap.description);

    // Badges
    QString clientStyle = cap.supportsClient ?
        badgeStyle(accent) : "background:#1A2233;color:#4A5568;border-radius:3px;"
                              "padding:2px 8px;font-size:10px;font-family:monospace;";
    QString serverStyle = cap.supportsServer ?
        badgeStyle(accent) : "background:#1A2233;color:#4A5568;border-radius:3px;"
                              "padding:2px 8px;font-size:10px;font-family:monospace;";
    m_badgeClient->setStyleSheet(clientStyle);
    m_badgeServer->setStyleSheet(serverStyle);

    m_mcVersionCombo->setStyleSheet(comboStyle(accent));
    m_loaderVersionCombo->setStyleSheet(comboStyle(accent));
    m_dirEdit->setStyleSheet(lineEditStyle(accent));
    m_javaEdit->setStyleSheet(lineEditStyle(accent));
    m_installBtn->setStyleSheet(btnPrimary(accent));

    QString rs = radioStyle(accent);
    m_radioClient->setStyleSheet(rs);
    m_radioServer->setStyleSheet(rs);
    m_radioBoth->setStyleSheet(rs);
}

void InstallerPage::updateTargetOptions()
{
    auto cap = capabilityOf(m_currentType);
    m_radioClient->setEnabled(cap.supportsClient);
    m_radioServer->setEnabled(cap.supportsServer);
    m_radioBoth->setEnabled(cap.supportsClient && cap.supportsServer);

    // Auto-select sensible default
    if (!cap.supportsClient && cap.supportsServer)
        m_radioServer->setChecked(true);
    else if (cap.supportsClient && !cap.supportsServer)
        m_radioClient->setChecked(true);
    else
        m_radioClient->setChecked(true);

    // Profile checkbox only relevant for client
    m_profileCheck->setVisible(cap.supportsClient);
}

void InstallerPage::rebuildLoaderVersions()
{
    const QList<ModloaderVersion> &vers = m_versionData.value(m_currentType);

    // For loaders with both mc-version and loader-version dimensions,
    // mc version combo lists unique MC versions; loader combo filters.
    // For loaders where mcVersion is empty (Fabric, Quilt), mc combo hidden.

    bool hasMcDim = (m_currentType == ModloaderType::Vanilla ||
                     m_currentType == ModloaderType::Forge   ||
                     m_currentType == ModloaderType::NeoForge||
                     m_currentType == ModloaderType::OptiFine);

    m_mcVersionLabel->setVisible(hasMcDim);
    m_mcVersionCombo->setVisible(hasMcDim);

    bool isVanilla = (m_currentType == ModloaderType::Vanilla);
    m_loaderVersionLabel->setVisible(!isVanilla);
    m_loaderVersionCombo->setVisible(!isVanilla);

    m_mcVersionCombo->blockSignals(true);
    m_mcVersionCombo->clear();
    m_loaderVersionCombo->clear();

    if (vers.isEmpty()) {
        m_mcVersionCombo->addItem("– fetch versions first –");
        m_loaderVersionCombo->addItem("– fetch versions first –");
        m_mcVersionCombo->blockSignals(false);
        return;
    }

    if (hasMcDim) {
        QStringList mcVersions;
        for (const auto &v : vers) {
            if (!mcVersions.contains(v.mcVersion))
                mcVersions << v.mcVersion;
        }
        m_mcVersionCombo->addItems(mcVersions);
    }

    if (!hasMcDim) {
        // Fabric/Quilt: just list loader versions
        for (const auto &v : vers) {
            m_loaderVersionCombo->addItem(
                v.displayName + (v.isStable ? "" : "  [unstable]"),
                v.loaderVersion);
        }
    } else {
        onMcVersionChanged(0);
    }

    m_mcVersionCombo->blockSignals(false);
}

void InstallerPage::onMcVersionChanged(int index)
{
    if (index < 0) return;
    QString mc = m_mcVersionCombo->currentText();
    const QList<ModloaderVersion> &vers = m_versionData.value(m_currentType);

    m_loaderVersionCombo->clear();

    for (const auto &v : vers) {
        if (v.mcVersion == mc) {
            m_loaderVersionCombo->addItem(
                v.displayName + (v.isStable ? " ★" : ""),
                v.loaderVersion);
        }
    }

    // For Vanilla: mc version IS the version
    if (m_currentType == ModloaderType::Vanilla && m_loaderVersionCombo->count() == 0) {
        // Pick the MC version entry directly
        for (const auto &v : vers) {
            if (v.mcVersion == mc)
                m_loaderVersionCombo->addItem(v.displayName);
        }
    }
}

void InstallerPage::onBrowseClicked()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, "Select Install Directory",
        m_dirEdit->text(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty())
        m_dirEdit->setText(dir);
}

void InstallerPage::onInstallClicked()
{
    InstallConfig cfg = buildConfig();

    if (cfg.installDir.isEmpty()) {
        QMessageBox::warning(this, "Missing Directory",
                             "Please select an install directory.");
        return;
    }
    if (m_versionData.value(m_currentType).isEmpty()) {
        QMessageBox::warning(this, "No Versions",
                             "Please fetch versions first.");
        return;
    }

    emit installRequested(cfg);
}

InstallConfig InstallerPage::buildConfig() const
{
    InstallConfig cfg;
    cfg.type       = m_currentType;
    cfg.installDir = m_dirEdit->text();
    cfg.javaPath   = m_javaEdit->text().trimmed();
    cfg.createProfile = m_profileCheck->isChecked();

    // Target
    if (m_radioServer->isChecked())
        cfg.target = InstallTarget::Server;
    else if (m_radioBoth->isChecked())
        cfg.target = InstallTarget::Both;
    else
        cfg.target = InstallTarget::Client;

    // Version
    const QList<ModloaderVersion> &vers = m_versionData.value(m_currentType);
    if (!vers.isEmpty()) {
        QString mc     = m_mcVersionCombo->currentText();
        QString loader = m_loaderVersionCombo->currentData().toString();
        if (loader.isEmpty()) loader = m_loaderVersionCombo->currentText();

        for (const auto &v : vers) {
            bool mcMatch  = v.mcVersion.isEmpty() || v.mcVersion == mc;
            bool ldrMatch = v.loaderVersion.isEmpty() || v.loaderVersion == loader ||
                            v.mcVersion == mc; // Vanilla
            if (mcMatch && ldrMatch) {
                cfg.version = v;
                break;
            }
        }
        // Fallback
        if (cfg.version.mcVersion.isEmpty() && cfg.version.loaderVersion.isEmpty()) {
            cfg.version.mcVersion    = mc;
            cfg.version.loaderVersion = loader;
        }
    }

    return cfg;
}
