
#include "khtml_ext.h"
#include "khtmlview.h"

#include <qapplication.h>
#include <qclipboard.h>

#include <kdebug.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kio/job.h>
#include <ktoolbarbutton.h>
#include <ktoolbar.h>

#include <dom/dom_element.h>
#include <misc/htmltags.h>

KHTMLPartBrowserExtension::KHTMLPartBrowserExtension( KHTMLPart *parent, const char *name )
: KParts::BrowserExtension( parent, name )
{
  m_part = parent;
}

int KHTMLPartBrowserExtension::xOffset()
{
  return m_part->view()->contentsX();
}

int KHTMLPartBrowserExtension::yOffset()
{
  return m_part->view()->contentsY();
}

void KHTMLPartBrowserExtension::saveState( QDataStream &stream )
{
  kdDebug( 6050 ) << "saveState!" << endl;
  m_part->saveState( stream );
}

void KHTMLPartBrowserExtension::restoreState( QDataStream &stream )
{
  kdDebug( 6050 ) << "restoreState!" << endl;
  m_part->restoreState( stream );
}

void KHTMLPartBrowserExtension::copy()
{
  // get selected text and paste to the clipboard
  QString text = m_part->selectedText();
  QClipboard *cb = QApplication::clipboard();
  cb->setText(text);
}

void KHTMLPartBrowserExtension::reparseConfiguration()
{
  m_part->reparseConfiguration();
}

void KHTMLPartBrowserExtension::print()
{
  m_part->view()->print();
}

class KHTMLPopupGUIClient::KHTMLPopupGUIClientPrivate
{
public:
  KHTMLPart *m_khtml;
  KURL m_url;
  KURL m_imageURL;
  KAction *m_paPrintFrame;
  KAction *m_paSaveLinkAs;
  KAction *m_paSaveImageAs;
  KAction *m_paCopyLinkLocation;
  KAction *m_paCopyImageLocation;
  KAction *m_paReloadFrame;
  KAction *m_paViewFrameSource;
};


KHTMLPopupGUIClient::KHTMLPopupGUIClient( KHTMLPart *khtml, const QString &doc, const KURL &url )
{
  d = new KHTMLPopupGUIClientPrivate;
  d->m_khtml = khtml;
  d->m_url = url;

  setInstance( khtml->instance() );

  actionCollection()->insert( khtml->actionCollection()->action( "selectAll" ) );

  // frameset? -> add "Reload Frame"
  if ( khtml->parentPart() )
  {
    d->m_paReloadFrame = new KAction( i18n( "Reload Frame" ), 0, this, SLOT( slotReloadFrame() ),
				      actionCollection(), "reloadframe" );
    d->m_paPrintFrame = new KAction( i18n( "Print Frame..." ), "fileprint", 0, d->m_khtml->browserExtension(), SLOT( print() ), actionCollection(), "printFrame" );
    d->m_paViewFrameSource = new KAction( i18n( "View Frame Source" ), 0, d->m_khtml, SLOT( slotViewDocumentSource() ),
					  actionCollection(), "viewFrameSource" );
  }

  if ( !url.isEmpty() )
  {
    d->m_paSaveLinkAs = new KAction( i18n( "&Save Link As ..." ), 0, this, SLOT( slotSaveLinkAs() ),
 				     actionCollection(), "savelinkas" );
    d->m_paCopyLinkLocation = new KAction( i18n( "Copy Link Location" ), 0, this, SLOT( slotCopyLinkLocation() ),
					   actionCollection(), "copylinklocation" );
  }

  DOM::Element e;
  e = khtml->nodeUnderMouse();

  if ( !e.isNull() && e.elementId() == ID_IMG )
  {
    d->m_imageURL = KURL( d->m_khtml->url(), e.getAttribute( "src" ).string() );
    d->m_paSaveImageAs = new KAction( i18n( "Save Image As ..." ), 0, this, SLOT( slotSaveImageAs() ),
				      actionCollection(), "saveimageas" );
    d->m_paCopyImageLocation = new KAction( i18n( "Copy Image Location" ), 0, this, SLOT( slotCopyImageLocation() ),
					    actionCollection(), "copyimagelocation" );
  }

  setXML( doc );
  setDOMDocument( QDomDocument(), true ); // ### HACK

  QDomElement menu = domDocument().documentElement().namedItem( "Menu" ).toElement();

  if ( actionCollection()->count() > 0 )
    menu.insertBefore( domDocument().createElement( "separator" ), menu.firstChild() );
}

KHTMLPopupGUIClient::~KHTMLPopupGUIClient()
{
  delete d;
}

void KHTMLPopupGUIClient::slotSaveLinkAs()
{
  if ( d->m_url.fileName( false ).isEmpty() )
    d->m_url.setFileName( "index.html" );

  saveURL( d->m_khtml->widget(), i18n( "Save Link As" ), d->m_url );
}

void KHTMLPopupGUIClient::slotSaveImageAs()
{
  saveURL( d->m_khtml->widget(), i18n( "Save Image As" ), d->m_imageURL );
}

void KHTMLPopupGUIClient::slotCopyLinkLocation()
{
  QApplication::clipboard()->setText( d->m_url.url() );
}

void KHTMLPopupGUIClient::slotCopyImageLocation()
{
  QApplication::clipboard()->setText( d->m_imageURL.url() );
}

void KHTMLPopupGUIClient::slotReloadFrame()
{
  KParts::URLArgs args( d->m_khtml->browserExtension()->urlArgs() );
  args.reload = true;
  // reload document
  d->m_khtml->closeURL();
  d->m_khtml->browserExtension()->setURLArgs( args );
  d->m_khtml->openURL( d->m_khtml->url() );
}

void KHTMLPopupGUIClient::saveURL( QWidget *parent, const QString &caption, const KURL &url )
{
  KFileDialog *dlg = new KFileDialog( QString::null, QString::null, parent, "filedia", true );

  dlg->setCaption( caption );

  dlg->setSelection( url.fileName() );

  if ( dlg->exec() )
  {
    KURL destURL( dlg->selectedURL() );
    if ( !destURL.isMalformed() )
    {
      /*KIO::Job *job = */ KIO::copy( url, destURL );
      // TODO connect job result, to display errors
    }
  }

  delete dlg;
}

KHTMLPartBrowserHostExtension::KHTMLPartBrowserHostExtension( KHTMLPart *part )
: KParts::BrowserHostExtension( part )
{
  m_part = part;
}

KHTMLPartBrowserHostExtension::~KHTMLPartBrowserHostExtension()
{
}

QStringList KHTMLPartBrowserHostExtension::frameNames() const
{
  return m_part->frameNames();
}

const QList<KParts::ReadOnlyPart> KHTMLPartBrowserHostExtension::frames() const
{
  return m_part->frames();
}

bool KHTMLPartBrowserHostExtension::openURLInFrame( const KURL &url, const KParts::URLArgs &urlArgs )
{
  return m_part->openURLInFrame( url, urlArgs );
}

KHTMLFontSizeAction::KHTMLFontSizeAction( KHTMLPart *part, bool direction, const QString &text, const QString &icon, const QObject *receiver, const char *slot, QObject *parent, const char *name )
    : KAction( text, icon, 0, receiver, slot, parent, name )
{
    m_direction = direction;
    m_part = part;

    m_popup = new QPopupMenu;
    m_popup->insertItem( i18n( "Default font size" ) );

    int m = m_direction ? 1 : -1;

    for ( int i = 1; i < 5; ++i )
    {
	int num = i * m;
	QString numStr = QString::number( num );
	if ( num > 0 ) numStr.prepend( '+' );
	
	m_popup->insertItem( i18n( "Font Size %1" ).arg( numStr ) );
    }

    connect( m_popup, SIGNAL( activated( int ) ), this, SLOT( slotActivated( int ) ) );
}

KHTMLFontSizeAction::~KHTMLFontSizeAction()
{
    delete m_popup;
}

int KHTMLFontSizeAction::plug( QWidget *w, int index )
{
    int containerId = KAction::plug( w, index );
    if ( containerId == -1 || !w->inherits( "KToolBar" ) )
	return containerId;

    KToolBarButton *button = static_cast<KToolBar *>( w )->getButton( menuId( containerId ) );
    if ( !button )
	return containerId;

    button->setDelayedPopup( m_popup );
    return containerId;
}

void KHTMLFontSizeAction::slotActivated( int id )
{
    int idx = m_popup->indexOf( id );

    if ( idx == 0 )
	m_part->setFontBaseInternal( 0, true );
    else
	m_part->setFontBaseInternal( idx * ( m_direction ? 1 : -1 ), false );
}

#include "khtml_ext.moc"
