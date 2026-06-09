#include "InstallerWorker.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QNetworkRequest>
#include <QDebug>

InstallerWorker::InstallerWorker(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{}

InstallerWorker::~InstallerWorker()
{
    cancel();
}

void InstallerWorker::cancel()
{
    m_cancelled = true;
    if (m_reply)  { m_reply->abort(); }
    if (m_process){ m_process->kill(); }
    if (m_outFile && m_outFile->isOpen()) m_outFile->close();
}

void InstallerWorker::install(const InstallConfig &cfg)
{
    m_cfg = cfg;
    m_cancelled = false;

    QDir().mkpath(cfg.installDir);

    switch (cfg.type) {
    case ModloaderType::Vanilla:  installVanillaServer(); break;
    case ModloaderType::Fabric:   installFabric();        break;
    case ModloaderType::Quilt:    installQuilt();         break;
    case ModloaderType::Forge:    installForge();         break;
    case ModloaderType::NeoForge: installNeoForge();      break;
    case ModloaderType::OptiFine: installOptiFine();      break;
    }
}

// ──────────────────────────── Download helper ──────────────────────────────

void InstallerWorker::startDownload(const QUrl &url, const QString &destPath,
                                     std::function<void()> onSuccess)
{
    m_downloadCallback = onSuccess;

    if (m_outFile) { delete m_outFile; }
    m_outFile = new QFile(destPath, this);
    if (!m_outFile->open(QIODevice::WriteOnly)) {
        emit installFinished(false, "Cannot open file for writing: " + destPath);
        return;
    }

    emit progressMessage("Downloading: " + url.toString());

    QNetworkRequest req(url);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
    m_reply = m_nam->get(req);

    connect(m_reply, &QNetworkReply::downloadProgress,
            this, &InstallerWorker::onDownloadProgress);
    connect(m_reply, &QNetworkReply::readyRead, this, [this]() {
        if (m_outFile) m_outFile->write(m_reply->readAll());
    });
    connect(m_reply, &QNetworkReply::finished,
            this, &InstallerWorker::onDownloadFinished);
}

void InstallerWorker::onDownloadProgress(qint64 received, qint64 total)
{
    if (total > 0)
        emit progressPercent(static_cast<int>(received * 100 / total));
    else
        emit progressPercent(-1);
}

void InstallerWorker::onDownloadFinished()
{
    m_outFile->close();

    if (m_cancelled) return;

    if (m_reply->error() != QNetworkReply::NoError) {
        emit installFinished(false, "Download failed: " + m_reply->errorString());
        m_reply->deleteLater();
        m_reply = nullptr;
        return;
    }

    m_reply->deleteLater();
    m_reply = nullptr;

    emit progressPercent(100);
    if (m_downloadCallback) m_downloadCallback();
}

// ─────────────────────────── Process helper ────────────────────────────────

void InstallerWorker::runInstallerJar(const QString &jarPath,
                                        const QStringList &extraArgs,
                                        std::function<void(bool)> onFinished)
{
    m_processCallback = onFinished;

    QString java = m_cfg.javaPath.isEmpty() ? "java" : m_cfg.javaPath;

    QStringList args;
    args << "-jar" << jarPath;
    args << extraArgs;

    emit progressMessage("Running: " + java + " " + args.join(' '));
    emit progressPercent(-1);

    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &InstallerWorker::onProcessOutput);
    connect(m_process, QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
            this, &InstallerWorker::onProcessFinished);

    m_process->start(java, args);
}

void InstallerWorker::onProcessOutput()
{
    QString out = QString::fromUtf8(m_process->readAllStandardOutput()).trimmed();
    if (!out.isEmpty())
        emit progressMessage(out);
}

void InstallerWorker::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    m_process->deleteLater();
    m_process = nullptr;

    bool ok = (exitCode == 0 && status == QProcess::NormalExit);
    if (m_processCallback) m_processCallback(ok);
}

// ─────────────────────── Vanilla (server only) ─────────────────────────────

QString InstallerWorker::resolveVanillaServerUrl() const
{
    // Read locally cached MojangData.json
    QString path = QCoreApplication::applicationDirPath() + "/version/MojangData.json";
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return {};

    QJsonArray versions = QJsonDocument::fromJson(f.readAll())
                              .object()["versions"].toArray();
    for (const QJsonValue &v : versions) {
        QJsonObject o = v.toObject();
        if (o["id"].toString() == m_cfg.version.mcVersion) {
            // Need to fetch the version-specific JSON to get server URL
            // Return the version manifest URL for now; we'll chain downloads
            return o["url"].toString();
        }
    }
    return {};
}

void InstallerWorker::installVanillaServer()
{
    emit progressMessage("Resolving Vanilla server for " + m_cfg.version.mcVersion + "...");

    QString manifestUrl = resolveVanillaServerUrl();
    if (manifestUrl.isEmpty()) {
        emit installFinished(false, "Could not find version in manifest.");
        return;
    }

    QString tmpManifest = m_cfg.installDir + "/version_info.json";

    // Step 1: download version-specific manifest
    startDownload(QUrl(manifestUrl), tmpManifest, [this, tmpManifest]() {
        QFile f(tmpManifest);
        if (!f.open(QIODevice::ReadOnly)) {
            emit installFinished(false, "Cannot read version info JSON.");
            return;
        }
        QJsonObject downloads = QJsonDocument::fromJson(f.readAll())
                                    .object()["downloads"].toObject();
        f.close();
        QFile::remove(tmpManifest);

        QString serverUrl = downloads["server"].toObject()["url"].toString();
        if (serverUrl.isEmpty()) {
            emit installFinished(false, "No server download for this version.");
            return;
        }

        QString dest = m_cfg.installDir + "/minecraft_server." +
                       m_cfg.version.mcVersion + ".jar";

        // Step 2: download actual server jar
        startDownload(QUrl(serverUrl), dest, [this, dest]() {
            emit progressMessage("Server jar saved to: " + dest);
            emit installFinished(true,
                "Vanilla server installed successfully!\n"
                "Run with: java -jar " + QFileInfo(dest).fileName() + " nogui");
        });
    });
}

// ─────────────────────────── Fabric ───────────────────────────────────────

void InstallerWorker::installFabric()
{
    // Download Fabric universal installer
    QString installerUrl = QString(
        "https://maven.fabricmc.net/net/fabricmc/fabric-installer/1.0.1/"
        "fabric-installer-1.0.1.jar");

    QString jarPath = m_cfg.installDir + "/fabric-installer.jar";

    emit progressMessage("Downloading Fabric installer...");
    startDownload(QUrl(installerUrl), jarPath, [this, jarPath]() {
        QStringList args;
        bool doClient = (m_cfg.target == InstallTarget::Client ||
                         m_cfg.target == InstallTarget::Both);
        bool doServer = (m_cfg.target == InstallTarget::Server ||
                         m_cfg.target == InstallTarget::Both);

        auto finish = [this, jarPath](bool ok) {
            QFile::remove(jarPath);
            if (ok)
                emit installFinished(true, "Fabric installed successfully!");
            else
                emit installFinished(false, "Fabric installer reported an error.");
        };

        if (doClient) {
            QStringList cArgs = {
                "client",
                "-dir",      m_cfg.installDir,
                "-mcversion", m_cfg.version.mcVersion,
                "-loader",   m_cfg.version.loaderVersion,
                m_cfg.createProfile ? "" : "-noprofile"
            };
            cArgs.removeAll("");

            if (doServer) {
                // Chain: client → then server
                runInstallerJar(jarPath, cArgs, [this, jarPath, finish](bool ok) {
                    if (!ok) { finish(false); return; }
                    QStringList sArgs = { "server",
                                          "-dir",    m_cfg.installDir,
                                          "-mcversion", m_cfg.version.mcVersion,
                                          "-loader", m_cfg.version.loaderVersion,
                                          "-downloadMinecraft" };
                    runInstallerJar(jarPath, sArgs, finish);
                });
            } else {
                runInstallerJar(jarPath, cArgs, finish);
            }
        } else if (doServer) {
            QStringList sArgs = { "server",
                                   "-dir",    m_cfg.installDir,
                                   "-mcversion", m_cfg.version.mcVersion,
                                   "-loader", m_cfg.version.loaderVersion,
                                   "-downloadMinecraft" };
            runInstallerJar(jarPath, sArgs, finish);
        }
    });
}

// ─────────────────────────── Quilt ────────────────────────────────────────

void InstallerWorker::installQuilt()
{
    QString installerUrl =
        "https://quiltmc.org/api/v1/download-latest-installer/java-universal";

    QString jarPath = m_cfg.installDir + "/quilt-installer.jar";

    emit progressMessage("Downloading Quilt installer...");
    startDownload(QUrl(installerUrl), jarPath, [this, jarPath]() {
        bool doClient = (m_cfg.target != InstallTarget::Server);
        bool doServer = (m_cfg.target != InstallTarget::Client);

        auto finish = [this, jarPath](bool ok) {
            QFile::remove(jarPath);
            if (ok) emit installFinished(true, "Quilt installed successfully!");
            else    emit installFinished(false, "Quilt installer reported an error.");
        };

        if (doClient) {
            QStringList cArgs = { "install", "client",
                                   m_cfg.version.mcVersion,
                                   m_cfg.version.loaderVersion,
                                   "--install-dir=" + m_cfg.installDir };
            if (doServer) {
                runInstallerJar(jarPath, cArgs, [this, jarPath, finish](bool ok) {
                    if (!ok) { finish(false); return; }
                    QStringList sArgs = { "install", "server",
                                           m_cfg.version.mcVersion,
                                           m_cfg.version.loaderVersion,
                                           "--install-dir=" + m_cfg.installDir,
                                           "--download-server" };
                    runInstallerJar(jarPath, sArgs, finish);
                });
            } else {
                runInstallerJar(jarPath, cArgs, finish);
            }
        } else {
            QStringList sArgs = { "install", "server",
                                   m_cfg.version.mcVersion,
                                   m_cfg.version.loaderVersion,
                                   "--install-dir=" + m_cfg.installDir,
                                   "--download-server" };
            runInstallerJar(jarPath, sArgs, finish);
        }
    });
}

// ─────────────────────────── Forge ────────────────────────────────────────

void InstallerWorker::installForge()
{
    // Forge installer: net.minecraftforge:forge:<mc>-<forge>:installer
    QString ver    = m_cfg.version.mcVersion + "-" + m_cfg.version.loaderVersion;
    QString url    = QString("https://maven.minecraftforge.net/net/minecraftforge/"
                             "forge/%1/forge-%1-installer.jar").arg(ver);
    QString jarPath = m_cfg.installDir + "/forge-installer.jar";

    emit progressMessage("Downloading Forge installer for " + ver + "...");
    startDownload(QUrl(url), jarPath, [this, jarPath]() {
        bool doClient = (m_cfg.target != InstallTarget::Server);
        bool doServer = (m_cfg.target != InstallTarget::Client);

        auto finish = [this, jarPath](bool ok) {
            QFile::remove(jarPath);
            if (ok) emit installFinished(true, "Forge installed successfully!");
            else    emit installFinished(false, "Forge installer reported an error.");
        };

        if (doClient) {
            QStringList cArgs = { "--installClient", m_cfg.installDir };
            if (doServer) {
                runInstallerJar(jarPath, cArgs, [this, jarPath, finish](bool ok) {
                    if (!ok) { finish(false); return; }
                    runInstallerJar(jarPath,
                        { "--installServer", m_cfg.installDir }, finish);
                });
            } else {
                runInstallerJar(jarPath, cArgs, finish);
            }
        } else {
            runInstallerJar(jarPath, { "--installServer", m_cfg.installDir }, finish);
        }
    });
}

// ─────────────────────────── NeoForge ─────────────────────────────────────

void InstallerWorker::installNeoForge()
{
    QString ver = m_cfg.version.loaderVersion;
    QString url = QString("https://maven.neoforged.net/releases/net/neoforged/"
                          "neoforge/%1/neoforge-%1-installer.jar").arg(ver);
    QString jarPath = m_cfg.installDir + "/neoforge-installer.jar";

    emit progressMessage("Downloading NeoForge installer " + ver + "...");
    startDownload(QUrl(url), jarPath, [this, jarPath]() {
        bool doClient = (m_cfg.target != InstallTarget::Server);
        bool doServer = (m_cfg.target != InstallTarget::Client);

        auto finish = [this, jarPath](bool ok) {
            QFile::remove(jarPath);
            if (ok) emit installFinished(true, "NeoForge installed successfully!");
            else    emit installFinished(false, "NeoForge installer reported an error.");
        };

        if (doClient) {
            QStringList cArgs = { "--installClient", m_cfg.installDir };
            if (doServer) {
                runInstallerJar(jarPath, cArgs, [this, jarPath, finish](bool ok) {
                    if (!ok) { finish(false); return; }
                    runInstallerJar(jarPath,
                        { "--installServer", m_cfg.installDir }, finish);
                });
            } else {
                runInstallerJar(jarPath, cArgs, finish);
            }
        } else {
            runInstallerJar(jarPath, { "--installServer", m_cfg.installDir }, finish);
        }
    });
}

// ─────────────────────────── OptiFine ─────────────────────────────────────

void InstallerWorker::installOptiFine()
{
    // Read mirror URL from cached OptiFineVersion.json
    QString path = QCoreApplication::applicationDirPath() + "/version/OptiFineVersion.json";
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        emit installFinished(false, "OptiFineVersion.json not found. Run version fetch first.");
        return;
    }

    QJsonArray arr = QJsonDocument::fromJson(f.readAll()).object()["versions"].toArray();
    QString mirrorUrl;

    for (const QJsonValue &v : arr) {
        QJsonObject o = v.toObject();
        if (o["minecraft"].toString() == m_cfg.version.mcVersion &&
            o["optifine"].toString()  == m_cfg.version.loaderVersion)
        {
            mirrorUrl = o["mirror"].toString();
            break;
        }
    }

    if (mirrorUrl.isEmpty()) {
        emit installFinished(false, "OptiFine version not found in cached data.");
        return;
    }

    QString fileName = QString("OptiFine_%1_%2.jar")
                           .arg(m_cfg.version.mcVersion, m_cfg.version.loaderVersion);
    QString dest = m_cfg.installDir + "/" + fileName;

    emit progressMessage("Downloading OptiFine from mirror...");
    startDownload(QUrl(mirrorUrl), dest, [this, dest, fileName]() {
        // OptiFine can self-install when run as jar with no args (GUI)
        // or with --headless --mcDir <dir>
        emit progressMessage("Launching OptiFine installer...");
        runInstallerJar(dest, {}, [this, dest](bool ok) {
            if (ok)
                emit installFinished(true, "OptiFine installed successfully!");
            else {
                // Installer might have non-zero exit but still worked;
                // just place the jar and inform user.
                emit installFinished(true,
                    "OptiFine jar downloaded to install dir.\n"
                    "Double-click the jar to complete installation if needed.");
            }
        });
    });
}
