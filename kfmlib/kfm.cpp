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
#include <qmessagebox.h>
#include <qapplication.h>

#include "kfm.h"
#include <kapp.h>

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
    warning("KFM class (kfmlib) is deprecated !! It won't do anything.");
    flag = 0;
    ok = FALSE;
    allowRestart = FALSE;
    modal_hack_widget = 0;

    init();
}

KFM::~KFM()
{
}              


bool KFM::download(const QString & src, QString & target){
  KURL u (src);
  if (u.isLocalFile()){
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
    ok = TRUE;
}

void KFM::refreshDesktop()
{
}

void KFM::sortDesktop()
{
}

void KFM::configure()
{
}

void KFM::openURL()
{
  warning("Deprecated KFM::openURL called. Use 'kfmclient openURL' instead.");
}

void KFM::openURL( const char *_url )
{
  warning("Deprecated KFM::openURL called. Use 'kfmclient openURL' instead.");
}

void KFM::list( const char *_url )
{
}

void KFM::refreshDirectory( const char *_url )
{
}

void KFM::openProperties( const char *_url )
{
  warning("Deprecated KFM::openURL called. Use PropertiesDialog (kio_dlgprops.h) instead.");
}

void KFM::exec( const char *_url, const char *_binding )
{
  warning("Deprecated KFM::openURL called. Use KRun instead.");
}

void KFM::copy( const char *_src, const char *_dest )
{
  warning("Deprecated KFM::copy called. Use KIOJob instead.");
}

void KFM::move( const char *_src, const char *_dest )
{
  warning("Deprecated KFM::move called. Use KIOJob instead.");
}

void KFM::copyClient( const char *_src, const char *_dest )
{
  warning("Deprecated KFM::copyClient called. Use KIOJob instead.");
}

void KFM::moveClient( const char *_src, const char *_dest )
{
  warning("Deprecated KFM::moveClient called. Use KIOJob instead.");
}

void KFM::selectRootIcons( int _x, int _y, int _w, int _h, bool _add )
{
}

void KFM::slotFinished()
{
}

bool KFM::test()
{
  return false;
}

void KFM::allowKFMRestart( bool _allow )
{
    allowRestart = _allow;
}

bool KFM::isKFMRunning()
{
  return FALSE;
}

void KFM::slotError( int _kerror, const QString&_text )
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

//static
void KFM::setSilent(bool _silent) { silent = _silent; }
bool KFM::silent = false;

DlgLocation::DlgLocation( const char *_text, const char* _value, QWidget *parent )
        : QDialog( parent, 0L, TRUE )
{

    warning("DlgLocation class (kfmlib) is deprecated !!");

    QLabel *label = new QLabel( _text , this );
    label->adjustSize(); // depends on the text length
    label->move(10,10);

    edit = new QLineEdit( this, 0L );
    connect( edit, SIGNAL(returnPressed()), SLOT(accept()) );

    ok = new QPushButton( i18n("OK"), this );
    connect( ok, SIGNAL(clicked()), SLOT(accept()) );
    ok->adjustSize();

    cancel = new QPushButton( i18n("Cancel"), this );
    connect( cancel, SIGNAL(clicked()), SLOT(reject()) );
    cancel->adjustSize();

    edit->setText( _value );
    edit->setFocus();

    setMinimumSize ( 200, 40+edit->height()+10+ok->height()+10 );
    setGeometry( x(), y(), label->width()+20, 110 );
}

void DlgLocation::resizeEvent(QResizeEvent *e)
{
    QDialog::resizeEvent(e);
    int w = rect().width();
    int h = rect().height();
    edit->setGeometry (10, 40, w-20, 20);
    ok->move( 10, h-10-ok->height() );
    cancel->move( w-10-cancel->width(), h-10-cancel->height() );
}

#include "kfm.moc"
