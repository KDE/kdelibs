/* This file is part of the KDE project
   Copyright (C) 2002 David Faure <david@mandrakesoft.com>

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

#include "kmultipart.h"

#include <qvbox.h>
#include <kinstance.h>
#include <kmimetype.h>
#include <klocale.h>
#include <kio/job.h>
#include <qfile.h>
#include <ktempfile.h>
#include <kmessagebox.h>
#include <kparts/componentfactory.h>
#include <kparts/genericfactory.h>
#include <khtml_part.h>
#include <unistd.h>
#include <kxmlguifactory.h>

typedef KParts::GenericFactory<KMultiPart> KMultiPartFactory; // factory for the part
K_EXPORT_COMPONENT_FACTORY( libkmultipart /*library name*/, KMultiPartFactory );

//#define DEBUG_PARSING

class KLineParser
{
public:
    KLineParser() {
        m_lineComplete = false;
    }
    void addChar( char c, bool storeNewline ) {
        if ( !storeNewline && c == '\r' )
            return;
        Q_ASSERT( !m_lineComplete );
        if ( storeNewline || c != '\n' )
            m_currentLine += c;
        if ( c == '\n' )
            m_lineComplete = true;
    }
    bool isLineComplete() const {
        return m_lineComplete;
    }
    QCString currentLine() const {
        return m_currentLine;
    }
    void clearLine() {
        Q_ASSERT( m_lineComplete );
        reset();
    }
    void reset() {
        m_currentLine = 0L;
        m_lineComplete = false;
    }
private:
    QCString m_currentLine;
    bool m_lineComplete; // true when ending with '\n'
};

/* testcase:
   Content-type: multipart/mixed;boundary=ThisRandomString

--ThisRandomString
Content-type: text/plain

Data for the first object.

--ThisRandomString
Content-type: text/plain

Data for the second and last object.

--ThisRandomString--
*/


KMultiPart::KMultiPart( QWidget *parentWidget, const char *widgetName,
                        QObject *parent, const char *name, const QStringList& )
    : KParts::ReadOnlyPart( parent, name )
{
    setInstance( KMultiPartFactory::instance() );

    QVBox *box = new QVBox( parentWidget, widgetName );
    setWidget( box );

    m_extension = new KParts::BrowserExtension( this );

    // We probably need to use m_extension to get the urlArgs in openURL...

    m_part = 0L;
    m_job = 0L;
    m_lineParser = new KLineParser;
    m_tempFile = 0L;
}

KMultiPart::~KMultiPart()
{
    // important: delete the nested part before the part or qobject destructor runs.
    // we now delete the nested part which deletes the part's widget which makes
    // _OUR_ m_widget 0 which in turn avoids our part destructor to delete the
    // widget ;-)
    // ### additional note: it _can_ be that the part has been deleted before:
    // when we're in a html frameset and the view dies first, then it will also
    // kill the htmlpart
    if ( m_part )
        delete static_cast<KParts::ReadOnlyPart *>( m_part );
    delete m_job;
    delete m_lineParser;
    delete m_tempFile;
}

bool KMultiPart::openURL( const KURL &url )
{
    m_url = url;
    m_lineParser->reset();
    m_bParsingHeader = true; // we expect a header to come first
    m_bGotAnyHeader = false;

    KParts::URLArgs args = m_extension->urlArgs();
    //m_mimeType = args.serviceType;

    // Hmm, args.reload is set to true when reloading, but this doesn't seem to be enough...
    // I get "HOLD: Reusing held slave for <url>", and the old data

    m_job = KIO::get( url, args.reload, false );

    emit started( m_job );

    connect( m_job, SIGNAL( result( KIO::Job * ) ),
             this, SLOT( slotJobFinished( KIO::Job * ) ) );
    connect( m_job, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
             this, SLOT( slotData( KIO::Job *, const QByteArray & ) ) );

    return true;
}

// Yes, libkdenetwork's has such a parser already (MultiPart),
// but it works on the complete string, expecting the whole data to be available....
// The version here is asynchronous.
void KMultiPart::slotData( KIO::Job *job, const QByteArray &data )
{
    if (m_boundary.isNull())
    {
       QString tmp = job->queryMetaData("media-boundary");
       kdDebug() << "Got Boundary from kio-http '" << tmp << "'" << endl;
       m_boundary = QCString("--")+tmp.latin1();
       m_boundaryLength = m_boundary.length();
    }
    // Append to m_currentLine until eol
    for ( uint i = 0; i < data.size() ; ++i )
    {
        // Store char. Skip if '\n' and currently parsing a header.
        m_lineParser->addChar( data[i], !m_bParsingHeader );
        if ( m_lineParser->isLineComplete() )
        {
            QCString line = m_lineParser->currentLine();
#ifdef DEBUG_PARSING
            kdDebug() << "[" << m_bParsingHeader << "] line='" << line << "'" << endl;
#endif
            if ( m_bParsingHeader )
            {
                if ( !line.isEmpty() )
                    m_bGotAnyHeader = true;
                // ### HACK set the multipart boundary to the first line
                // TODO: get it as metadata from kio_http (part of Content-type field)
                if ( m_boundary.isNull() )
                {
                    if ( !line.isEmpty() ) {
#ifdef DEBUG_PARSING
                        kdDebug() << "Boundary is " << line << endl;
#endif
                        m_boundary = line;
                        m_boundaryLength = m_boundary.length();
                    }
                }
                // parse Content-Type
                else if ( !qstrnicmp( line.data(), "Content-Type:", 13 ) )
                {
                    Q_ASSERT( m_nextMimeType.isNull() );
                    m_nextMimeType = QString::fromLatin1( line.data() + 14 ).stripWhiteSpace();
                    kdDebug() << "m_nextMimeType=" << m_nextMimeType << endl;
                }
                // Empty line, end of headers (if we had any header line before)
                else if ( line.isEmpty() && m_bGotAnyHeader )
                {
                    m_bParsingHeader = false;
#ifdef DEBUG_PARSING
                    kdDebug() << "end of headers" << endl;
#endif
                    startOfData();
                }
                // First header (when we know it from kio_http)
                else if ( line == m_boundary )
                    ; // nothing to do
                else if ( !line.isEmpty() ) // this happens with e.g. Set-Cookie:
                    kdDebug() << "Ignoring header " << line << endl;
            } else {
                if ( !qstrncmp( line, m_boundary, m_boundaryLength ) )
                {
#ifdef DEBUG_PARSING
                    kdDebug() << "boundary found!" << endl;
#endif
                    //kdDebug() << "after it is " << line.data() + m_boundaryLength << endl;
                    // Was it the very last boundary ?
                    if ( !qstrncmp( line.data() + m_boundaryLength, "--", 2 ) )
                    {
#ifdef DEBUG_PARSING
                        kdDebug() << "Completed!" << endl;
#endif
                        endOfData();
                        emit completed();
                    } else
                    {
                        char nextChar = *(line.data() + m_boundaryLength);
                        //kdDebug() << "KMultiPart::slotData nextChar='" << nextChar << "'" << endl;
                        if ( nextChar == '\n' || nextChar == '\r' ) {
                            endOfData();
                            m_bParsingHeader = true;
                            m_bGotAnyHeader = false;
                        }
                        else {
                            // otherwise, false hit, it has trailing stuff
                            sendData( line );
                        }
                    }
                } else {
                    // send to part
                    sendData( line );
                }
            }
            m_lineParser->clearLine();
        }
    }
}

void KMultiPart::setPart( const QString& mimeType )
{
    KXMLGUIFactory *guiFactory = factory();
    guiFactory->removeClient( this );
    kdDebug() << "KMultiPart::setPart " << mimeType << endl;
    delete m_part;
    // Try to find an appropriate viewer component
    m_part = KParts::ComponentFactory::createPartInstanceFromQuery<KParts::ReadOnlyPart>
             ( m_mimeType, QString::null, widget(), 0L, this, 0L );
    if ( !m_part ) {
        // TODO launch external app
        KMessageBox::error( widget(), i18n("No handler found for %1!").arg(m_mimeType) );
        return;
    }
    // By making the part a child XMLGUIClient of ours, we get its GUI merged in.
    insertChildClient( m_part );
    m_part->widget()->show();

    connect( m_part, SIGNAL( completed() ),
             this, SLOT( slotPartCompleted() ) );

    KParts::BrowserExtension* childExtension = KParts::BrowserExtension::childObject( m_part );

    if ( childExtension )
    {

        // Forward signals from the part's browser extension
        // this is very related (but not exactly like) KHTMLPart::processObjectRequest

        connect( childExtension, SIGNAL( openURLNotify() ),
                 m_extension, SIGNAL( openURLNotify() ) );

        connect( childExtension, SIGNAL( openURLRequestDelayed( const KURL &, const KParts::URLArgs & ) ),
                 m_extension, SIGNAL( openURLRequest( const KURL &, const KParts::URLArgs & ) ) );

        connect( childExtension, SIGNAL( createNewWindow( const KURL &, const KParts::URLArgs & ) ),
                 m_extension, SIGNAL( createNewWindow( const KURL &, const KParts::URLArgs & ) ) );
        connect( childExtension, SIGNAL( createNewWindow( const KURL &, const KParts::URLArgs &, const KParts::WindowArgs &, KParts::ReadOnlyPart *& ) ),
                 m_extension, SIGNAL( createNewWindow( const KURL &, const KParts::URLArgs & , const KParts::WindowArgs &, KParts::ReadOnlyPart *&) ) );

        connect( childExtension, SIGNAL( popupMenu( const QPoint &, const KFileItemList & ) ),
                 m_extension, SIGNAL( popupMenu( const QPoint &, const KFileItemList & ) ) );
        connect( childExtension, SIGNAL( popupMenu( KXMLGUIClient *, const QPoint &, const KFileItemList & ) ),
                 m_extension, SIGNAL( popupMenu( KXMLGUIClient *, const QPoint &, const KFileItemList & ) ) );
        connect( childExtension, SIGNAL( popupMenu( const QPoint &, const KURL &, const QString &, mode_t ) ),
                 m_extension, SIGNAL( popupMenu( const QPoint &, const KURL &, const QString &, mode_t ) ) );
        connect( childExtension, SIGNAL( popupMenu( KXMLGUIClient *, const QPoint &, const KURL &, const QString &, mode_t ) ),
                 m_extension, SIGNAL( popupMenu( KXMLGUIClient *, const QPoint &, const KURL &, const QString &, mode_t ) ) );

        connect( childExtension, SIGNAL( infoMessage( const QString & ) ),
                 m_extension, SIGNAL( infoMessage( const QString & ) ) );

        childExtension->setBrowserInterface( m_extension->browserInterface() );

        connect( childExtension, SIGNAL( enableAction( const char *, bool ) ),
                 m_extension, SIGNAL( enableAction( const char *, bool ) ) );
        connect( childExtension, SIGNAL( setLocationBarURL( const QString& ) ),
                 m_extension, SIGNAL( setLocationBarURL( const QString& ) ) );
        connect( childExtension, SIGNAL( setIconURL( const KURL& ) ),
                 m_extension, SIGNAL( setIconURL( const KURL& ) ) );
        connect( childExtension, SIGNAL( loadingProgress( int ) ),
                 m_extension, SIGNAL( loadingProgress( int ) ) );
        connect( childExtension, SIGNAL( speedProgress( int ) ),
                 m_extension, SIGNAL( speedProgress( int ) ) );
        connect( childExtension, SIGNAL( selectionInfo( const KFileItemList& ) ),
                 m_extension, SIGNAL( selectionInfo( const KFileItemList& ) ) );
        connect( childExtension, SIGNAL( selectionInfo( const QString& ) ),
                 m_extension, SIGNAL( selectionInfo( const QString& ) ) );
        connect( childExtension, SIGNAL( selectionInfo( const KURL::List& ) ),
                 m_extension, SIGNAL( selectionInfo( const KURL::List& ) ) );
        connect( childExtension, SIGNAL( mouseOverInfo( const KFileItem* ) ),
                 m_extension, SIGNAL( mouseOverInfo( const KFileItem* ) ) );
    }

    m_isHTMLPart = ( mimeType == "text/html" );
    // Load the part's plugins too.
    loadPlugins( this, m_part, m_part->instance() );
    // Get the part's GUI to appear
    guiFactory->addClient( this );
}

void KMultiPart::startOfData()
{
    kdDebug() << "KMultiPart::startOfData" << endl;
    Q_ASSERT( !m_nextMimeType.isNull() );
    if( m_nextMimeType.isNull() )
        return;
    if ( m_mimeType != m_nextMimeType )
    {
        // Need to switch parts (or create the initial one)
        m_mimeType = m_nextMimeType;
        setPart( m_mimeType );
    }
    Q_ASSERT( m_part );
    // Pass URLArgs (e.g. reload)
    KParts::BrowserExtension* childExtension = KParts::BrowserExtension::childObject( m_part );
    if ( childExtension )
        childExtension->setURLArgs( m_extension->urlArgs() );

    m_nextMimeType = QString::null;
    delete m_tempFile;
    m_tempFile = 0L;
    if ( m_isHTMLPart )
    {
        KHTMLPart* htmlPart = static_cast<KHTMLPart *>( static_cast<KParts::ReadOnlyPart *>( m_part ) );
        htmlPart->begin( url() );
    }
    else
        m_tempFile = new KTempFile;
}

void KMultiPart::sendData( const QCString& line )
{
    if ( m_isHTMLPart )
    {
        KHTMLPart* htmlPart = static_cast<KHTMLPart *>( static_cast<KParts::ReadOnlyPart *>( m_part ) );
        htmlPart->write(  line.data(), line.size() );
    }
    else
        m_tempFile->file()->writeBlock( line.data(), line.size() );
}

void KMultiPart::endOfData()
{
    Q_ASSERT( m_part );
    if ( m_isHTMLPart )
    {
        KHTMLPart* htmlPart = static_cast<KHTMLPart *>( static_cast<KParts::ReadOnlyPart *>( m_part ) );
        htmlPart->end();
    } else
    {
        Q_ASSERT( m_tempFile );
	m_tempFile->close();
        kdDebug() << "KMultiPart::endOfData opening " << m_tempFile->name() << endl;
        KURL url;
        url.setPath( m_tempFile->name() );
        (void) m_part->openURL( url );
        delete m_tempFile;
        m_tempFile = 0L;
    }
}

void KMultiPart::slotPartCompleted()
{
    if ( !m_isHTMLPart )
    {
        Q_ASSERT( m_part );
        // Delete temp file used by the part
        Q_ASSERT( m_part->url().isLocalFile() );
        (void) unlink( QFile::encodeName( m_part->url().path() ) );
        // Do not emit completed from here.
    }
}

bool KMultiPart::closeURL()
{
    if ( m_part )
        return m_part->closeURL();
    return true;
}

void KMultiPart::guiActivateEvent( KParts::GUIActivateEvent * )
{
    // Not public!
    //if ( m_part )
    //    m_part->guiActivateEvent( e );
}

void KMultiPart::slotJobFinished( KIO::Job *job )
{
    if ( job->error() )
    {
        // TODO use khtml's error:// scheme
        job->showErrorDialog();
        emit canceled( job->errorString() );
    }
    else
    {
        /*if ( m_khtml->view()->contentsY() == 0 )
        {
            KParts::URLArgs args = m_ext->urlArgs();
            m_khtml->view()->setContentsPos( args.xOffset, args.yOffset );
        }*/

        emit completed();

        //QTimer::singleShot( 0, this, SLOT( updateWindowCaption() ) );
    }
    m_job = 0L;
}

KAboutData* KMultiPart::createAboutData()
{
    KAboutData* aboutData = new KAboutData( "kmultipart", I18N_NOOP("KMultiPart"),
                                             "0.1",
                                            I18N_NOOP( "Embeddable component for multipart/mixed" ),
                                             KAboutData::License_GPL,
                                             "(c) 2001, David Faure <david@mandrakesoft.com>");
    return aboutData;
}

#if 0
KMultiPartBrowserExtension::KMultiPartBrowserExtension( KMultiPart *parent, const char *name )
    : KParts::BrowserExtension( parent, name )
{
    m_imgPart = parent;
}

int KMultiPartBrowserExtension::xOffset()
{
    return m_imgPart->doc()->view()->contentsX();
}

int KMultiPartBrowserExtension::yOffset()
{
    return m_imgPart->doc()->view()->contentsY();
}

void KMultiPartBrowserExtension::print()
{
    static_cast<KHTMLPartBrowserExtension *>( m_imgPart->doc()->browserExtension() )->print();
}

void KMultiPartBrowserExtension::reparseConfiguration()
{
    static_cast<KHTMLPartBrowserExtension *>( m_imgPart->doc()->browserExtension() )->reparseConfiguration();
    m_imgPart->doc()->setAutoloadImages( true );
}
#endif

#include "kmultipart.moc"
