#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include "SidebarWidget.h"
#include "InstallerPage.h"
#include "VersionFetcher.h"
#include "InstallerWorker.h"
#include "ModloaderInfo.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoaderSelected(ModloaderType type);
    void onInstallRequested(const InstallConfig &cfg);
    void onFetchRequested();
    void onFetchProgress(const QString &msg);
    void onFetchFinished();
    void onFetchError(const QString &err);
    void onVersionsReady(ModloaderType type, const QList<ModloaderVersion> &versions);
    void onInstallProgress(const QString &msg);
    void onInstallPercent(int pct);
    void onInstallFinished(bool success, const QString &msg);

private:
    SidebarWidget  *m_sidebar;
    InstallerPage  *m_page;
    VersionFetcher *m_fetcher;
    InstallerWorker*m_worker;

    class ProgressDialog *m_progressDlg = nullptr;
};
