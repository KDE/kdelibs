#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <qdir.h>
#include <qstring.h>
#include <qmsgbox.h>
#include <qapp.h>

#include "kfm.h"

KFM::KFM()
{
    flag = 0;
    ok = FALSE;
    ipc = 0L;
    allowRestart = FALSE;
    
    init();
}

KFM::~KFM()
{
    if ( ipc )
	delete ipc;
}              

void KFM::init()
{
    QString file = QDir::homeDirPath();
    file += "/.kde/share/apps/kfm/pid";
    
    // Try to open the pid file
    FILE *f = fopen( file.data(), "rb" );
    if ( f == 0L )
    {
	// Did we already try to start a new kfm ?
	if ( flag == 0 && allowRestart )
	{
	    // Try to start a new kfm
	    system( "kfm -d &" );
	    // dont try twice
	    flag = 1;
	    sleep( 10 );
	    init();
	    return;
	}
	
	warning("ERROR: KFM is not running\n");
	return;
    }
    
    // Read the PID
    char buffer[ 1024 ];
    buffer[0] = 0;
    fgets( buffer, 1023, f );
    int pid = atoi( buffer );
    if ( pid <= 0 )
    {
	warning("ERROR: Invalid PID\n");
	fclose( f );
	return;
    }

    // Is the PID ok ?
    if ( kill( pid, 0 ) != 0 )
    {
	// Did we already try to start a new kfm ?
	if ( flag == 0 && allowRestart )
	{
	    flag = 1;
	    // Try to start a new kfm
	    system( "kfm -d &" );
	    sleep( 10 );
	    fclose( f );
	    init();
	    return;
	}

	warning("ERROR: KFM crashed\n");
	fclose( f );
	return;
    }

    // Read the port number
    buffer[0] = 0;
    fgets( buffer, 1023, f );
    fclose( f );
    int slot = atoi( buffer );
    if ( slot <= 0 )
    {
	warning("ERROR: Invalid Slot\n");
	return;
    }
    
    // Connect to KFM
    ipc = new KfmIpc( slot );

    connect( ipc, SIGNAL( finished() ), this, SLOT( slotFinished() ) );
    connect( ipc, SIGNAL( error( int, const char* ) ),
	     this, SLOT( slotError( int, const char* ) ) );
    connect( ipc, SIGNAL( dirEntry( const char*, const char*, const char*, const char*, const char*, int ) ),
	     this, SLOT( slotDirEntry( const char*, const char*, const char*, const char*, const char*, int ) ) );

    // Read the password
    QString fn = getenv( "HOME" );
    fn += "/.kde/share/apps/kfm/magic";
    f = fopen( fn.data(), "rb" );
    if ( f == 0L )
    {
	QMessageBox::message( "KFM Error",
			      "You dont have the file ~/.kde/share/apps/kfm/magic\n\rCould not do Authorization" );
	return;
    }
    char *p = fgets( buffer, 1023, f );
    fclose( f );
    if ( p == 0L )
    {
	QMessageBox::message( "KFM Error",
			      "The file ~/.kde/share/apps/kfm/magic is corrupted\n\rCould not do Authorization" );
	return;
    }

    ipc->auth( buffer );
    
    ok = TRUE;
}

void KFM::refreshDesktop()
{
    if ( !test() )
	return;
    
    ipc->refreshDesktop();
}

void KFM::sortDesktop()
{
    if ( !test() )
	return;
    
    ipc->sortDesktop();
}

void KFM::openURL()
{
    if ( !test() )
	return;
    
    ipc->openURL( "" );
}

void KFM::openURL( const char *_url )
{
    if ( !test() )
	return;
    
    ipc->openURL( _url );
}

void KFM::list( const char *_url )
{
    if ( !test() )
	return;
    
    ipc->list( _url );
}

void KFM::refreshDirectory( const char *_url )
{
    if ( !test() )
	return;
    
    ipc->refreshDirectory( _url );
}

void KFM::openProperties( const char *_url )
{
    if ( !test() )
	return;
    
    ipc->openProperties( _url );
}

void KFM::exec( const char *_url, const char *_binding )
{
    if ( !test() )
	return;
    
    ipc->exec( _url, _binding );
}

void KFM::copy( const char *_src, const char *_dest )
{
    if ( !test() )
	return;
    
    ipc->copy( _src, _dest );
}

void KFM::move( const char *_src, const char *_dest )
{
    if ( !test() )
	return;
    
    ipc->move( _src, _dest );
}

void KFM::copyClient( const char *_src, const char *_dest )
{
    if ( !test() )
	return;
    
    ipc->copyClient( _src, _dest );
}

void KFM::moveClient( const char *_src, const char *_dest )
{
    if ( !test() )
	return;
    
    ipc->moveClient( _src, _dest );
}

void KFM::selectRootIcons( int _x, int _y, int _w, int _h, bool _add )
{
    warning( "KFM call: selectRootIcons\n");
    if ( !test() )
	return;
    warning( "KFM doing call\n");
    
    ipc->selectRootIcons( _x, _y, _w, _h, _add );
}

void KFM::slotFinished()
{
    emit finished();
}

bool KFM::test()
{
    if ( ( ipc == 0L || !ipc->isConnected() ) && allowRestart )
    {
	warning( "*********** KFM crashed **************\n" );
	if ( ipc )
	    delete ipc;
	
	ipc = 0L;
	flag = 0;
	ok = FALSE;

	warning( "KFM recovery\n" );
	init();
	warning( "KFM recovery done\n" );
    }

    if ( ipc == 0L )
	warning( "KFM NOT READY\n");
    
    return ( ipc==0L?false:true );
}

void KFM::allowKFMRestart( bool _allow )
{
    allowRestart = _allow;
}

bool KFM::isKFMRunning()
{
    if ( ipc == 0L ) return FALSE;
    if ( ipc->isConnected() )
	return TRUE;
    return FALSE;
}

void KFM::slotError( int _kerror, const char *_text )
{
  emit error( _kerror, _text );
}

void KFM::slotDirEntry(const char* _name, const char* _access, const char* _owner,
		  const char* _group, const char* _date, int _size)
{
  entry.name = _name;
  entry.access = _access;
  entry.owner = _owner;
  entry.group = _group;
  entry.date = _date;
  entry.size = _size;
  emit dirEntry( entry );
}

DlgLocation::DlgLocation( const char *_text, const char* _value, QWidget *parent )
        : QDialog( parent, 0L, TRUE )
{
    setGeometry( x(), y(), 200, 110 );

    QLabel *label = new QLabel( _text , this );
    label->setGeometry( 10, 10, 180, 15 );
    
    edit = new QLineEdit( this, 0L );
    edit->setGeometry( 10, 40, 180, 20 );
    connect( edit, SIGNAL(returnPressed()), SLOT(accept()) );

    QPushButton *ok;
    QPushButton *cancel;
    ok = new QPushButton( "Ok", this );
    ok->setGeometry( 10,70, 50,30 );
    connect( ok, SIGNAL(clicked()), SLOT(accept()) );

    cancel = new QPushButton( "Cancel", this );
    cancel->setGeometry( 140, 70, 50, 30 );
    connect( cancel, SIGNAL(clicked()), SLOT(reject()) );

    edit->setText( _value );
    edit->setFocus();
}

#include "kfm.moc"



