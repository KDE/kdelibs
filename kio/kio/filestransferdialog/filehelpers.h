/*  This file is part of the KDE project
    Copyright (C) 2012 Cyril Oblikov <munknex@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2+ as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef FILEHELPERS_H
#define FILEHELPERS_H

#include <QObject>

class FileHelper : public QObject {
    Q_OBJECT
    Q_ENUMS(FileActions)
    Q_ENUMS(FileRoles)
    Q_ENUMS(Models)
    Q_ENUMS(Unit)
public:
    FileHelper(QObject *parent = 0);
//    enum FileActions {
//        None = 0,
//        Retry = 1,
//        Skip = 2,
//        OpenDir = 4
//    };
    enum FileActions {
        Finished,
        Unfinished,
        Skipped,
        Unreadable,
        Disappeared
    };
    enum FileRoles {
        FileID = Qt::UserRole + 1,
        URL = Qt::UserRole + 2,
        Actions = Qt::UserRole + 3,
        Size = Qt::UserRole + 4,
        Progress = Qt::UserRole + 5
    };
//    enum Models {
//        Normal,
//        Skipped,
//        Errors
//    };
    enum Unit { Bytes, Files, Directories };
};

//class Amounts : public QObject {
//    Q_OBJECT
//    Q_PROPERTY(QString m_files READ files)
//    Q_PROPERTY(QString m_bytes READ bytes)
//    Q_PROPERTY(QString m_ratio READ ratio)
//public:
//    Amounts(qulonglong files, qulonglong bytes, float ratio)
//    {
//        m_files = KGlobal::locale()->formatByteSize(files);
//        m_bytes = KGlobal::locale()->formatByteSize(bytes);
//        m_ratio = ratio;
//    }
//    QString files() { return m_files; }
//    QString bytes() { return m_bytes; }
//    float ratio() { return m_ratio; }
//private:
//    QString m_files;
//    QString m_bytes;
//    float m_ratio;
//};




#endif // FILEHELPERS_H
