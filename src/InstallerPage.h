#pragma once
#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>
#include <QCheckBox>
#include <QGroupBox>
#include <QMap>
#include <QList>
#include <QStringList>
#include "ModloaderInfo.h"

class InstallerPage : public QWidget
{
    Q_OBJECT
public:
    explicit InstallerPage(QWidget *parent = nullptr);

    void setLoader(ModloaderType type);

    // Loader versions (mcVersion may be empty for Fabric/Quilt)
    void setVersions(ModloaderType type, const QList<ModloaderVersion> &versions);

    // Minecraft game versions for Fabric/Quilt (independent of loader version)
    void setGameVersions(ModloaderType type, const QStringList &mcVersions);

    InstallConfig buildConfig() const;

signals:
    void installRequested(const InstallConfig &cfg);
    void fetchRequested();

private slots:
    void onInstallClicked();
    void onBrowseClicked();
    void onMcVersionChanged(int index);

private:
    void rebuildVersionCombos();
    void updateTargetOptions();
    void applyLoaderTheme();

    ModloaderType m_currentType = ModloaderType::Fabric;

    // Loader versions per loader (mcVersion empty for Fabric/Quilt)
    QMap<ModloaderType, QList<ModloaderVersion>> m_versionData;

    // Separate game version lists for Fabric & Quilt
    QMap<ModloaderType, QStringList> m_gameVersions;

    // Header
    QLabel       *m_titleLabel;
    QLabel       *m_descLabel;
    QLabel       *m_badgeClient;
    QLabel       *m_badgeServer;

    // Version selectors
    QLabel       *m_mcVersionLabel;
    QComboBox    *m_mcVersionCombo;
    QLabel       *m_loaderVersionLabel;
    QComboBox    *m_loaderVersionCombo;

    // Install target
    QGroupBox    *m_targetGroup;
    QRadioButton *m_radioClient;
    QRadioButton *m_radioServer;
    QRadioButton *m_radioBoth;

    // Options
    QCheckBox    *m_profileCheck;
    QLineEdit    *m_dirEdit;
    QPushButton  *m_browseBtn;
    QLineEdit    *m_javaEdit;

    // Buttons
    QPushButton  *m_fetchBtn;
    QPushButton  *m_installBtn;

    QLabel       *m_statusLabel;
};
