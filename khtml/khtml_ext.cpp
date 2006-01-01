// -*- c-basic-offset: 2 -*-
/* This file is part of the KDE project
 *
 * Copyright (C) 2000-2003 Simon Hausmann <hausmann@kde.org>
 *               2001-2003 George Staikos <staikos@kde.org>
 *               2001-2003 Laurent Montel <montel@kde.org>
 *               2001-2003 Dirk Mueller <mueller@kde.org>
 *               2001-2003 Waldo Bastian <bastian@kde.org>
 *               2001-2003 David Faure <faure@kde.org>
 *               2001-2003 Daniel Naber <dnaber@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <assert.h>
#include "khtml_ext.h"
#include "khtmlview.h"
#include "khtml_pagecache.h"
#include "rendering/render_form.h"
#include "rendering/render_image.h"
#include "html/html_imageimpl.h"
#include "misc/loader.h"
#include "dom/html_form.h"
#include "dom/html_image.h"
#include <qclipboard.h>
#include <qfileinfo.h>
#include <qmenu.h>
#include <qmetaobject.h>

#include <kdebug.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kio/job.h>
#include <kprocess.h>
#include <ktoolbarbutton.h>
#include <ktoolbar.h>
#include <ksavefile.h>
#include <kstringhandler.h>
#include <ktoolinvocation.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <krun.h>
#include <kurifilter.h>
#include <kiconloader.h>
#include <kdesktopfile.h>
#include <kinputdialog.h>
#include <ktempfile.h>
#include "khtml_factory.h"

#include "dom/dom_element.h"
#include "misc/htmltags.h"

#include "khtmlpart_p.h"

KHTMLPartBrowserExtension::KHTMLPartBrowserExtension( KHTMLPart *parent )
: KParts::BrowserExtension( parent )
{
    m_part = parent;
    setURLDropHandlingEnabled( true );

    enableAction( "cut", false );
    enableAction( "copy", false );
    enableAction( "paste", false );

    m_connectedToClipboard = false;
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
  //kdDebug( 6050 ) << "saveState!" << endl;
  m_part->saveState( stream );
}

void KHTMLPartBrowserExtension::restoreState( QDataStream &stream )
{
  //kdDebug( 6050 ) << "restoreState!" << endl;
  m_part->restoreState( stream );
}

void KHTMLPartBrowserExtension::editableWidgetFocused( QWidget *widget )
{
    m_editableFormWidget = widget;
    updateEditActions();

    if ( !m_connectedToClipboard && m_editableFormWidget )
    {
        connect( QApplication::clipboard(), SIGNAL( dataChanged() ),
                 this, SLOT( updateEditActions() ) );

        if ( m_editableFormWidget->inherits( "QLineEdit" ) || m_editableFormWidget->inherits( "QTextEdit" ) )
            connect( m_editableFormWidget, SIGNAL( selectionChanged() ),
                     this, SLOT( updateEditActions() ) );

        m_connectedToClipboard = true;
    }
    editableWidgetFocused();
}

void KHTMLPartBrowserExtension::editableWidgetBlurred( QWidget * /*widget*/ )
{
    QWidget *oldWidget = m_editableFormWidget;

    m_editableFormWidget = 0;
    enableAction( "cut", false );
    enableAction( "paste", false );
    m_part->emitSelectionChanged();

    if ( m_connectedToClipboard )
    {
        disconnect( QApplication::clipboard(), SIGNAL( dataChanged() ),
                    this, SLOT( updateEditActions() ) );

        if ( oldWidget )
        {
            if ( oldWidget->inherits( "QLineEdit" ) || oldWidget->inherits( "QTextEdit" ) )
                disconnect( oldWidget, SIGNAL( selectionChanged() ),
                            this, SLOT( updateEditActions() ) );
        }

        m_connectedToClipboard = false;
    }
    editableWidgetBlurred();
}

void KHTMLPartBrowserExtension::setExtensionProxy( KParts::BrowserExtension *proxy )
{
    if ( m_extensionProxy )
    {
        disconnect( m_extensionProxy, SIGNAL( enableAction( const char *, bool ) ),
                    this, SLOT( extensionProxyActionEnabled( const char *, bool ) ) );
        if ( m_extensionProxy->inherits( "KHTMLPartBrowserExtension" ) )
        {
            disconnect( m_extensionProxy, SIGNAL( editableWidgetFocused() ),
                        this, SLOT( extensionProxyEditableWidgetFocused() ) );
            disconnect( m_extensionProxy, SIGNAL( editableWidgetBlurred() ),
                        this, SLOT( extensionProxyEditableWidgetBlurred() ) );
        }
    }

    m_extensionProxy = proxy;

    if ( m_extensionProxy )
    {
        connect( m_extensionProxy, SIGNAL( enableAction( const char *, bool ) ),
                 this, SLOT( extensionProxyActionEnabled( const char *, bool ) ) );
        if ( m_extensionProxy->inherits( "KHTMLPartBrowserExtension" ) )
        {
            connect( m_extensionProxy, SIGNAL( editableWidgetFocused() ),
                     this, SLOT( extensionProxyEditableWidgetFocused() ) );
            connect( m_extensionProxy, SIGNAL( editableWidgetBlurred() ),
                     this, SLOT( extensionProxyEditableWidgetBlurred() ) );
        }

        enableAction( "cut", m_extensionProxy->isActionEnabled( "cut" ) );
        enableAction( "copy", m_extensionProxy->isActionEnabled( "copy" ) );
        enableAction( "paste", m_extensionProxy->isActionEnabled( "paste" ) );
    }
    else
    {
        updateEditActions();
        enableAction( "copy", false ); // ### re-check this
    }
}

void KHTMLPartBrowserExtension::cut()
{
    if ( m_extensionProxy )
    {
        callExtensionProxyMethod( "cut()" );
        return;
    }

    if ( !m_editableFormWidget )
        return;

    if ( m_editableFormWidget->inherits( "QLineEdit" ) )
        static_cast<QLineEdit *>( &(*m_editableFormWidget) )->cut();
    else if ( m_editableFormWidget->inherits( "QTextEdit" ) )
        static_cast<Q3TextEdit *>( &(*m_editableFormWidget) )->cut();
}

void KHTMLPartBrowserExtension::copy()
{
    if ( m_extensionProxy )
    {
        callExtensionProxyMethod( "copy()" );
        return;
    }

    kdDebug( 6050 ) << "************! KHTMLPartBrowserExtension::copy()" << endl;
    if ( !m_editableFormWidget )
    {
        // get selected text and paste to the clipboard
        QString text= m_part->selectedText();
	text.replace( QChar( 0xa0 ), ' ' );


        QClipboard *cb = QApplication::clipboard();
        disconnect( cb, SIGNAL( selectionChanged() ), m_part, SLOT( slotClearSelection() ) );
#ifndef QT_NO_MIMECLIPBOARD
	QString htmltext;
	/*
	 * When selectionModeEnabled, that means the user has just selected
	 * the text, not ctrl+c to copy it.  The selection clipboard
	 * doesn't seem to support mime type, so to save time, don't calculate
	 * the selected text as html.
	 * optomisation disabled for now until everything else works.
	*/
	//if(!cb->selectionModeEnabled())
	    htmltext = m_part->selectedTextAsHTML();
	QMimeData *mimeData = new QMimeData;
	mimeData->setText(text);
	if(!htmltext.isEmpty()) {
	    htmltext.replace( QChar( 0xa0 ), ' ' );
	    mimeData->setHtml(htmltext);
	}
        cb->setMimeData(mimeData);
#else
	cb->setText(text);
#endif

        connect( cb, SIGNAL( selectionChanged() ), m_part, SLOT( slotClearSelection() ) );
    }
    else
    {
        if ( m_editableFormWidget->inherits( "QLineEdit" ) )
            static_cast<QLineEdit *>( &(*m_editableFormWidget) )->copy();
        else if ( m_editableFormWidget->inherits( "QTextEdit" ) )
            static_cast<Q3TextEdit *>( &(*m_editableFormWidget) )->copy();
    }
}

void KHTMLPartBrowserExtension::searchProvider()
{
    // action name is of form "previewProvider[<searchproviderprefix>:]"
    const QString searchProviderPrefix = QString( sender()->name() ).mid( 14 );

    KURIFilterData data;
    QStringList list;
    data.setData( searchProviderPrefix + m_part->selectedText() );
    list << "kurisearchfilter" << "kuriikwsfilter";

    if( !KURIFilter::self()->filterURI(data, list) )
    {
        KDesktopFile file("searchproviders/google.desktop", true, "services");
        data.setData(file.readEntry("Query").replace("\\{@}", m_part->selectedText()));
    }

    KParts::URLArgs args;
    args.frameName = "_blank";

    emit m_part->browserExtension()->openURLRequest( data.uri(), args );
}

void KHTMLPartBrowserExtension::openSelection()
{
    KParts::URLArgs args;
    args.frameName = "_blank";

    emit m_part->browserExtension()->openURLRequest( m_part->selectedText(), args );
}

void KHTMLPartBrowserExtension::paste()
{
    if ( m_extensionProxy )
    {
        callExtensionProxyMethod( "paste()" );
        return;
    }

    if ( !m_editableFormWidget )
        return;

    if ( m_editableFormWidget->inherits( "QLineEdit" ) )
        static_cast<QLineEdit *>( &(*m_editableFormWidget) )->paste();
    else if ( m_editableFormWidget->inherits( "QTextEdit" ) )
        static_cast<Q3TextEdit *>( &(*m_editableFormWidget) )->paste();
}

void KHTMLPartBrowserExtension::callExtensionProxyMethod( const char *method )
{
    if ( !m_extensionProxy )
        return;

    int slot = m_extensionProxy->metaObject()->indexOfSlot( method );
    if ( slot == -1 )
        return;

    QMetaObject::invokeMethod(m_extensionProxy, method, Qt::DirectConnection);
}

void KHTMLPartBrowserExtension::updateEditActions()
{
    if ( !m_editableFormWidget )
    {
        enableAction( "cut", false );
        enableAction( "copy", false );
        enableAction( "paste", false );
        return;
    }

    // ### duplicated from KonqMainWindow::slotClipboardDataChanged
#ifndef QT_NO_MIMECLIPBOARD // Handle minimalized versions of Qt Embedded
    QMimeSource *data = QApplication::clipboard()->data();
    enableAction( "paste", data->provides( "text/plain" ) );
#else
    QString data=QApplication::clipboard()->text();
    enableAction( "paste", data.contains("://"));
#endif
    bool hasSelection = false;

    if( m_editableFormWidget) {
        if ( qobject_cast<QLineEdit*>(m_editableFormWidget))
            hasSelection = static_cast<QLineEdit *>( &(*m_editableFormWidget) )->hasSelectedText();
        else if(qobject_cast<Q3TextEdit*>(m_editableFormWidget))
            hasSelection = static_cast<Q3TextEdit *>( &(*m_editableFormWidget) )->hasSelectedText();
    }

    enableAction( "copy", hasSelection );
    enableAction( "cut", hasSelection );
}

void KHTMLPartBrowserExtension::extensionProxyEditableWidgetFocused() {
	editableWidgetFocused();
}

void KHTMLPartBrowserExtension::extensionProxyEditableWidgetBlurred() {
	editableWidgetBlurred();
}

void KHTMLPartBrowserExtension::extensionProxyActionEnabled( const char *action, bool enable )
{
    // only forward enableAction calls for actions we actually do forward
    if ( strcmp( action, "cut" ) == 0 ||
         strcmp( action, "copy" ) == 0 ||
         strcmp( action, "paste" ) == 0 ) {
        enableAction( action, enable );
    }
}

void KHTMLPartBrowserExtension::reparseConfiguration()
{
  m_part->reparseConfiguration();
}

void KHTMLPartBrowserExtension::print()
{
  m_part->view()->print();
}

void KHTMLPartBrowserExtension::disableScrolling()
{
  Q3ScrollView *scrollView = m_part->view();
  if (scrollView) {
    scrollView->setVScrollBarMode(Q3ScrollView::AlwaysOff);
    scrollView->setHScrollBarMode(Q3ScrollView::AlwaysOff);
  }
}

class KHTMLPopupGUIClient::KHTMLPopupGUIClientPrivate
{
public:
  KHTMLPart *m_khtml;
  KURL m_url;
  KURL m_imageURL;
  QPixmap m_pixmap;
  QString m_suggestedFilename;
};


KHTMLPopupGUIClient::KHTMLPopupGUIClient( KHTMLPart *khtml, const QString &doc, const KURL &url )
  : QObject( khtml )
{
  d = new KHTMLPopupGUIClientPrivate;
  d->m_khtml = khtml;
  d->m_url = url;
  bool isImage = false;
  bool hasSelection = khtml->hasSelection();
  setInstance( khtml->instance() );

  DOM::Element e;
  e = khtml->nodeUnderMouse();

  if ( !e.isNull() && (e.elementId() == ID_IMG ||
                       (e.elementId() == ID_INPUT && !static_cast<DOM::HTMLInputElement>(e).src().isEmpty())))
  {
    if (e.elementId() == ID_IMG) {
      DOM::HTMLImageElementImpl *ie = static_cast<DOM::HTMLImageElementImpl*>(e.handle());
      khtml::RenderImage *ri = dynamic_cast<khtml::RenderImage*>(ie->renderer());
      if (ri && ri->contentObject()) {
        d->m_suggestedFilename = static_cast<khtml::CachedImage*>(ri->contentObject())->suggestedFilename();
      }
    }
    isImage=true;
  }

  if (hasSelection)
  {
      KAction* copyAction = KStdAction::copy( d->m_khtml->browserExtension(), SLOT( copy() ), actionCollection(), "copy" );
      copyAction->setText(i18n("&Copy Text"));
      copyAction->setEnabled(d->m_khtml->browserExtension()->isActionEnabled( "copy" ));
      actionCollection()->insert( khtml->actionCollection()->action( "selectAll" ) );


      // Fill search provider entries
      KConfig config("kuriikwsfilterrc");
      config.setGroup("General");
      const QString defaultEngine = config.readEntry("DefaultSearchEngine", "google");
      const char keywordDelimiter = config.readEntry("KeywordDelimiter", QVariant(':')).toInt();

      // search text
      QString selectedText = khtml->selectedText();
      if ( selectedText.length()>18 ) {
        selectedText.truncate(15);
        selectedText+="...";
      }

      // default search provider
      KService::Ptr service = KService::serviceByDesktopPath(QString("searchproviders/%1.desktop").arg(defaultEngine));

      // search provider icon
      QPixmap icon;
      KURIFilterData data;
      QStringList list;
      data.setData( QString("some keyword") );
      list << "kurisearchfilter" << "kuriikwsfilter";

      QString name;
      if ( KURIFilter::self()->filterURI(data, list) )
      {
        QString iconPath = locate("cache", KMimeType::favIconForURL(data.uri()) + ".png");
        if ( iconPath.isEmpty() )
          icon = SmallIcon("find");
        else
          icon = QPixmap( iconPath );
        name = service->name();
      }
      else
      {
        icon = SmallIcon("google");
        name = "Google";
      }

      new KAction( i18n( "Search '%1' at %2" ).arg( selectedText ).arg( name ), icon, 0, d->m_khtml->browserExtension(),
                     SLOT( searchProvider() ), actionCollection(), "searchProvider" );

      // favorite search providers
      QStringList favoriteEngines;
      favoriteEngines << "google" << "google_groups" << "google_news" << "webster" << "dmoz" << "wikipedia";
      favoriteEngines = config.readListEntry("FavoriteSearchEngines", favoriteEngines);

      if ( !favoriteEngines.isEmpty()) {
        KActionMenu* providerList = new KActionMenu( i18n( "Search '%1' At" ).arg( selectedText ), actionCollection(), "searchProviderList" );

        QStringList::ConstIterator it = favoriteEngines.begin();
        for ( ; it != favoriteEngines.end(); ++it ) {
          if (*it==defaultEngine)
            continue;
          service = KService::serviceByDesktopPath(QString("searchproviders/%1.desktop").arg(*it));
          if (!service)
            continue;
          const QString searchProviderPrefix = *(service->property("Keys").toStringList().begin()) + keywordDelimiter;
          data.setData( searchProviderPrefix + "some keyword" );

          if ( KURIFilter::self()->filterURI(data, list) )
          {
            QString iconPath = locate("cache", KMimeType::favIconForURL(data.uri()) + ".png");
            if ( iconPath.isEmpty() )
              icon = SmallIcon("find");
            else
              icon = QPixmap( iconPath );
            name = service->name();

            providerList->insert( new KAction( name, icon, 0, d->m_khtml->browserExtension(),
              SLOT( searchProvider() ), actionCollection(), QString( "searchProvider" + searchProviderPrefix ).latin1() ) );
          }
        }
      }


      if ( selectedText.contains("://") && KURL(selectedText).isValid() )
         new KAction( i18n( "Open '%1'" ).arg( selectedText ), "window_new", 0,
         d->m_khtml->browserExtension(), SLOT( openSelection() ), actionCollection(), "openSelection" );
  }
  else if ( url.isEmpty() && !isImage )
  {
      actionCollection()->insert( khtml->actionCollection()->action( "security" ) );
      actionCollection()->insert( khtml->actionCollection()->action( "setEncoding" ) );
      new KAction( i18n( "Stop Animations" ), 0, this, SLOT( slotStopAnimations() ),
                   actionCollection(), "stopanimations" );
  }

  if ( !url.isEmpty() )
  {
    if (url.protocol() == "mailto")
    {
      new KAction( i18n( "Copy Email Address" ), 0, this, SLOT( slotCopyLinkLocation() ),
                 actionCollection(), "copylinklocation" );
    }
    else
    {
      new KAction( i18n( "&Save Link As..." ), 0, this, SLOT( slotSaveLinkAs() ),
                 actionCollection(), "savelinkas" );
      new KAction( i18n( "Copy &Link Address" ), 0, this, SLOT( slotCopyLinkLocation() ),
                 actionCollection(), "copylinklocation" );
    }
  }

  // frameset? -> add "Reload Frame" etc.
  if (!hasSelection)
  {
    if ( khtml->parentPart() )
    {
      new KAction( i18n( "Open in New &Window" ), "window_new", 0, this, SLOT( slotFrameInWindow() ),
                                          actionCollection(), "frameinwindow" );
      new KAction( i18n( "Open in &This Window" ), 0, this, SLOT( slotFrameInTop() ),
                                          actionCollection(), "frameintop" );
      new KAction( i18n( "Open in &New Tab" ), "tab_new", 0, this, SLOT( slotFrameInTab() ),
                                       actionCollection(), "frameintab" );
      new KAction( i18n( "Reload Frame" ), 0, this, SLOT( slotReloadFrame() ),
                                        actionCollection(), "reloadframe" );

      if ( KHTMLFactory::defaultHTMLSettings()->isAdFilterEnabled() ) {
          if ( khtml->d->m_frame->m_type == khtml::ChildFrame::IFrame )
              new KAction( i18n( "Block IFrame..." ), 0, this, SLOT( slotBlockIFrame() ), actionCollection(), "blockiframe" );
      }

      new KAction( i18n( "View Frame Source" ), 0, d->m_khtml, SLOT( slotViewDocumentSource() ),
                                          actionCollection(), "viewFrameSource" );
      new KAction( i18n( "View Frame Information" ), 0, d->m_khtml, SLOT( slotViewPageInfo() ), actionCollection(), "viewFrameInfo" );
      // This one isn't in khtml_popupmenu.rc anymore, because Print isn't either,
      // and because print frame is already in the toolbar and the menu.
      // But leave this here, so that it's easy to read it.
      new KAction( i18n( "Print Frame..." ), "frameprint", 0, d->m_khtml->browserExtension(), SLOT( print() ), actionCollection(), "printFrame" );
      new KAction( i18n( "Save &Frame As..." ), 0, d->m_khtml, SLOT( slotSaveFrame() ), actionCollection(), "saveFrame" );

      actionCollection()->insert( khtml->parentPart()->actionCollection()->action( "viewDocumentSource" ) );
      actionCollection()->insert( khtml->parentPart()->actionCollection()->action( "viewPageInfo" ) );
    } else {
      actionCollection()->insert( khtml->actionCollection()->action( "viewDocumentSource" ) );
      actionCollection()->insert( khtml->actionCollection()->action( "viewPageInfo" ) );
    }
  } else if (isImage || !url.isEmpty()) {
    actionCollection()->insert( khtml->actionCollection()->action( "viewDocumentSource" ) );
    actionCollection()->insert( khtml->actionCollection()->action( "viewPageInfo" ) );
    new KAction( i18n( "Stop Animations" ), 0, this, SLOT( slotStopAnimations() ),
                 actionCollection(), "stopanimations" );
  }

  if (isImage)
  {
    if ( e.elementId() == ID_IMG ) {
      d->m_imageURL = KURL( static_cast<DOM::HTMLImageElement>( e ).src().string() );
      DOM::HTMLImageElementImpl *imageimpl = static_cast<DOM::HTMLImageElementImpl *>( e.handle() );
      Q_ASSERT(imageimpl);
      if(imageimpl) // should be true always.  right?
      {
        if(imageimpl->complete()) {
	  d->m_pixmap = imageimpl->currentPixmap();
	}
      }
    }
    else
      d->m_imageURL = KURL( static_cast<DOM::HTMLInputElement>( e ).src().string() );
    new KAction( i18n( "Save Image As..." ), 0, this, SLOT( slotSaveImageAs() ),
                 actionCollection(), "saveimageas" );
    new KAction( i18n( "Send Image..." ), 0, this, SLOT( slotSendImage() ),
                 actionCollection(), "sendimage" );


#ifndef QT_NO_MIMECLIPBOARD
    (new KAction( i18n( "Copy Image" ), 0, this, SLOT( slotCopyImage() ),
                 actionCollection(), "copyimage" ))->setEnabled(!d->m_pixmap.isNull());
#endif

    if(d->m_pixmap.isNull()) {    //fallback to image location if still loading the image.  this will always be true if ifdef QT_NO_MIMECLIPBOARD
      new KAction( i18n( "Copy Image Location" ), 0, this, SLOT( slotCopyImageLocation() ),
                   actionCollection(), "copyimagelocation" );
    }

    QString name = KStringHandler::csqueeze(d->m_imageURL.fileName()+d->m_imageURL.query(), 25);
    new KAction( i18n( "View Image (%1)" ).arg(d->m_suggestedFilename.isEmpty() ? name.replace("&", "&&") : d->m_suggestedFilename.replace("&", "&&")), 0, this, SLOT( slotViewImage() ),
                 actionCollection(), "viewimage" );

    if (KHTMLFactory::defaultHTMLSettings()->isAdFilterEnabled())
    {
      new KAction( i18n( "Block Image..." ), 0, this, SLOT( slotBlockImage() ),
                   actionCollection(), "blockimage" );

      if (!d->m_imageURL.host().isEmpty() &&
          !d->m_imageURL.protocol().isEmpty())
      {
        new KAction( i18n( "Block Images From %1" ).arg(d->m_imageURL.host()), 0, this, SLOT( slotBlockHost() ),
                     actionCollection(), "blockhost" );
      }
    }
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
  KIO::MetaData metaData;
  metaData["referrer"] = d->m_khtml->referrer();
  saveURL( d->m_khtml->widget(), i18n( "Save Link As" ), d->m_url, metaData );
}

void KHTMLPopupGUIClient::slotSendImage()
{
    QStringList urls;
    urls.append( d->m_imageURL.url());
    QString subject = d->m_imageURL.url();
    KToolInvocation::invokeMailer(QString(), QString(), QString(), subject,
                       QString(), //body
                       QString(),
                       urls); // attachments


}

void KHTMLPopupGUIClient::slotSaveImageAs()
{
  KIO::MetaData metaData;
  metaData["referrer"] = d->m_khtml->referrer();
  saveURL( d->m_khtml->widget(), i18n( "Save Image As" ), d->m_imageURL, metaData, QString(), 0, d->m_suggestedFilename );
}

void KHTMLPopupGUIClient::slotBlockHost()
{
    QString name=d->m_imageURL.protocol()+"://"+d->m_imageURL.host()+"/*";
    KHTMLFactory::defaultHTMLSettings()->addAdFilter( name );
    d->m_khtml->reparseConfiguration();
}

void KHTMLPopupGUIClient::slotBlockImage()
{
    bool ok = false;

    QString url = KInputDialog::getText( i18n("Add URL to Filter"),
                                         "Enter the URL:",
                                         d->m_imageURL.url(),
                                         &ok);
    if ( ok ) {
        KHTMLFactory::defaultHTMLSettings()->addAdFilter( url );
        d->m_khtml->reparseConfiguration();
    }
}

void KHTMLPopupGUIClient::slotBlockIFrame()
{
    bool ok = false;
    QString url = KInputDialog::getText( i18n( "Add URL to Filter"),
                                               "Enter the URL:",
                                               d->m_khtml->url().url(),
                                               &ok );
    if ( ok ) {
        KHTMLFactory::defaultHTMLSettings()->addAdFilter( url );
        d->m_khtml->reparseConfiguration();
    }
}

void KHTMLPopupGUIClient::slotCopyLinkLocation()
{
  KURL safeURL(d->m_url);
  safeURL.setPass(QString());
#ifndef QT_NO_MIMECLIPBOARD
  // Set it in both the mouse selection and in the clipboard
  QMimeData* mimeData = new QMimeData;
  safeURL.populateMimeData( mimeData );
  QApplication::clipboard()->setMimeData( mimeData, QClipboard::Clipboard );

  mimeData = new QMimeData;
  safeURL.populateMimeData( mimeData );
  QApplication::clipboard()->setMimeData( mimeData, QClipboard::Selection );

#else
  QApplication::clipboard()->setText( safeURL.url() ); //FIXME(E): Handle multiple entries
#endif
}

void KHTMLPopupGUIClient::slotStopAnimations()
{
  d->m_khtml->stopAnimations();
}

void KHTMLPopupGUIClient::slotCopyImage()
{
#ifndef QT_NO_MIMECLIPBOARD
  KURL safeURL(d->m_imageURL);
  safeURL.setPass(QString());

  // Set it in both the mouse selection and in the clipboard
  QMimeData* mimeData = new QMimeData;
  mimeData->setImageData( d->m_pixmap );
  safeURL.populateMimeData( mimeData );
  QApplication::clipboard()->setMimeData( mimeData, QClipboard::Clipboard );

  mimeData = new QMimeData;
  mimeData->setImageData( d->m_pixmap );
  safeURL.populateMimeData( mimeData );
  QApplication::clipboard()->setMimeData( mimeData, QClipboard::Selection );
#else
  kdDebug() << "slotCopyImage called when the clipboard does not support this.  This should not be possible." << endl;
#endif
}

void KHTMLPopupGUIClient::slotCopyImageLocation()
{
  KURL safeURL(d->m_imageURL);
  safeURL.setPass(QString());
#ifndef QT_NO_MIMECLIPBOARD
  // Set it in both the mouse selection and in the clipboard
  QMimeData* mimeData = new QMimeData;
  safeURL.populateMimeData( mimeData );
  QApplication::clipboard()->setMimeData( mimeData, QClipboard::Clipboard );
  mimeData = new QMimeData;
  safeURL.populateMimeData( mimeData );
  QApplication::clipboard()->setMimeData( mimeData, QClipboard::Selection );
#else
  QApplication::clipboard()->setText( safeURL.url() ); //FIXME(E): Handle multiple entries
#endif
}

void KHTMLPopupGUIClient::slotViewImage()
{
  d->m_khtml->browserExtension()->createNewWindow(d->m_imageURL);
}

void KHTMLPopupGUIClient::slotReloadFrame()
{
  KParts::URLArgs args( d->m_khtml->browserExtension()->urlArgs() );
  args.reload = true;
  args.metaData()["referrer"] = d->m_khtml->pageReferrer();
  // reload document
  d->m_khtml->closeURL();
  d->m_khtml->browserExtension()->setURLArgs( args );
  d->m_khtml->openURL( d->m_khtml->url() );
}

void KHTMLPopupGUIClient::slotFrameInWindow()
{
  KParts::URLArgs args( d->m_khtml->browserExtension()->urlArgs() );
  args.metaData()["referrer"] = d->m_khtml->pageReferrer();
  args.metaData()["forcenewwindow"] = "true";
  emit d->m_khtml->browserExtension()->createNewWindow( d->m_khtml->url(), args );
}

void KHTMLPopupGUIClient::slotFrameInTop()
{
  KParts::URLArgs args( d->m_khtml->browserExtension()->urlArgs() );
  args.metaData()["referrer"] = d->m_khtml->pageReferrer();
  args.frameName = "_top";
  emit d->m_khtml->browserExtension()->openURLRequest( d->m_khtml->url(), args );
}

void KHTMLPopupGUIClient::slotFrameInTab()
{
  KParts::URLArgs args( d->m_khtml->browserExtension()->urlArgs() );
  args.metaData()["referrer"] = d->m_khtml->pageReferrer();
  args.setNewTab(true);
  emit d->m_khtml->browserExtension()->createNewWindow( d->m_khtml->url(), args );
}

void KHTMLPopupGUIClient::saveURL( QWidget *parent, const QString &caption,
                                   const KURL &url,
                                   const QMap<QString, QString> &metadata,
                                   const QString &filter, long cacheId,
                                   const QString & suggestedFilename )
{
  QString name = QLatin1String( "index.html" );
  if ( !suggestedFilename.isEmpty() )
    name = suggestedFilename;
  else if ( !url.fileName().isEmpty() )
    name = url.fileName();

  KURL destURL;
  int query;
  do {
    query = KMessageBox::Yes;
    destURL = KFileDialog::getSaveURL( name, filter, parent, caption );
      if( destURL.isLocalFile() )
      {
        QFileInfo info( destURL.path() );
        if( info.exists() ) {
          // TODO: use KIO::RenameDlg (shows more information)
          query = KMessageBox::warningContinueCancel( parent, i18n( "A file named \"%1\" already exists. " "Are you sure you want to overwrite it?" ).arg( info.fileName() ), i18n( "Overwrite File?" ), i18n( "Overwrite" ) );
        }
       }
   } while ( query == KMessageBox::Cancel );

  if ( destURL.isValid() )
    saveURL(url, destURL, metadata, cacheId);
}

void KHTMLPopupGUIClient::saveURL( const KURL &url, const KURL &destURL,
                                   const QMap<QString, QString> &metadata,
                                   long cacheId )
{
    if ( destURL.isValid() )
    {
        bool saved = false;
        if (KHTMLPageCache::self()->isComplete(cacheId))
        {
            if (destURL.isLocalFile())
            {
                KSaveFile destFile(destURL.path());
                if (destFile.status() == 0)
                {
                    KHTMLPageCache::self()->saveData(cacheId, destFile.dataStream());
                    saved = true;
                }
            }
            else
            {
                // save to temp file, then move to final destination.
                KTempFile destFile;
                if (destFile.status() == 0)
                {
                    KHTMLPageCache::self()->saveData(cacheId, destFile.dataStream());
                    destFile.close();
                    KURL url2 = KURL();
                    url2.setPath(destFile.name());
                    KIO::file_move(url2, destURL, -1, true /*overwrite*/);
                    saved = true;
                }
            }
        }
        if(!saved)
        {
          // DownloadManager <-> konqueror integration
          // find if the integration is enabled
          // the empty key  means no integration
          // only use download manager for non-local urls!
          bool downloadViaKIO = true;
          if ( !url.isLocalFile() )
          {
            KConfig cfg("konquerorrc", false, false);
            cfg.setGroup("HTML Settings");
            QString downloadManger = cfg.readPathEntry("DownloadManager");
            if (!downloadManger.isEmpty())
            {
                // then find the download manager location
                kdDebug(1000) << "Using: "<<downloadManger <<" as Download Manager" <<endl;
                QString cmd = KStandardDirs::findExe(downloadManger);
                if (cmd.isEmpty())
                {
                    QString errMsg=i18n("The Download Manager (%1) could not be found in your $PATH ").arg(downloadManger);
                    QString errMsgEx= i18n("Try to reinstall it  \n\nThe integration with Konqueror will be disabled!");
                    KMessageBox::detailedSorry(0,errMsg,errMsgEx);
                    cfg.writePathEntry("DownloadManager",QString());
                    cfg.sync ();
                }
                else
                {
                    downloadViaKIO = false;
                    KURL cleanDest = destURL;
                    cleanDest.setPass( QString() ); // don't put password into commandline
                    cmd += " " + KProcess::quote(url.url()) + " " +
                           KProcess::quote(cleanDest.url());
                    kdDebug(1000) << "Calling command  "<<cmd<<endl;
                    KRun::runCommand(cmd);
                }
            }
          }

          if ( downloadViaKIO )
          {
              KIO::Job *job = KIO::file_copy( url, destURL, -1, true /*overwrite*/ );
              job->setMetaData(metadata);
              job->addMetaData("MaxCacheSize", "0"); // Don't store in http cache.
              job->addMetaData("cache", "cache"); // Use entry from cache if available.
              job->setAutoErrorHandlingEnabled( true );
          }
        } //end if(!saved)
    }
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

const Q3PtrList<KParts::ReadOnlyPart> KHTMLPartBrowserHostExtension::frames() const
{
  return m_part->frames();
}

bool KHTMLPartBrowserHostExtension::openURLInFrame( const KURL &url, const KParts::URLArgs &urlArgs )
{
  return m_part->openURLInFrame( url, urlArgs );
}

void KHTMLPartBrowserHostExtension::virtual_hook( int id, void *data )
{
  if (id == VIRTUAL_FIND_FRAME_PARENT)
  {
    FindFrameParentParams *param = static_cast<FindFrameParentParams*>(data);
    KHTMLPart *parentPart = m_part->findFrameParent(param->callingPart, param->frame);
    if (parentPart)
       param->parent = parentPart->browserHostExtension();
    return;
  }
  BrowserHostExtension::virtual_hook( id, data );
}


// defined in khtml_part.cpp
extern const int KDE_NO_EXPORT fastZoomSizes[];
extern const int KDE_NO_EXPORT fastZoomSizeCount;

KHTMLZoomFactorAction::KHTMLZoomFactorAction( KHTMLPart *part, bool direction, const QString &text, const QString &icon, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name )
    : KAction( text, icon, cut, receiver, slot, parent, name )
{
    init(part, direction);
}

void KHTMLZoomFactorAction::init(KHTMLPart *part, bool direction)
{
    m_direction = direction;
    m_part = part;

    m_popup = new QMenu;
    // xgettext: no-c-format
    m_popup->insertItem( i18n( "Default Font Size (100%)" ) );

    int m = m_direction ? 1 : -1;
    int ofs = fastZoomSizeCount / 2;       // take index of 100%

    // this only works if there is an odd number of elements in fastZoomSizes[]
    for ( int i = m; i != m*(ofs+1); i += m )
    {
        int num = i * m;
        QString numStr = QString::number( num );
        if ( num > 0 ) numStr.prepend( QLatin1Char('+') );

        // xgettext: no-c-format
        m_popup->insertItem( i18n( "%1%" ).arg( fastZoomSizes[ofs + i] ) );
    }

    connect( m_popup, SIGNAL( activated( int ) ), this, SLOT( slotActivated( int ) ) );
}

KHTMLZoomFactorAction::~KHTMLZoomFactorAction()
{
    delete m_popup;
}

int KHTMLZoomFactorAction::plug( QWidget *w, int index )
{
    int containerId = KAction::plug( w, index );
    if ( containerId == -1 || !w->inherits( "KToolBar" ) )
        return containerId;

    KToolBarButton *button = static_cast<KToolBar *>( w )->getButton( itemId( containerId ) );
    if ( !button )
        return containerId;

    button->setDelayedPopup( m_popup );
    return containerId;
}

void KHTMLZoomFactorAction::slotActivated( int id )
{
    int idx = m_popup->indexOf( id );

    if (idx == 0)
        m_part->setZoomFactor(100);
    else
        m_part->setZoomFactor(fastZoomSizes[fastZoomSizeCount/2 + (m_direction ? 1 : -1)*idx]);
}

#include "khtml_ext.moc"

