#include <kde_terminal_interface.h>
#include <kparts/part.h>
#include <ktrader.h>
#include <klibloader.h>
#include <kmainwindow.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <qdir.h>
#include <assert.h>
#include <kmessagebox.h>
#include <cassert>
#include "main.h"
#include "main.moc"

Win::Win()
{
    KLibFactory* factory = KLibLoader::self()->factory( "libkonsolepart" );
    assert( factory );
    KParts::Part* p = static_cast<KParts::Part*>( factory->create( this, "tralala", "QObject", "KParts::ReadOnlyPart" ) );
    setCentralWidget( p->widget() );

    TerminalInterface* t = static_cast<TerminalInterface*>( p->qt_cast( "TerminalInterface" ) );
    t->showShellInDir( QDir::home().path() );
//     QStrList l;
//     l.append( "python" );
//     t->startProgram( QString::fromUtf8( "/usr/bin/python" ), l );

    connect( p, SIGNAL( processExited( int ) ),
             this, SLOT( pythonExited( int ) ) );
}


int main( int argc, char** argv )
{
    KAboutData* about = new KAboutData( "tetest", "TETest", "0.1" );
    KCmdLineArgs::init( argc, argv, about );
    KApplication a;
    Win* win = new Win();
    win->show();
    return a.exec();
};

#include <iostream>

void Win::pythonExited()
{
    std::cerr << "hee, " << p << std::endl;
    std::cerr << ( p->qt_cast( "TerminalInterface" ) ) << std::endl;
    // KMessageBox::sorry( this, QString::fromUtf8( "Exited, status was %1" ).arg( status ) );
    disconnect(p, SIGNAL( processExited() ),
            this, SLOT( pythonExited() ));
    TerminalInterface* t = static_cast<TerminalInterface*>( p->qt_cast( "TerminalInterface" ) );
    QStrList l;
    l.append( "echo" );
    l.append( "hello world" );
    t->startProgram( QString::fromUtf8( "/bin/echo" ), l );
}

void Win::forked()
{
    std::cerr << "hello from the child process!" << std::endl;
}
