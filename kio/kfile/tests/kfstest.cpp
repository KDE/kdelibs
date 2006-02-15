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
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <qdir.h>
#include <qlayout.h>
#include <qstringlist.h>
#include <qwidget.h>

#include <kfiledialog.h>
#include <kfileiconview.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kurl.h>
#include <kurlbar.h>
#include <kdiroperator.h>
#include <kfile.h>
#include <kdebug.h>
#include <kicondialog.h>

#include "kfdtest.h"

int main(int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "kfstest","kfstest","test app","0");
    KApplication a;
    a.setQuitOnLastWindowClosed(false);

    QString name1;
    QStringList names;

    QString argv1;
    QString startDir;
    if (argc > 1)
	argv1 = QLatin1String(argv[1]);
    if ( argc > 2 )
        startDir = QLatin1String( argv[2]);

    if (argv1 == QLatin1String("diroperator")) {
	KDirOperator *op = new KDirOperator(startDir, 0);
	KConfigGroup grp(KGlobal::config(), "TestGroup" );
	op->setViewConfig(&grp);
	op->setView(KFile::Simple);
	op->show();
	a.setMainWidget(op);
	a.exec();
    }

    else if (argv1 == QLatin1String("justone")) {
        QString name = KFileDialog::getOpenFileName(startDir);
        qDebug("filename=%s",name.latin1());
    }

    else if (argv1 == QLatin1String("existingURL")) {
        KUrl url = KFileDialog::getExistingURL();
        qDebug("URL=%s",url.url().latin1());
        name1 = url.url();
    }

    else if (argv1 == QLatin1String("preview")) {
        KUrl u =  KFileDialog::getImageOpenURL();
        qDebug("filename=%s", u.url().latin1());
    }

    else if (argv1 == QLatin1String("preselect")) {
        names = KFileDialog::getOpenFileNames(QLatin1String("/etc/passwd"));
        QStringList::Iterator it = names.begin();
        while ( it != names.end() ) {
            qDebug("selected file: %s", (*it).latin1());
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
    KUrlBar *urlBar = dlg->speedBar();
    if ( urlBar )
    {
        urlBar->insertDynamicItem( KUrl("ftp://ftp.kde.org"), 
                                   QLatin1String("KDE FTP Server") );
    }

	if ( dlg->exec() == KDialog::Accepted )
	    name1 = dlg->selectedURL().url();
    }

    else if ( argv1 == QLatin1String("eventloop") )
    {
        KFDTest *test = new KFDTest( startDir );
        return a.exec();
    }

    else if (argv1 == QLatin1String("save")) {
        KUrl u = KFileDialog::getSaveURL();
//        QString(QDir::homePath() + QLatin1String("/testfile")),
//        QString(), 0L);
        name1 = u.url();
    }

    else if (argv1 == QLatin1String("icon")) {
    	KIconDialog dlg;
	QString icon = dlg.selectIcon();
	kDebug() << icon << endl;
    }

//     else if ( argv1 == QLatin1String("dirselect") ) {
//         KUrl url;
//         url.setPath( "/" );
//         KUrl selected = KDirSelectDialog::selectDirectory( url );
//         name1 = selected.url();
//         qDebug("*** selected: %s", selected.url().latin1());
//     }

    else {
	KFileDialog dlg(startDir,
			QString::fromLatin1("*|All Files\n"
					    "*.lo *.o *.la|All libtool Files"),0);
//    dlg.setFilter( "*.kdevelop" );
	dlg.setMode( (KFile::Mode) (KFile::Files |
                                    KFile::Directory |
                                    KFile::ExistingOnly |
                                    KFile::LocalOnly) );
//         QStringList filter;
//         filter << "text/plain" << "text/html" << "image/png";
//        dlg.setMimeFilter( filter );
//    KMimeType::List types;
//    types.append( KMimeType::mimeType( "text/plain" ) );
//    types.append( KMimeType::mimeType( "text/html" ) );
//    dlg.setFilterMimeType( "Filetypes:", types, types.first() );
	if ( dlg.exec() == QDialog::Accepted ) {
	    KUrl::List list = dlg.selectedURLs();
	    KUrl::List::ConstIterator it = list.begin();
            qDebug("*** selectedURLs(): ");
	    while ( it != list.end() ) {
		name1 = (*it).url();
		qDebug("  -> %s", name1.latin1());
		++it;
            }
            qDebug("*** selectedFile: %s", dlg.selectedFile().latin1());
            qDebug("*** selectedURL: %s", dlg.selectedURL().url().latin1());
            qDebug("*** selectedFiles: ");
            QStringList l = dlg.selectedFiles();
            QStringList::Iterator it2 = l.begin();
            while ( it2 != l.end() ) {
                qDebug("  -> %s", (*it2).latin1());
                ++it2;
            }
	}
    }

    if (!(name1.isNull()))
	KMessageBox::information(0, QLatin1String("You selected the file " ) + name1,
				 QLatin1String("Your Choice"));
    return 0;
}
