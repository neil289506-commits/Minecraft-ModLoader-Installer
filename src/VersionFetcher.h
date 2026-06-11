#pragma once
#include <QObject>
#include <QProcess>
#include <QDir>
#include <QStringList>
#include "ModloaderInfo.h"

class VersionFetcher : public QObject
{
    Q_OBJECT
public:
    explicit VersionFetcher(QObject *parent = nullptr);
    void fetchAll();
    void parseLocal(const QString &versionDir);
    bool isFetching() const { return m_fetching; }

signals:
    void fetchProgress(const QString &message);
    void fetchFinished();
    void fetchError(const QString &error);
    // Loader versions (mcVersion may be empty for Fabric/Quilt)
    void versionsReady(ModloaderType type, const QList<ModloaderVersion> &versions);
    // Minecraft game versions supported by a loader (Fabric/Quilt)
    void gameVersionsReady(ModloaderType type, const QStringList &mcVersions);

private slots:
    void onProcessOutput();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    void generatePythonScript(const QString &scriptPath);
    void parseVanilla(const QString &dir);
    void parseFabric(const QString &dir);
    void parseQuilt(const QString &dir);
    void parseForge(const QString &dir);
    void parseNeoForge(const QString &dir);
    void parseOptiFine(const QString &dir);

    QProcess *m_process = nullptr;
    bool      m_fetching = false;
    QString   m_versionDir;
};
