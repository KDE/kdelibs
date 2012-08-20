#ifndef FILEITEM_H
#define FILEITEM_H

#include <QVariantList>
#include "filehelpers.h"
#include "kio/copyjob.h"

class FileItem
{
public:
    FileItem();
    FileItem(int id, KIO::CopyInfo file, int actions = 0, int progress = 0);
    FileItem(int id, const QString &url, int size, int actions = 0, int progress = 0);
    bool operator==(const FileItem &other) const;

    bool isNull() const;
    QVariant get(int role) const;
    static const QHash<int, QByteArray>& header();

    QVariant id;
    QVariant url;
    QVariant size;
    QVariant actions;
    QVariant progress;
};

#endif // FILEITEM_H
