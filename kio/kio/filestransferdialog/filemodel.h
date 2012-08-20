#ifndef FILEMODEL_H
#define FILEMODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include "filehelpers.h"
#include "fileitem.h"


class FileModel : public QAbstractListModel
{
    Q_OBJECT
    Q_ENUMS(FileActions)

    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit FileModel(QObject *parent = 0);
    int count() const { return rowCount(); }

    void enqueue(const FileItem &file);
    FileItem dequeue();
    bool contains(int id) const;
    FileItem peekAtFile(int id) const;
    FileItem takeFile(int id);
    void replaceFile(int id, const FileItem &newFile);
    void updateFileProgress(int id, qreal progress, FileHelper::FileActions actions);
    void setFinished();
    void removeFinished();
    QList<int> allIDs() const;
    QList<int> takeAll();

    // overloaded
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

signals:
    void countChanged();

private:
    int findPosById(int id) const;
    QList<FileItem> m_files;
};

#endif // FILEMODEL_H
