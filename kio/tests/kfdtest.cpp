#include "kfdtest.h"

#include <QtCore/QMutableStringListIterator>
#include <kfiledialog.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <QtCore/QTimer>

KFDTest::KFDTest( const KUrl& startDir, QObject *parent, const char *name )
    : QObject( parent ),
      m_startDir( startDir )
{
    setObjectName( name );
    QTimer::singleShot( 1000, this, SLOT( doit() ));
}

void KFDTest::doit()
{
    KFileDialog *dlg = new KFileDialog( m_startDir, QString(), 0L);
    dlg->setMode( KFile::File);
    dlg->setOperationMode( KFileDialog::Saving );
    QStringList filter;
    filter << "all/allfiles" << "text/plain";
    dlg->setMimeFilter( filter, "all/allfiles" );

    if ( dlg->exec() == KDialog::Accepted )
    {
        KMessageBox::information(0, QString::fromLatin1("You selected the file: %1").arg( dlg->selectedUrl().prettyUrl() ));
    }

//     qApp->quit();
}

#include "kfdtest.moc"
