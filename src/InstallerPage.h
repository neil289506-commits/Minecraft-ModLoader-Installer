#pragma once
#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>
#include <QCheckBox>
#include <QGroupBox>
#include <QList>
#include "ModloaderInfo.h"

/**
 * The main right-hand content area.
 * Shows version selectors, install target options, and install button.
 */
class InstallerPage : public QWidget
{
    Q_OBJECT
public:
    explicit InstallerPage(QWidget *parent = nullptr);

    void setLoader(ModloaderType type);
    void setVersions(ModloaderType type, const QList<ModloaderVersion> &versions);

    InstallConfig buildConfig() const;

signals:
    void installRequested(const InstallConfig &cfg);
    void fetchRequested();

private slots:
    void onInstallClicked();
    void onBrowseClicked();
    void onMcVersionChanged(int index);

private:
    void rebuildLoaderVersions();
    void updateTargetOptions();
    void applyLoaderTheme();

    ModloaderType m_currentType = ModloaderType::Fabric;

    // Version data per loader
    QMap<ModloaderType, QList<ModloaderVersion>> m_versionData;

    // Header
    QLabel       *m_iconLabel;
    QLabel       *m_titleLabel;
    QLabel       *m_descLabel;
    QLabel       *m_badgeClient;
    QLabel       *m_badgeServer;

    // Version selectors
    QComboBox    *m_mcVersionCombo;    // Minecraft version
    QLabel       *m_mcVersionLabel;
    QComboBox    *m_loaderVersionCombo; // Loader version
    QLabel       *m_loaderVersionLabel;

    // Install target
    QGroupBox    *m_targetGroup;
    QRadioButton *m_radioClient;
    QRadioButton *m_radioServer;
    QRadioButton *m_radioBoth;

    // Options
    QCheckBox    *m_profileCheck;   // Create launcher profile (client)
    QLineEdit    *m_dirEdit;
    QPushButton  *m_browseBtn;

    // Java path
    QLineEdit    *m_javaEdit;

    // Buttons
    QPushButton  *m_fetchBtn;
    QPushButton  *m_installBtn;

    // Status
    QLabel       *m_statusLabel;
};
