/* This file is part of the KDE libraries
    Copyright (C) 1997 Torben Weis (weis@kde.org)

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <qdir.h>
#include <qstring.h>
#include <qmsgbox.h>
#include <qapp.h>

#include "kfm.h"
#include <kapp.h>
#include <kstring.h>

QString displayName()
{
    QString d( getenv( "DISPLAY" ) );
    int i = d.find( ':' );
    if ( i != -1 )
	d[i] = '_';
    if ( d.find( '.' ) == -1 )
	d += ".0";
    
    return d;
}

KFM::KFM()
{
    flag = 0;
    ok = FALSE;
    ipc = 0L;
    allowRestart = FALSE;
    modal_hack_widget = 0;

    init();
}

KFM::~KFM()
{
    if ( ipc )
	delete ipc;
}              


bool KFM::download(const QString & src, QString & target){
  KURL u (src);
  if (qstrcmp( u.protocol(), "file")==0){
    // file protocol. We do not need the network
    target = u.path();
    return true;
  }
  KFM* kfm = new KFM;
  bool result = kfm->downloadInternal(src,target);
  delete kfm;
  return result;
}

QStrList* KFM::tmpfiles = 0;

void KFM::removeTempFile(const QString & name){
  if (!tmpfiles)
    return;
  if (tmpfiles->contains(name)){
    unlink(name);
    tmpfiles->remove(name);
  }
}

bool KFM::downloadInternal(const QString & src, QString & target){
  if (target.isEmpty()){
    target = tmpnam(0);
    if (!tmpfiles)
      tmpfiles = new QStrList;
    tmpfiles->append(qstrdup(target.data()));
  }
  download_state = true; // success

  /* this is a bit tricky. We use a faked modal dialog to be able to
     process the download syncronious. For the user it will look
     (almost) as if the kfm-dialog is the modal dialog of your
     application. After show() we will also enter a local event loop
     within Qt. The modal_hack_widget will be hidden and destroyed in
     the finish slot. This will implictly exit the local event loop
     in addition (Matthias) 
  */
  modal_hack_widget = new QWidget(0,0,WType_Modal);
  modal_hack_widget->setGeometry(-10,-10,2,2);
  copy(src, target);
  modal_hack_widget->show();
  qApp->enter_loop();
  return download_state; 
}


void KFM::init()
{
    QString file = KApplication::localkdedir() + "/share/apps/kfm/pid";
    file += displayName();
    
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
	
	warning("ERROR: KFM is not running");
	return;
    }
    
    // Read the PID
    char buffer[ 1024 ];
    buffer[0] = 0;
    fgets( buffer, 1023, f );
    int pid = atoi( buffer );
    if ( pid <= 0 )
    {
	warning("ERROR: Invalid PID");
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

	warning("ERROR: KFM crashed");
	fclose( f );
	return;
    }

    // Read the socket's name
    buffer[0] = 0;
    fscanf(f, "%s", buffer); 
    fclose( f );
    char * slot = strdup( buffer );
    if ( slot == (void *) 0 )
    {
	warning("ERROR: Invalid Slot");
	return;
    }
    
    // Connect to KFM
    ipc = new KfmIpc( slot );
    free(slot);

    connect( ipc, SIGNAL( finished() ), this, SLOT( slotFinished() ) );
    connect( ipc, SIGNAL( error( int, const char* ) ),
	     this, SLOT( slotError( int, const char* ) ) );
    connect( ipc, SIGNAL( dirEntry( const char*, const char*, const char*, const char*, const char*, int ) ),
	     this, SLOT( slotDirEntry( const char*, const char*, const char*, const char*, const char*, int ) ) );

    // Read the password
    QString fn = KApplication::localkdedir() + "/share/apps/kfm/magic";
    f = fopen( fn.data(), "rb" );
    if ( f == 0L )
    {
	QString ErrorMessage;
	ksprintf(ErrorMessage, i18n("You dont have the file %s\n"
				    "Could not do Authorization"), fn.data());
	
	QMessageBox::message( i18n("KFM Error"), ErrorMessage );
	return;
    }
    char *p = fgets( buffer, 1023, f );
    fclose( f );
    if ( p == 0L )
    {
	ksprintf(ErrorMessage, i18n("The file %s is corrupted\n"
				    "Could not do Authorization"), fn.data());
	QMessageBox::message( i18n("KFM Error"), ErrorMessage );
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
    if ( !test() ) {
	return;
    }
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
    warning( "KFM call: selectRootIcons");
    if ( !test() )
	return;
    warning( "KFM doing call");
    
    ipc->selectRootIcons( _x, _y, _w, _h, _add );
}

void KFM::slotFinished()
{
  if (modal_hack_widget){
    modal_hack_widget->close(true);
    modal_hack_widget = 0;
    qApp->exit_loop();
  }
  emit finished();
}

bool KFM::test()
{
    if ( ( ipc == 0L || !ipc->isConnected() ) && allowRestart )
    {
	warning( "*********** KFM crashed **************" );
	if ( ipc )
	    delete ipc;
	
	ipc = 0L;
	flag = 0;
	ok = FALSE;

	warning( "KFM recovery" );
	init();
	warning( "KFM recovery done" );
    }

    if ( ipc == 0L )
	warning( "KFM NOT READY");
    
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
  download_state = false;
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
    ok = new QPushButton( i18n("Ok"), this );
    ok->setGeometry( 10,70, 50,30 );
    connect( ok, SIGNAL(clicked()), SLOT(accept()) );

    cancel = new QPushButton( i18n("Cancel"), this );
    cancel->setGeometry( 140, 70, 50, 30 );
    connect( cancel, SIGNAL(clicked()), SLOT(reject()) );

    edit->setText( _value );
    edit->setFocus();
}

#include "kfm.moc"



