#include "MainWindow.h"
#include "NoguiDialog.h"
#include <QHBoxLayout>
#include <QFrame>
#include <QMessageBox>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_fetcher(new VersionFetcher(this))
    , m_worker(new InstallerWorker(this))
{
    setWindowTitle("Minecraft Modloader Installer");
    setMinimumSize(900, 640);
    resize(1060, 700);

    setStyleSheet(R"(
        QMainWindow { background: #0D1117; }
        QStatusBar  {
            background: #0A0E14; color: #4A5568;
            font-size: 11px; font-family: monospace;
            border-top: 1px solid #1E2D3D;
        }
    )");

    auto *central = new QWidget(this);
    central->setStyleSheet("background:#0D1117;");
    setCentralWidget(central);

    auto *h = new QHBoxLayout(central);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(0);

    m_sidebar = new SidebarWidget(central);
    m_page    = new InstallerPage(central);

    h->addWidget(m_sidebar);

    auto *sep = new QFrame(central);
    sep->setFrameShape(QFrame::VLine);
    sep->setStyleSheet("color:#1E2D3D;");
    h->addWidget(sep);

    h->addWidget(m_page, 1);

    statusBar()->showMessage("Ready  –  click 'Fetch Versions' to load available versions");

    // ── Connections ──────────────────────────────────────────────────────
    connect(m_sidebar, &SidebarWidget::loaderSelected,
            this, &MainWindow::onLoaderSelected);
    connect(m_page, &InstallerPage::installRequested,
            this, &MainWindow::onInstallRequested);
    connect(m_page, &InstallerPage::fetchRequested,
            this, &MainWindow::onFetchRequested);

    connect(m_fetcher, &VersionFetcher::fetchProgress,
            this, &MainWindow::onFetchProgress);
    connect(m_fetcher, &VersionFetcher::fetchFinished,
            this, &MainWindow::onFetchFinished);
    connect(m_fetcher, &VersionFetcher::fetchError,
            this, &MainWindow::onFetchError);
    connect(m_fetcher, &VersionFetcher::versionsReady,
            this, &MainWindow::onVersionsReady);
    connect(m_fetcher, &VersionFetcher::gameVersionsReady,
            this, &MainWindow::onGameVersionsReady);

    connect(m_worker, &InstallerWorker::progressMessage,
            this, &MainWindow::onInstallProgress);
    connect(m_worker, &InstallerWorker::progressPercent,
            this, &MainWindow::onInstallPercent);
    connect(m_worker, &InstallerWorker::progressStage,
            this, &MainWindow::onInstallStage);
    connect(m_worker, &InstallerWorker::installFinished,
            this, &MainWindow::onInstallFinished);

    m_page->setLoader(ModloaderType::Vanilla);
}

MainWindow::~MainWindow() {}

void MainWindow::onLoaderSelected(ModloaderType type)
{
    m_page->setLoader(type);
}

void MainWindow::onFetchRequested()
{
    if (m_fetcher->isFetching()) return;
    statusBar()->showMessage("Fetching version data…");
    m_fetcher->fetchAll();
}

void MainWindow::onFetchProgress(const QString &msg)
{
    statusBar()->showMessage(msg);
}

void MainWindow::onFetchFinished()
{
    statusBar()->showMessage("All version data fetched.");
}

void MainWindow::onFetchError(const QString &err)
{
    statusBar()->showMessage("Fetch error: " + err);
    QMessageBox::critical(this, "Version Fetch Error", err);
}

void MainWindow::onVersionsReady(ModloaderType type, const QList<ModloaderVersion> &versions)
{
    m_page->setVersions(type, versions);
}

void MainWindow::onGameVersionsReady(ModloaderType type, const QStringList &mcVersions)
{
    m_page->setGameVersions(type, mcVersions);
}

void MainWindow::onInstallRequested(const InstallConfig &cfg)
{
    // Ask nogui only for server installs
    bool isServer = (cfg.target == InstallTarget::Server ||
                     cfg.target == InstallTarget::Both);

    InstallConfig finalCfg = cfg;

    if (isServer) {
        if (!m_noguiDecided) {
            NoguiDialog dlg(modloaderName(cfg.type), this);
            dlg.exec();
            m_cachedNogui  = dlg.useNogui();
            m_noguiDecided = dlg.rememberIt();
        }
        finalCfg.useNogui = m_cachedNogui;
    }

    if (m_progressDlg) {
        m_progressDlg->deleteLater();
        m_progressDlg = nullptr;
    }

    m_progressDlg = new ProgressDialog(
        "Installing " + modloaderName(cfg.type) + "…", this);

    connect(m_progressDlg, &ProgressDialog::cancelRequested, this, [this]() {
        m_worker->cancel();
        if (m_progressDlg)
            m_progressDlg->appendLog("[User cancelled installation]");
    });

    m_progressDlg->show();
    m_worker->install(finalCfg);
}

void MainWindow::onInstallProgress(const QString &msg)
{
    if (m_progressDlg) m_progressDlg->appendLog(msg);
    statusBar()->showMessage(msg.length() > 120 ? msg.left(117) + "..." : msg);
}

void MainWindow::onInstallPercent(int pct)
{
    if (m_progressDlg) m_progressDlg->setPercent(pct);
}

void MainWindow::onInstallStage(int current, int total)
{
    if (m_progressDlg) m_progressDlg->setStage(current, total);
}

void MainWindow::onInstallFinished(bool success, const QString &msg)
{
    if (m_progressDlg) m_progressDlg->setFinished(success, msg);
    statusBar()->showMessage(success ? "Installation complete." : "Installation failed.");
    // Reset nogui cache if not remembered
    if (!m_noguiDecided) {
        m_cachedNogui = true;
    }
}
