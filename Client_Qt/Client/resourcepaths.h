#ifndef RESOURCEPATHS_H
#define RESOURCEPATHS_H

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QString>
#include <QStringList>

namespace ResourcePaths {

inline QStringList buildAncestorCandidates(const QString &baseDir, const QString &relativePath)
{
    QStringList candidates;
    QDir dir(baseDir);
    for (int depth = 0; depth < 6; ++depth) {
        candidates << dir.filePath(relativePath);
        if (!dir.cdUp()) {
            break;
        }
    }
    return candidates;
}

inline QString findPhoto(const QString &fileName)
{
    const QString appDir = QCoreApplication::applicationDirPath();
    QStringList candidates = {
        appDir + "/photo/" + fileName,
        appDir + "/../photo/" + fileName,
        appDir + "/../../Client_Qt/photo/" + fileName,
        appDir + "/../../Client_Qt/Client/photo/" + fileName,
        appDir + "/../Client/photo/" + fileName,
        QDir::currentPath() + "/photo/" + fileName,
        QDir::currentPath() + "/Client_Qt/photo/" + fileName,
        QDir::currentPath() + "/Client_Qt/Client/photo/" + fileName
    };
    candidates.append(buildAncestorCandidates(appDir, "Client_Qt/photo/" + fileName));
    candidates.append(buildAncestorCandidates(QDir::currentPath(), "Client_Qt/photo/" + fileName));
    candidates.removeDuplicates();

    for (const QString &candidate : candidates) {
        QFileInfo info(QDir::cleanPath(candidate));
        if (info.exists() && info.isFile()) {
            qDebug() << "[ResourcePaths] resolved" << fileName << "->" << info.absoluteFilePath();
            return info.absoluteFilePath();
        }
    }

    qWarning() << "[ResourcePaths] failed to resolve" << fileName
               << "appDir=" << appDir
               << "cwd=" << QDir::currentPath();
    return QString();
}

}

#endif // RESOURCEPATHS_H
