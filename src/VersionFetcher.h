#pragma once
#include <QObject>
#include <QProcess>
#include <QDir>
#include <QMap>
#include "ModloaderInfo.h"

/**
 * Runs requestVer.py in a subprocess and signals progress.
 * After completion, parses the downloaded JSON/XML files
 * and emits versionsFetched() per loader.
 */
class VersionFetcher : public QObject
{
    Q_OBJECT
public:
    explicit VersionFetcher(QObject *parent = nullptr);

    // Launch python script to download version data
    void fetchAll();

    // Parse already-downloaded files (no network)
    void parseLocal(const QString &versionDir);

    bool isFetching() const { return m_fetching; }

signals:
    void fetchProgress(const QString &message);
    void fetchFinished();          // python exited OK
    void fetchError(const QString &error);
    void versionsReady(ModloaderType type, const QList<ModloaderVersion> &versions);

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
    bool m_fetching = false;
    QString m_versionDir;
};
