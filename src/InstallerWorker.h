#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>
#include <QFile>
#include <QUrl>
#include <QStringList>
#include <functional>
#include "ModloaderInfo.h"

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
    void progressPercent(int percent);          // 0-100, -1 = indeterminate (stage bar)
    void progressStage(int current, int total); // overall bar
    void installFinished(bool success, const QString &message);

private slots:
    void onDownloadProgress(qint64 received, qint64 total);
    void onDownloadFinished();
    void onProcessOutput();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    void startDownload(const QUrl &url, const QString &destPath,
                       std::function<void()> onSuccess);

    void installVanillaServer();
    void installFabric();
    void installQuilt();
    void installForge();
    void installNeoForge();
    void installOptiFine();

    void runInstallerJar(const QString &jarPath, const QStringList &args,
                          std::function<void(bool)> onFinished);

    // Create launcher_profiles.json so Forge/NeoForge don't complain
    void ensureLauncherProfile(const QString &dir);
    // Create start_server.bat/.sh with optional nogui
    void createStartScript(const QString &dir, const QString &jarName, bool nogui);

    QString resolveVanillaServerUrl() const;

    void hideProcessWindow(QProcess *p);

    InstallConfig m_cfg;
    QNetworkAccessManager *m_nam     = nullptr;
    QNetworkReply         *m_reply   = nullptr;
    QProcess              *m_process = nullptr;
    QFile                 *m_outFile = nullptr;
    bool                   m_cancelled = false;

    std::function<void()>     m_downloadCallback;
    std::function<void(bool)> m_processCallback;
};
