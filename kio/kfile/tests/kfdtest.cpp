#include "kfdtest.h"

#include <qstringlist.h>
#include <kfiledialog.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <qtimer.h>

KFDTest::KFDTest( const QString& startDir, QObject *parent, const char *name )
    : QObject( parent, name ),
      m_startDir( startDir )
{
    QTimer::singleShot( 1000, this, SLOT( doit() ));
}

void KFDTest::doit()
{
    KFileDialog *dlg = new KFileDialog( m_startDir, QString(), 0L,
                                        "file dialog", true );
    dlg->setMode( KFile::File);
    dlg->setOperationMode( KFileDialog::Saving );
    QStringList filter;
    filter << "all/allfiles" << "text/plain";
    dlg->setMimeFilter( filter, "all/allfiles" );

    if ( dlg->exec() == KDialog::Accepted )
    {
        KMessageBox::information(0, QString::fromLatin1("You selected the file: %1").arg( dlg->selectedURL().prettyURL() ));
    }
    
//     qApp->quit();
}

#include "kfdtest.moc"
