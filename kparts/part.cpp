/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
             (C) 1999 David Faure <faure@kde.org>

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

#include <kparts/part.h>
#include <kparts/event.h>
#include <kparts/plugin.h>
#include <kparts/mainwindow.h>
#include <kparts/partmanager.h>

#include <qfile.h>
#include <qpoint.h>
#include <qpointarray.h>
#include <qpainter.h>
#include <qtextstream.h>

#include <kinstance.h>
#include <klocale.h>
#include <ktempfile.h>
#include <kmessagebox.h>
#include <kio/job.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <kdebug.h>

template class QPtrList<KXMLGUIClient>;

using namespace KParts;

namespace KParts
{

class PartBasePrivate
{
public:
  PartBasePrivate()
  {
  }
  ~PartBasePrivate()
  {
  }
};

class PartPrivate
{
public:
  PartPrivate()
  {
    m_bSelectable = true;
  }
  ~PartPrivate()
  {
  }

  bool m_bSelectable;
};
};

PartBase::PartBase()
{
  d = new PartBasePrivate;
  m_obj = 0L;
}

PartBase::~PartBase()
{
  delete d;
}

void PartBase::setPartObject( QObject *obj )
{
  m_obj = obj;
}

QObject *PartBase::partObject() const
{
  return m_obj;
}

void PartBase::setInstance( KInstance *inst )
{
  setInstance( inst, true );
}

void PartBase::setInstance( KInstance *inst, bool loadPlugins )
{
  KXMLGUIClient::setInstance( inst );
  KGlobal::locale()->insertCatalogue( inst->instanceName() );
  // install 'instancename'data resource type
  KGlobal::dirs()->addResourceType( inst->instanceName() + "data",
                                    KStandardDirs::kde_default( "data" )
                                    + QString::fromLatin1( inst->instanceName() ) + '/' );
  if ( loadPlugins )
    Plugin::loadPlugins( m_obj, instance() );
}

Part::Part( QObject *parent, const char* name )
 : QObject( parent, name )
{
  d = new PartPrivate;
  m_widget = 0L;
  m_manager = 0L;
  PartBase::setPartObject( this );
}

Part::~Part()
{
  kdDebug(1000) << "Part::~Part " << this << endl;
  if ( m_manager )
    m_manager->removePart(this);

  if ( m_widget )
  {
    // We need to disconnect first, to avoid calling it !
    disconnect( m_widget, SIGNAL( destroyed() ),
                this, SLOT( slotWidgetDestroyed() ) );
    kdDebug(1000) << "deleting widget " << m_widget << " " << m_widget->name() << endl;
    delete (QWidget*) m_widget;
  }

  delete d;
}

void Part::embed( QWidget * parentWidget )
{
  if ( widget() )
    widget()->reparent( parentWidget, 0, QPoint( 0, 0 ), true );
}

QWidget *Part::widget()
{
  return m_widget;
}

void Part::setManager( PartManager *manager )
{
  m_manager = manager;
}

PartManager *Part::manager() const
{
  return m_manager;
}

Part *Part::hitTest( QWidget *widget, const QPoint & )
{
  if ( (QWidget *)m_widget != widget )
    return 0L;

  return this;
}

void Part::setWidget( QWidget *widget )
{
  assert ( !m_widget ); // otherwise we get two connects
  m_widget = widget;
  connect( m_widget, SIGNAL( destroyed() ),
           this, SLOT( slotWidgetDestroyed() ) );

  // Tell the actionCollection() which widget its
  //  action shortcuts should be connected to.
  actionCollection()->setWidget( widget );

  // Since KParts objects are XML-based, shortcuts should
  //  be connected to the widget when the XML settings
  //  are processed, rather than on KAction construction.
  actionCollection()->setAutoConnectShortcuts( false );
}

void Part::setSelectable( bool selectable )
{
  d->m_bSelectable = selectable;
}

bool Part::isSelectable() const
{
  return d->m_bSelectable;
}

void Part::customEvent( QCustomEvent *event )
{
  if ( PartActivateEvent::test( event ) )
  {
    partActivateEvent( (PartActivateEvent *)event );
    return;
  }

  if ( PartSelectEvent::test( event ) )
  {
    partSelectEvent( (PartSelectEvent *)event );
    return;
  }

  if ( GUIActivateEvent::test( event ) )
  {
    guiActivateEvent( (GUIActivateEvent *)event );
    return;
  }

  QObject::customEvent( event );
}

void Part::partActivateEvent( PartActivateEvent * )
{
}

void Part::partSelectEvent( PartSelectEvent * )
{
}

void Part::guiActivateEvent( GUIActivateEvent * )
{
}

QWidget *Part::hostContainer( const QString &containerName )
{
  if ( !factory() )
    return 0L;

  return factory()->container( containerName, this );
}

void Part::slotWidgetDestroyed()
{
  kdDebug(1000) << "KPart::slotWidgetDestroyed(), deleting part " << name() << endl;
  m_widget = 0;
  delete this;
}

//////////////////////////////////////////////////

namespace KParts
{

class ReadOnlyPartPrivate
{
public:
  ReadOnlyPartPrivate()
  {
    m_job = 0L;
    m_showProgressInfo = true;
  }
  ~ReadOnlyPartPrivate()
  {
  }

  KIO::FileCopyJob * m_job;
  bool m_showProgressInfo;
};

};

ReadOnlyPart::ReadOnlyPart( QObject *parent, const char *name )
 : Part( parent, name ), m_bTemp( false )
{
  d = new ReadOnlyPartPrivate;
}

ReadOnlyPart::~ReadOnlyPart()
{
  ReadOnlyPart::closeURL();
  delete d;
}

void ReadOnlyPart::setProgressInfoEnabled( bool show )
{
  d->m_showProgressInfo = show;
}

bool ReadOnlyPart::isProgressInfoEnabled() const
{
  return d->m_showProgressInfo;
}

#ifndef KDE_NO_COMPAT
void ReadOnlyPart::showProgressInfo( bool show )
{
  d->m_showProgressInfo = show;
}
#endif

bool ReadOnlyPart::openURL( const KURL &url )
{
  if ( url.isMalformed() )
    return false;
  if ( !closeURL() )
    return false;
  m_url = url;
  emit setWindowCaption( m_url.prettyURL() );
  if ( m_url.isLocalFile() )
  {
    emit started( 0 );
    m_file = m_url.path();
    bool ret = openFile();
    if (ret)
        emit completed();
    return ret;
  }
  else
  {
    m_bTemp = true;
    // Use same extension as remote file. This is important for mimetype-determination (e.g. koffice)
    QString extension;
    QString fileName = url.fileName();
    int extensionPos = fileName.findRev( '.' );
    if ( extensionPos != -1 && url.query().isNull() ) // not if the URL has a query, e.g. cgi.pl?something
        extension = fileName.mid( extensionPos ); // keep the '.'
    KTempFile tempFile( QString::null, extension );
    m_file = tempFile.name();

    KURL destURL;
    destURL.setPath( m_file );
    d->m_job = KIO::file_copy( m_url, destURL, 0600, true, false, d->m_showProgressInfo );
    emit started( d->m_job );
    connect( d->m_job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotJobFinished ( KIO::Job * ) ) );
    return true;
  }
}

void ReadOnlyPart::abortLoad()
{
  if ( d->m_job )
  {
    //kdDebug(1000) << "Aborting job " << d->m_job << endl;
    d->m_job->kill();
    d->m_job = 0;
  }
}

bool ReadOnlyPart::closeURL()
{
  abortLoad(); //just in case

  if ( m_bTemp )
  {
    unlink( QFile::encodeName(m_file) );
    m_bTemp = false;
  }
  // It always succeeds for a read-only part,
  // but the return value exists for reimplementations
  // (e.g. pressing cancel for a modified read-write part)
  return true;
}

void ReadOnlyPart::slotJobFinished( KIO::Job * job )
{
  kdDebug(1000) << "ReadOnlyPart::slotJobFinished" << endl;
  assert( job == d->m_job );
  d->m_job = 0;
  if (job->error())
    emit canceled( job->errorString() );
  else
  {
    openFile();
    emit completed();
  }
}

void ReadOnlyPart::guiActivateEvent( GUIActivateEvent * event )
{
  if (event->activated())
  {
    if (!m_url.isEmpty())
    {
      kdDebug(1000) << "ReadOnlyPart::guiActivateEvent -> " << m_url.prettyURL() << endl;
      emit setWindowCaption( m_url.prettyURL() );
    } else emit setWindowCaption( "" );
  }
}

bool ReadOnlyPart::openStream( const QString& mimeType, const KURL& url )
{
    if ( !closeURL() )
        return false;
    m_url = url;
    return doOpenStream( mimeType );
}

bool ReadOnlyPart::writeStream( const QByteArray& data )
{
    return doWriteStream( data );
}

bool ReadOnlyPart::closeStream()
{
    return doCloseStream();
}

//////////////////////////////////////////////////

ReadWritePart::ReadWritePart( QObject *parent, const char *name )
 : ReadOnlyPart( parent, name ), m_bModified( false ), m_bClosing( false )
{
  m_bReadWrite = true;
}

ReadWritePart::~ReadWritePart()
{
  // parent destructor will delete temp file
  // we can't call our own closeURL() here, because
  // "cancel" wouldn't cancel anything. We have to assume
  // the app called closeURL() before destroying us.
}

void ReadWritePart::setReadWrite( bool readwrite )
{
  // Perhaps we should check isModified here and issue a warning if true
  m_bReadWrite = readwrite;
}

void ReadWritePart::setModified( bool modified )
{
  kdDebug(1000) << "ReadWritePart::setModified( " << (modified ? "true" : "false") << ")" << endl;
  if ( !m_bReadWrite && modified )
  {
      kdError(1000) << "Can't set a read-only document to 'modified' !" << endl;
      return;
  }
  m_bModified = modified;
}

void ReadWritePart::setModified()
{
  setModified( true );
}

bool ReadWritePart::closeURL()
{
  abortLoad(); //just in case
  if ( m_bModified && m_bReadWrite )
  {
    int res = KMessageBox::warningYesNoCancel( widget(),
            i18n( "The document \"%1\" has been modified.\n"
                  "Do you want to save it?" ).arg( url().fileName() ),
            i18n( "Save Document?" ), KStdGuiItem::save(), KStdGuiItem::discard() );

    switch(res) {
    case KMessageBox::Yes :
      m_bClosing = true; // remember to clean up the temp file
      if (m_url.isEmpty())
      {
          KURL url = KFileDialog::getSaveURL();
          if (url.isEmpty())
          {
            m_bClosing = false;
            return false;
          }
          return saveAs( url );
      }
      return save();
    case KMessageBox::No :
      setModified( false ); // the user isn't interested in the changes, forget them
      return true;
    default : // case KMessageBox::Cancel :
      return false;
    }
  }
  // Not modified => ok and delete temp file.
  return ReadOnlyPart::closeURL();
}

bool ReadWritePart::save()
{
  if( saveFile() )
    return saveToURL();
  m_bClosing = false;
  return false;
}

bool ReadWritePart::saveAs( const KURL & kurl )
{
  if (kurl.isMalformed())
  {
      kdError(1000) << "saveAs: Malformed URL" << kurl.url() << endl;
      m_bClosing = false;
      return false;
  }
  m_url = kurl; // Store where to upload in saveToURL
  // Local file
  if ( m_url.isLocalFile() )
  {
    if ( m_bTemp ) // get rid of a possible temp file first
    {              // (happens if previous url was remote)
      unlink( QFile::encodeName(m_file) );
      m_bTemp = false;
    }
    m_file = m_url.path();
  }
  else
  { // Remote file
    // We haven't saved yet, or we did but locally - provide a temp file
    if ( m_file.isEmpty() || !m_bTemp )
    {
      KTempFile tempFile;
      m_file = tempFile.name();
      m_bTemp = true;
    }
    // otherwise, we already had a temp file
  }
  emit setWindowCaption( m_url.prettyURL() );
  return save(); // Save local file and upload local file
}

bool ReadWritePart::saveToURL()
{
  if ( m_url.isLocalFile() )
  {
    setModified( false );
    emit completed();
    // if m_url is a local file there won't be a temp file -> nothing to remove
    assert( !m_bTemp );
    m_bClosing = false; // no temp file to cleaned up
    return true; // Nothing to do
  }
  else
  {
    KIO::Job * job = KIO::file_copy( m_file, m_url, -1, true /*overwrite*/ );
    connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotUploadFinished (KIO::Job *) ) );
    return true;
  }
}

void ReadWritePart::slotUploadFinished( KIO::Job * job )
{
  if (job->error())
    emit canceled( job->errorString() );
  else
  {
    setModified( false );
    if ( m_bClosing && m_bTemp ) // We're finished with this document -> remove temp file
    {
      unlink( QFile::encodeName(m_file) );
      m_bTemp = false;
    }
    emit completed();
  }
  m_bClosing = false; // temp file was cleaned up
}

#include "part.moc"

// vim:sw=2:ts=8:et
