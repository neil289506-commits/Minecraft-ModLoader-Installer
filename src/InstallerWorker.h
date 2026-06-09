#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>
#include <QFile>
#include "ModloaderInfo.h"

/**
 * Performs the actual installation in a background-friendly way.
 * Emits signals for progress and result.
 *
 * Strategy per loader:
 *  Vanilla  (server)   – download server jar from Mojang manifest
 *  Fabric   (both)     – run Fabric installer jar via Java
 *  Quilt    (both)     – run Quilt installer jar via Java
 *  Forge    (both)     – run Forge installer jar via Java
 *  NeoForge (both)     – run NeoForge installer jar via Java
 *  OptiFine (client)   – download OptiFine jar; optionally patch client
 */
class InstallerWorker : public QObject
{
    Q_OBJECT
public:
    explicit InstallerWorker(QObject *parent = nullptr);
    ~InstallerWorker();

    void install(const InstallConfig &cfg);
    void cancel();

signals:
    void progressMessage(const QString &msg);
    void progressPercent(int percent);   // 0-100, -1 = indeterminate
    void installFinished(bool success, const QString &message);

private slots:
    void onDownloadProgress(qint64 received, qint64 total);
    void onDownloadFinished();
    void onProcessOutput();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    // Download helpers
    void startDownload(const QUrl &url, const QString &destPath,
                       std::function<void()> onSuccess);

    // Loader-specific install steps
    void installVanillaServer();
    void installFabric();
    void installQuilt();
    void installForge();
    void installNeoForge();
    void installOptiFine();

    // Run an installer jar
    void runInstallerJar(const QString &jarPath, const QStringList &args,
                          std::function<void(bool)> onFinished);

    // Resolve Vanilla server URL from cached manifest
    QString resolveVanillaServerUrl() const;

    InstallConfig m_cfg;
    QNetworkAccessManager *m_nam = nullptr;
    QNetworkReply         *m_reply  = nullptr;
    QProcess              *m_process = nullptr;
    QFile                 *m_outFile = nullptr;
    bool m_cancelled = false;

    std::function<void()> m_downloadCallback;
    std::function<void(bool)> m_processCallback;
};
