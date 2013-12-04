/*  This file is part of the KDE libraries
 *  Copyright 2013 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License or ( at
 *  your option ) version 3 or, at the discretion of KDE e.V. ( which shall
 *  act as a proxy as in section 14 of the GPLv3 ), any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef KDEPLATFORMFONTDIALOGHELPER_H
#define KDEPLATFORMFONTDIALOGHELPER_H

#include <qpa/qplatformdialoghelper.h>
#include <QDialog>

class KFileWidget;
class QDialogButtonBox;
class KDEPlatformFileDialog : public QDialog
{
Q_OBJECT
public:
    friend class KDEPlatformFileDialogHelper;

    explicit KDEPlatformFileDialog();
    QUrl directory();
    void selectNameFilter(const QString &filter);
    void setDirectory(const QUrl &directory);
    void selectFile(const QUrl &filename);
    QString selectedNameFilter();
    QList<QUrl> selectedFiles();

Q_SIGNALS:
    void fileSelected(const QUrl &file);
    void filesSelected(const QList<QUrl> &files);
    void currentChanged(const QUrl &path);
    void directoryEntered(const QUrl &directory);
    void filterSelected(const QString &filter);

private Q_SLOTS:
    void selectionChanged();

protected:
    KFileWidget* m_fileWidget;
    QDialogButtonBox* m_buttons;
};

class KDEPlatformFileDialogHelper : public QPlatformFileDialogHelper
{
public:
    KDEPlatformFileDialogHelper();
    virtual ~KDEPlatformFileDialogHelper();

    void initializeDialog();

    virtual bool defaultNameFilterDisables() const;
    virtual QUrl directory() const;
    virtual QList<QUrl> selectedFiles() const;
    virtual QString selectedNameFilter() const;
    virtual void selectNameFilter(const QString &filter);
    virtual void selectFile(const QUrl &filename);
    virtual void setFilter();
    virtual void setDirectory(const QUrl &directory);

    virtual void exec();
    virtual void hide();
    virtual bool show(Qt::WindowFlags windowFlags, Qt::WindowModality windowModality, QWindow *parent);

private:
    KDEPlatformFileDialog* m_dialog;
};

#endif // KDEPLATFORMFONTDIALOGHELPER_H
