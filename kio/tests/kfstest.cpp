/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <QtCore/QDir>
#include <QApplication>
#include <QLayout>
#include <QtCore/QMutableStringListIterator>
#include <QWidget>

#include <kfiledialog.h>
#include <kmessagebox.h>
#if 0 // SPLIT-TODO
#include <kconfig.h>
#include <kurlbar.h>
#include <kdiroperator.h>
#endif
#include <kfile.h>
#include <kdebug.h>
#include <kicondialog.h>

#include "kfdtest.h"
#include <kconfiggroup.h>

int main(int argc, char **argv)
{
    //KCmdLineOptions options;
    //options.add("+[cmd]");
    //options.add("+[url]");

    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);

    QString name1;
    QStringList names;

    QString argv1;
    QUrl startDir;
    if (argc > 1)
        argv1 = QLatin1String(argv[1]);
    if ( argc > 2 )
        startDir = QUrl::fromUserInput(argv[2]);

#if 0 // SPLIT-TODO
    if (argv1 == QLatin1String("diroperator")) {
        KDirOperator *op = new KDirOperator(startDir, 0);
        KConfigGroup grp(KSharedConfig::openConfig(), "TestGroup" );
        op->setViewConfig(grp);
        op->setView(KFile::Simple);
        op->show();
        a.exec();
    } else
#endif
    if (argv1 == QLatin1String("localonly")) {
        QString name = KFileDialog::getOpenFileName(startDir);
        qDebug("filename=%s",name.toLatin1().constData());
    }
    else if (argv1 == QLatin1String("oneurl")) {
        QUrl url = KFileDialog::getOpenUrl(startDir);
        qDebug() << "url=" << url;
    }

    else if (argv1 == QLatin1String("existingDirectoryUrl")) {
        QUrl url = KFileDialog::getExistingDirectoryUrl();
        qDebug("URL=%s",url.toString().toLatin1().constData());
        name1 = url.toString();
    }

    else if (argv1 == QLatin1String("preview")) {
        QUrl u =  KFileDialog::getImageOpenUrl();
        qDebug("filename=%s", u.toString().toLatin1().constData());
    }

    else if (argv1 == QLatin1String("preselect")) {
        names = KFileDialog::getOpenFileNames(QUrl::fromLocalFile("/etc/passwd"));
        QStringList::Iterator it = names.begin();
        while ( it != names.end() ) {
            qDebug("selected file: %s", (*it).toLatin1().constData());
            ++it;
        }
    }

    else if (argv1 == QLatin1String("dirs"))
        name1 = KFileDialog::getExistingDirectory();

    else if (argv1 == QLatin1String("heap")) {
        KFileDialog *dlg = new KFileDialog( startDir, QString(), 0L);
        dlg->setMode( KFile::File);
        dlg->setOperationMode( KFileDialog::Saving );
        QStringList filter;
        filter << "all/allfiles" << "text/plain";
        dlg->setMimeFilter( filter, "all/allfiles" );
#if 0 // SPLIT-TODO
        KUrlBar *urlBar = dlg->speedBar();
        if ( urlBar )
        {
            urlBar->insertDynamicItem( QUrl("ftp://ftp.kde.org"),
                                       QLatin1String("KDE FTP Server") );
        }
#endif
        if ( dlg->exec() == QDialog::Accepted )
            name1 = dlg->selectedUrl().toString();
    }

    else if ( argv1 == QLatin1String("eventloop") )
    {
        new KFDTest( startDir );
        return a.exec();
    }

    else if (argv1 == QLatin1String("save")) {
        QUrl u = KFileDialog::getSaveUrl(startDir);
//        QString(QDir::homePath() + QLatin1String("/testfile")),
//        QString(), 0L);
        name1 = u.toString();
    }

    else if (argv1 == QLatin1String("icon")) {
        KIconDialog dlg;
        QString icon = dlg.getIcon();
        kDebug() << icon;
    }

//     else if ( argv1 == QLatin1String("dirselect") ) {
//         QUrl url = QUrl::fromLocalFile("/");
//         QUrl selected = KDirSelectDialog::selectDirectory( url );
//         name1 = selected.toString();
//         qDebug("*** selected: %s", selected.toString().toLatin1().constData());
//     }

    else {
        KFileDialog dlg(startDir,
                        QString::fromLatin1("*|All Files\n"
                                            "*.lo *.o *.la|All libtool Files"),0);
//    dlg.setFilter( "*.kdevelop" );
        dlg.setMode( KFile::Files |
                     KFile::Directory |
                     KFile::ExistingOnly |
                     KFile::LocalOnly );
//        QStringList filter;
//        filter << "text/plain" << "text/html" << "image/png";
//        dlg.setMimeFilter( filter );
//        KMimeType::List types;
//        types.append( KMimeType::mimeType( "text/plain" ) );
//        types.append( KMimeType::mimeType( "text/html" ) );
//        dlg.setFilterMimeType( "Filetypes:", types, types.first() );
        if ( dlg.exec() == QDialog::Accepted ) {
            const QList<QUrl> list = dlg.selectedUrls();
            QList<QUrl>::ConstIterator it = list.constBegin();
            qDebug("*** selectedUrls(): ");
            while ( it != list.constEnd() ) {
                name1 = (*it).toString();
                qDebug("  -> %s", name1.toLatin1().constData());
                ++it;
            }
            qDebug("*** selectedFile: %s", dlg.selectedFile().toLatin1().constData());
            qDebug("*** selectedUrl: %s", dlg.selectedUrl().toString().toLatin1().constData());
            qDebug("*** selectedFiles: ");
            QStringList l = dlg.selectedFiles();
            QStringList::Iterator it2 = l.begin();
            while ( it2 != l.end() ) {
                qDebug("  -> %s", (*it2).toLatin1().constData());
                ++it2;
            }
        }
    }

    if (!(name1.isNull()))
        KMessageBox::information(0, QLatin1String("You selected the file " ) + name1,
                                 QLatin1String("Your Choice"));
    return 0;
}
