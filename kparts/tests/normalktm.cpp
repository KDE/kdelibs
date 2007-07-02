
#include "normalktm.h"
#include "parts.h"
#include "notepad.h"

#include <QtGui/QSplitter>
#include <QtGui/QCheckBox>
#include <QtCore/QDir>
#include <QtGui/QMenu>

#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kmenubar.h>
#include <kicon.h>

Shell::Shell()
    : KXmlGuiWindow()
{
    // We can do this "switch active part" because we have a splitter with
    // two items in it.
    // I wonder what kdevelop uses/will use to embed kedit, BTW.
    m_splitter = new QSplitter( this );

    m_part1 = new Part1(this, m_splitter);
    m_part2 = new Part2(this, m_splitter);

    QMenu * pFile = new QMenu( "File", menuBar() );
    KActionCollection * coll = actionCollection();
    KAction * paLocal = new KAction( "&View local file", this );
    coll->addAction( "open_local_file", paLocal );
    connect(paLocal, SIGNAL(triggered()), this, SLOT(slotFileOpen()));
    // No XML: we need to add our actions to the menus ourselves
    pFile->addAction(paLocal);

    KAction * paRemote = new KAction( "&View remote file", this );
    coll->addAction( "open_remote_file", paRemote );
    connect(paRemote, SIGNAL(triggered()), this, SLOT(slotFileOpenRemote()));
    pFile->addAction(paRemote);

    m_paEditFile = new KAction( "&Edit file", this );
    coll->addAction( "edit_file", m_paEditFile );
    connect(m_paEditFile, SIGNAL(triggered()), this, SLOT(slotFileEdit()));
    pFile->addAction(m_paEditFile);

    m_paCloseEditor = new KAction( "&Close file editor", this );
    coll->addAction( "close_editor", m_paCloseEditor );
    connect(m_paCloseEditor, SIGNAL(triggered()), this, SLOT(slotFileCloseEditor()));
    m_paCloseEditor->setEnabled(false);
    pFile->addAction(m_paCloseEditor);

    KAction * paQuit = new KAction( "&Quit", this );
    coll->addAction( "shell_quit", paQuit );
    connect(paQuit, SIGNAL(triggered()), this, SLOT(close()));
    paQuit->setIcon(KIcon("application-exit"));
    pFile->addAction(paQuit);

    setCentralWidget( m_splitter );
    m_splitter->setMinimumSize( 400, 300 );

    m_splitter->show();

    m_editorpart = 0;
}

Shell::~Shell()
{
}

void Shell::slotFileOpen()
{
    if ( !m_part1->openUrl( KStandardDirs::locate("data", KGlobal::mainComponent().componentName()+"/kpartstest_shell.rc" ) ) )
        KMessageBox::error(this, "Couldn't open file !");
}

void Shell::slotFileOpenRemote()
{
    KUrl u ( "http://www.kde.org/index.html" );
    if ( !m_part1->openUrl( u ) )
        KMessageBox::error(this, "Couldn't open file !");
}

void Shell::embedEditor()
{
    // replace part2 with the editor part
    delete m_part2;
    m_part2 = 0;
    m_editorpart = new NotepadPart( m_splitter, this );
    m_editorpart->setReadWrite(); // read-write mode
    ////// m_manager->addPart( m_editorpart );
    m_editorpart->widget()->show(); //// we need to do this in a normal KTM....
    m_paEditFile->setEnabled(false);
    m_paCloseEditor->setEnabled(true);
}

void Shell::slotFileCloseEditor()
{
    delete m_editorpart;
    m_editorpart = 0;
    m_part2 = new Part2(this, m_splitter);
    ////// m_manager->addPart( m_part2 );
    m_part2->widget()->show(); //// we need to do this in a normal KTM....
    m_paEditFile->setEnabled(true);
    m_paCloseEditor->setEnabled(false);
}

void Shell::slotFileEdit()
{
    if ( !m_editorpart )
        embedEditor();
    // TODO use KFileDialog to allow testing remote files
    if ( ! m_editorpart->openUrl( QDir::current().absolutePath()+"/kpartstest_shell.rc" ) )
        KMessageBox::error(this,"Couldn't open file !");
}

int main( int argc, char **argv )
{
    // we cheat and call ourselves kpartstest for Shell::slotFileOpen()
    KCmdLineArgs::init( argc, argv, "kpartstest", 0, ki18n("kpartstest"), 0);
    KApplication app;

    Shell *shell = new Shell;
    shell->show();

    app.exec();

    return 0;
}

#include "normalktm.moc"
