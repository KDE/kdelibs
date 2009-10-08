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

#include "khtml_ext.h"
#include "khtmlview.h"
#include "khtml_pagecache.h"
#include "rendering/render_form.h"
#include "rendering/render_image.h"
#include "html/html_imageimpl.h"
#include "misc/loader.h"
#include "dom/html_form.h"
#include "dom/html_image.h"
#include <QtGui/QClipboard>
#include <QtCore/QFileInfo>
#include <QtGui/QMenu>
#include <QtCore/QUrl>
#include <QtCore/QMetaEnum>
#include <assert.h>

#include <kdebug.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kjobuidelegate.h>
#include <kio/job.h>
#include <kshell.h>
#include <ktoolbar.h>
#include <ksavefile.h>
#include <kstringhandler.h>
#include <ktoolinvocation.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <krun.h>
#include <kurifilter.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kdesktopfile.h>
#include <kinputdialog.h>
#include <ktemporaryfile.h>
#include "khtml_global.h"
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <kactionmenu.h>

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
  //kDebug( 6050 ) << "saveState!";
  m_part->saveState( stream );
}

void KHTMLPartBrowserExtension::restoreState( QDataStream &stream )
{
  //kDebug( 6050 ) << "restoreState!";
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
        callExtensionProxyMethod( "cut" );
        return;
    }

    if ( !m_editableFormWidget )
        return;

    QLineEdit* lineEdit = qobject_cast<QLineEdit *>( m_editableFormWidget );
    if ( lineEdit && !lineEdit->isReadOnly() )
        lineEdit->cut();
    QTextEdit* textEdit = qobject_cast<QTextEdit *>( m_editableFormWidget );
    if ( textEdit && !textEdit->isReadOnly() )
        textEdit->cut();
}

void KHTMLPartBrowserExtension::copy()
{
    if ( m_extensionProxy )
    {
        callExtensionProxyMethod( "copy" );
        return;
    }

    if ( !m_editableFormWidget )
    {
        // get selected text and paste to the clipboard
        QString text = m_part->selectedText();
        text.replace( QChar( 0xa0 ), ' ' );
        //kDebug(6050) << text;

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
        QLineEdit* lineEdit = qobject_cast<QLineEdit *>( m_editableFormWidget );
        if ( lineEdit )
            lineEdit->copy();
        QTextEdit* textEdit = qobject_cast<QTextEdit *>( m_editableFormWidget );
        if ( textEdit )
            textEdit->copy();
    }
}

void KHTMLPartBrowserExtension::searchProvider()
{
    // action name is of form "previewProvider[<searchproviderprefix>:]"
    const QString searchProviderPrefix = QString( sender()->objectName() ).mid( 14 );

    const QString text = m_part->simplifiedSelectedText();
    KUriFilterData data;
    QStringList list;
    data.setData( searchProviderPrefix + text );
    list << "kurisearchfilter" << "kuriikwsfilter";

    if( !KUriFilter::self()->filterUri(data, list) )
    {
        KDesktopFile file("services", "searchproviders/google.desktop");
        QString encodedSearchTerm = QUrl::toPercentEncoding(text);
        KConfigGroup cg(file.desktopGroup());
        data.setData(cg.readEntry("Query").replace("\\{@}", encodedSearchTerm));
    }

    KParts::BrowserArguments browserArgs;
    browserArgs.frameName = "_blank";

    emit m_part->browserExtension()->openUrlRequest( data.uri(), KParts::OpenUrlArguments(), browserArgs );
}

void KHTMLPartBrowserExtension::paste()
{
    if ( m_extensionProxy )
    {
        callExtensionProxyMethod( "paste" );
        return;
    }

    if ( !m_editableFormWidget )
        return;

    QLineEdit* lineEdit = qobject_cast<QLineEdit *>( m_editableFormWidget );
    if ( lineEdit && !lineEdit->isReadOnly() )
        lineEdit->paste();
    QTextEdit* textEdit = qobject_cast<QTextEdit *>( m_editableFormWidget );
    if ( textEdit && !textEdit->isReadOnly() )
        textEdit->paste();
}

void KHTMLPartBrowserExtension::callExtensionProxyMethod( const char *method )
{
    if ( !m_extensionProxy )
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
    const QMimeData *data = QApplication::clipboard()->mimeData();
    enableAction( "paste", data->hasFormat( "text/plain" ) );
#else
    QString data=QApplication::clipboard()->text();
    enableAction( "paste", data.contains("://"));
#endif
    bool hasSelection = false;

    if( m_editableFormWidget) {
        if ( qobject_cast<QLineEdit*>(m_editableFormWidget))
            hasSelection = static_cast<QLineEdit *>( &(*m_editableFormWidget) )->hasSelectedText();
        else if(qobject_cast<QTextEdit*>(m_editableFormWidget))
            hasSelection = static_cast<QTextEdit *>( &(*m_editableFormWidget) )->textCursor().hasSelection();
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
  QScrollArea *scrollArea = m_part->view();
  if (scrollArea) {
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  }
}

class KHTMLPopupGUIClient::KHTMLPopupGUIClientPrivate
{
public:
  KHTMLPart *m_khtml;
  KUrl m_url;
  KUrl m_imageURL;
  QPixmap m_pixmap;
  QString m_suggestedFilename;
    KActionCollection* m_actionCollection;
    KParts::BrowserExtension::ActionGroupMap actionGroups;
};


KHTMLPopupGUIClient::KHTMLPopupGUIClient( KHTMLPart *khtml, const KUrl &url )
    : QObject( khtml ), d(new KHTMLPopupGUIClientPrivate)
{
    d->m_khtml = khtml;
    d->m_url = url;
    d->m_actionCollection = new KActionCollection(this);
    bool isImage = false;
    bool hasSelection = khtml->hasSelection();

    DOM::Element e = khtml->nodeUnderMouse();

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

    if (hasSelection) {
        QList<QAction *> editActions;
        QAction* copyAction = d->m_actionCollection->addAction( KStandardAction::Copy, "copy",
                                                                d->m_khtml->browserExtension(), SLOT( copy() ) );

        copyAction->setText(i18n("&Copy Text"));
        copyAction->setEnabled(d->m_khtml->browserExtension()->isActionEnabled( "copy" ));
        editActions.append(copyAction);

        editActions.append(khtml->actionCollection()->action("selectAll"));

        addSearchActions(editActions);

        QString selectedTextURL = selectedTextAsOneLine();
        if ( selectedTextURL.contains("://") && KUrl(selectedTextURL).isValid() ) {
            if (selectedTextURL.length() > 18) {
                selectedTextURL.truncate(15);
                selectedTextURL += "...";
            }
            KAction *action = new KAction(i18n("Open '%1'", selectedTextURL), this);
            d->m_actionCollection->addAction( "openSelection", action );
            action->setIcon( KIcon( "window-new" ) );
            connect( action, SIGNAL(triggered(bool)), this, SLOT( openSelection() ) );
            editActions.append(action);
        }

        KAction* separator = new KAction(d->m_actionCollection);
        separator->setSeparator(true);
        editActions.append(separator);

        d->actionGroups.insert("editactions", editActions);
    }

    if (!url.isEmpty()) {
        QList<QAction *> linkActions;
        if (url.protocol() == "mailto") {
            KAction *action = new KAction( i18n( "&Copy Email Address" ), this );
            d->m_actionCollection->addAction( "copylinklocation", action );
            connect( action, SIGNAL(triggered(bool)), this, SLOT(slotCopyLinkLocation()) );
            linkActions.append(action);
        } else {
            KAction *action = new KAction( i18n( "&Save Link As..." ), this );
            d->m_actionCollection->addAction( "savelinkas", action );
            connect( action, SIGNAL(triggered(bool)), this, SLOT(slotSaveLinkAs()) );
            linkActions.append(action);

            action = new KAction( i18n( "&Copy Link Address" ), this );
            d->m_actionCollection->addAction( "copylinklocation", action );
            connect( action, SIGNAL(triggered(bool)), this, SLOT( slotCopyLinkLocation() ) );
            linkActions.append(action);
        }
        d->actionGroups.insert("linkactions", linkActions);
    }

    QList<QAction *> partActions;
    // frameset? -> add "Reload Frame" etc.
    if (!hasSelection) {
        if ( khtml->parentPart() ) {
            KActionMenu* menu = new KActionMenu( i18nc("@title:menu HTML frame/iframe", "Frame"), this);
            KAction *action = new KAction( i18n( "Open in New &Window" ), this );
            d->m_actionCollection->addAction( "frameinwindow", action );
            action->setIcon( KIcon( "window-new" ) );
            connect( action, SIGNAL(triggered(bool)), this, SLOT(slotFrameInWindow()) );
            menu->addAction(action);

            action = new KAction( i18n( "Open in &This Window" ), this );
            d->m_actionCollection->addAction( "frameintop", action );
            connect( action, SIGNAL(triggered(bool)), this, SLOT( slotFrameInTop() ) );
            menu->addAction(action);

            action = new KAction( i18n( "Open in &New Tab" ), this );
            d->m_actionCollection->addAction( "frameintab", action );
            action->setIcon( KIcon( "tab-new" ) );
            connect( action, SIGNAL(triggered(bool)), this, SLOT( slotFrameInTab() ) );
            menu->addAction(action);

            action = new KAction(d->m_actionCollection);
            action->setSeparator(true);
            menu->addAction(action);

            action = new KAction( i18n( "Reload Frame" ), this );
            d->m_actionCollection->addAction( "reloadframe", action );
            connect( action, SIGNAL(triggered(bool)), this, SLOT( slotReloadFrame() ) );
            menu->addAction(action);

            action = new KAction( i18n( "Print Frame..." ), this );
            d->m_actionCollection->addAction( "printFrame", action );
            action->setIcon( KIcon( "document-print-frame" ) );
            connect( action, SIGNAL(triggered(bool)), d->m_khtml->browserExtension(), SLOT( print() ) );
            menu->addAction(action);

            action = new KAction( i18n( "Save &Frame As..." ), this );
            d->m_actionCollection->addAction( "saveFrame", action );
            connect( action, SIGNAL(triggered(bool)), d->m_khtml, SLOT( slotSaveFrame() ) );
            menu->addAction(action);

            action = new KAction( i18n( "View Frame Source" ), this );
            d->m_actionCollection->addAction( "viewFrameSource", action );
            connect( action, SIGNAL(triggered(bool)), d->m_khtml, SLOT( slotViewDocumentSource() ) );
            menu->addAction(action);

            action = new KAction( i18n( "View Frame Information" ), this );
            d->m_actionCollection->addAction( "viewFrameInfo", action );
            connect( action, SIGNAL(triggered(bool)), d->m_khtml, SLOT( slotViewPageInfo() ) );

            action = new KAction(d->m_actionCollection);
            action->setSeparator(true);
            menu->addAction(action);

            if ( KHTMLGlobal::defaultHTMLSettings()->isAdFilterEnabled() ) {
                if ( khtml->d->m_frame->m_type == khtml::ChildFrame::IFrame ) {
                    action = new KAction( i18n( "Block IFrame..." ), this );
                    d->m_actionCollection->addAction( "blockiframe", action );
                    connect( action, SIGNAL(triggered(bool)), this, SLOT( slotBlockIFrame() ) );
                    menu->addAction(action);
                }
            }

            partActions.append(menu);
        }
    }

    if (isImage) {
        if ( e.elementId() == ID_IMG ) {
            d->m_imageURL = KUrl( static_cast<DOM::HTMLImageElement>( e ).src().string() );
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
            d->m_imageURL = KUrl( static_cast<DOM::HTMLInputElement>( e ).src().string() );
        KAction *action = new KAction( i18n( "Save Image As..." ), this );
        d->m_actionCollection->addAction( "saveimageas", action );
        connect( action, SIGNAL(triggered(bool)), this, SLOT( slotSaveImageAs() ) );
        partActions.append(action);

        action = new KAction( i18n( "Send Image..." ), this );
        d->m_actionCollection->addAction( "sendimage", action );
        connect( action, SIGNAL(triggered(bool)), this, SLOT( slotSendImage() ) );
        partActions.append(action);

#ifndef QT_NO_MIMECLIPBOARD
        action = new KAction( i18n( "Copy Image" ), this );
        d->m_actionCollection->addAction( "copyimage", action );
        action->setEnabled(!d->m_pixmap.isNull());
        connect( action, SIGNAL(triggered(bool)), this, SLOT( slotCopyImage() ) );
        partActions.append(action);
#endif

        if(d->m_pixmap.isNull()) {    //fallback to image location if still loading the image.  this will always be true if ifdef QT_NO_MIMECLIPBOARD
            action = new KAction( i18n( "Copy Image Location" ), this );
            d->m_actionCollection->addAction( "copyimagelocation", action );
            connect( action, SIGNAL(triggered(bool)), this, SLOT( slotCopyImageLocation() ) );
            partActions.append(action);
        }

        QString actionText = d->m_suggestedFilename.isEmpty() ?
                                   KStringHandler::csqueeze(d->m_imageURL.fileName()+d->m_imageURL.query(), 25)
                                   : d->m_suggestedFilename;
        action = new KAction( i18n("View Image (%1)", actionText.replace("&", "&&")), this );
        d->m_actionCollection->addAction( "viewimage", action );
        connect( action, SIGNAL(triggered(bool)), this, SLOT( slotViewImage() ) );
        partActions.append(action);

        if (KHTMLGlobal::defaultHTMLSettings()->isAdFilterEnabled()) {
            action = new KAction( i18n( "Block Image..." ), this );
            d->m_actionCollection->addAction( "blockimage", action );
            connect( action, SIGNAL(triggered(bool)), this, SLOT( slotBlockImage() ) );
            partActions.append(action);

            if (!d->m_imageURL.host().isEmpty() &&
                !d->m_imageURL.protocol().isEmpty())
            {
                action = new KAction( i18n( "Block Images From %1" , d->m_imageURL.host()), this );
                d->m_actionCollection->addAction( "blockhost", action );
                connect( action, SIGNAL(triggered(bool)), this, SLOT( slotBlockHost() ) );
                partActions.append(action);
            }
        }
        KAction* separator = new KAction(d->m_actionCollection);
        separator->setSeparator(true);
        partActions.append(separator);
    }

    if ( isImage || url.isEmpty() ) {
        KAction *action = new KAction( i18n( "Stop Animations" ), this );
        d->m_actionCollection->addAction( "stopanimations", action );
        connect( action, SIGNAL(triggered(bool)), this, SLOT(slotStopAnimations()) );
        partActions.append(action);
        KAction* separator = new KAction(d->m_actionCollection);
        separator->setSeparator(true);
        partActions.append(separator);
    }
    if (!hasSelection && url.isEmpty()) { // only when right-clicking on the page itself
        partActions.append(khtml->actionCollection()->action("viewDocumentSource"));
    }
    if (!hasSelection && url.isEmpty() && !isImage) {
        partActions.append(khtml->actionCollection()->action("setEncoding"));
    }
    d->actionGroups.insert("partactions", partActions);
}

KHTMLPopupGUIClient::~KHTMLPopupGUIClient()
{
    delete d->m_actionCollection;
    delete d;
}

void KHTMLPopupGUIClient::addSearchActions(QList<QAction *>& editActions)
{
    // Fill search provider entries
    KConfig config("kuriikwsfilterrc");
    KConfigGroup cg = config.group("General");
    const QString defaultEngine = cg.readEntry("DefaultSearchEngine", "google");
    const char keywordDelimiter = cg.readEntry("KeywordDelimiter", static_cast<int>(':'));

    // search text
    QString selectedText = d->m_khtml->simplifiedSelectedText();
    if (selectedText.isEmpty())
        return;

    selectedText.replace("&", "&&");
    if (selectedText.length() > 18) {
        selectedText.truncate(15);
        selectedText += "...";
    }

    // default search provider
    KService::Ptr service;
    if( !defaultEngine.isEmpty())
        service = KService::serviceByDesktopPath(QString("searchproviders/%1.desktop").arg(defaultEngine));

    // search provider icon
    KIcon icon;
    KUriFilterData data;
    QStringList list;
    data.setData(QString("some keyword"));
    list << "kurisearchfilter" << "kuriikwsfilter";

    QString name;
    if (KUriFilter::self()->filterUri(data, list)) {
        QString iconPath = KStandardDirs::locate("cache", KMimeType::favIconForUrl(data.uri()) + ".png");
        if (iconPath.isEmpty())
            icon = KIcon("edit-find");
        else
            icon = KIcon(QPixmap(iconPath));
        name = service->name();
    } else {
        icon = KIcon("google");
        name = "Google";
    }

    KAction *action = new KAction(i18n("Search for '%1' with %2", selectedText, name), this);
    d->m_actionCollection->addAction("searchProvider", action);
    editActions.append(action);
    action->setIcon(icon);
    connect(action, SIGNAL(triggered(bool)), d->m_khtml->browserExtension(), SLOT(searchProvider()));

    // favorite search providers
    QStringList favoriteEngines;
    favoriteEngines << "google" << "google_groups" << "google_news" << "webster" << "dmoz" << "wikipedia";
    favoriteEngines = cg.readEntry("FavoriteSearchEngines", favoriteEngines);

    if (!favoriteEngines.isEmpty()) {
        KActionMenu* providerList = new KActionMenu(i18n("Search for '%1' with", selectedText), this);
        d->m_actionCollection->addAction("searchProviderList", providerList);
        editActions.append(providerList);

        QStringList::ConstIterator it = favoriteEngines.constBegin();
        for (; it != favoriteEngines.constEnd(); ++it) {
            if (*it==defaultEngine)
                continue;
            service = KService::serviceByDesktopPath(QString("searchproviders/%1.desktop").arg(*it));
            if (!service)
                continue;
            const QString searchProviderPrefix = *(service->property("Keys").toStringList().begin()) + keywordDelimiter;
            data.setData(searchProviderPrefix + "some keyword");

            if (KUriFilter::self()->filterUri(data, list)) {
                const QString iconPath = KStandardDirs::locate("cache", KMimeType::favIconForUrl(data.uri()) + ".png");
                if (iconPath.isEmpty())
                    icon = KIcon("edit-find");
                else
                    icon = KIcon(iconPath);
                name = service->name();

                KAction *action = new KAction(name, this);
                d->m_actionCollection->addAction(QString("searchProvider" + searchProviderPrefix).toLatin1().constData(), action);
                action->setIcon(icon);
                connect(action, SIGNAL(triggered(bool)), d->m_khtml->browserExtension(), SLOT(searchProvider()));

                providerList->addAction(action);
            }
        }
    }
}

QString KHTMLPopupGUIClient::selectedTextAsOneLine() const
{
    QString text = d->m_khtml->simplifiedSelectedText();
    // in addition to what simplifiedSelectedText does,
    // remove linefeeds and any whitespace surrounding it (#113177),
    // to get it all in a single line.
    text.remove(QRegExp("[\\s]*\\n+[\\s]*"));
    return text;
}

void KHTMLPopupGUIClient::openSelection()
{
    KParts::BrowserArguments browserArgs;
    browserArgs.frameName = "_blank";

    emit d->m_khtml->browserExtension()->openUrlRequest(selectedTextAsOneLine(), KParts::OpenUrlArguments(), browserArgs);
}

KParts::BrowserExtension::ActionGroupMap KHTMLPopupGUIClient::actionGroups() const
{
    return d->actionGroups;
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
    KHTMLGlobal::defaultHTMLSettings()->addAdFilter( name );
    d->m_khtml->reparseConfiguration();
}

void KHTMLPopupGUIClient::slotBlockImage()
{
    bool ok = false;

    QString url = KInputDialog::getText( i18n("Add URL to Filter"),
                                         i18n("Enter the URL:"),
                                         d->m_imageURL.url(),
                                         &ok);
    if ( ok ) {
        KHTMLGlobal::defaultHTMLSettings()->addAdFilter( url );
        d->m_khtml->reparseConfiguration();
    }
}

void KHTMLPopupGUIClient::slotBlockIFrame()
{
    bool ok = false;
    QString url = KInputDialog::getText( i18n( "Add URL to Filter"),
                                               i18n("Enter the URL:"),
                                               d->m_khtml->url().url(),
                                               &ok );
    if ( ok ) {
        KHTMLGlobal::defaultHTMLSettings()->addAdFilter( url );
        d->m_khtml->reparseConfiguration();
    }
}

void KHTMLPopupGUIClient::slotCopyLinkLocation()
{
  KUrl safeURL(d->m_url);
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
  KUrl safeURL(d->m_imageURL);
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
  kDebug() << "slotCopyImage called when the clipboard does not support this.  This should not be possible.";
#endif
}

void KHTMLPopupGUIClient::slotCopyImageLocation()
{
  KUrl safeURL(d->m_imageURL);
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
  KParts::OpenUrlArguments args = d->m_khtml->arguments();
  args.setReload( true );
  args.metaData()["referrer"] = d->m_khtml->pageReferrer();
  // reload document
  d->m_khtml->closeUrl();
  d->m_khtml->setArguments( args );
  d->m_khtml->openUrl( d->m_khtml->url() );
}

void KHTMLPopupGUIClient::slotFrameInWindow()
{
  KParts::OpenUrlArguments args = d->m_khtml->arguments();
  args.metaData()["referrer"] = d->m_khtml->pageReferrer();
  args.metaData()["forcenewwindow"] = "true";
  emit d->m_khtml->browserExtension()->createNewWindow( d->m_khtml->url(), args );
}

void KHTMLPopupGUIClient::slotFrameInTop()
{
  KParts::OpenUrlArguments args = d->m_khtml->arguments();
  args.metaData()["referrer"] = d->m_khtml->pageReferrer();
  KParts::BrowserArguments browserArgs( d->m_khtml->browserExtension()->browserArguments() );
  browserArgs.frameName = "_top";
  emit d->m_khtml->browserExtension()->openUrlRequest( d->m_khtml->url(), args, browserArgs );
}

void KHTMLPopupGUIClient::slotFrameInTab()
{
  KParts::OpenUrlArguments args = d->m_khtml->arguments();
  args.metaData()["referrer"] = d->m_khtml->pageReferrer();
  KParts::BrowserArguments browserArgs( d->m_khtml->browserExtension()->browserArguments() );
  browserArgs.setNewTab(true);
  emit d->m_khtml->browserExtension()->createNewWindow( d->m_khtml->url(), args, browserArgs );
}

void KHTMLPopupGUIClient::saveURL( QWidget *parent, const QString &caption,
                                   const KUrl &url,
                                   const QMap<QString, QString> &metadata,
                                   const QString &filter, long cacheId,
                                   const QString & suggestedFilename )
{
  QString name = QLatin1String( "index.html" );
  if ( !suggestedFilename.isEmpty() )
    name = suggestedFilename;
  else if ( !url.fileName().isEmpty() )
    name = url.fileName();

  KUrl destURL;
  int query;
  do {
    query = KMessageBox::Yes;
    destURL = KFileDialog::getSaveUrl( name, filter, parent, caption );
      if( destURL.isLocalFile() )
      {
        QFileInfo info( destURL.toLocalFile() );
        if( info.exists() ) {
          // TODO: use KIO::RenameDlg (shows more information)
          query = KMessageBox::warningContinueCancel( parent, i18n( "A file named \"%1\" already exists. " "Are you sure you want to overwrite it?" ,  info.fileName() ), i18n( "Overwrite File?" ), KGuiItem(i18n( "Overwrite" )) );
        }
       }
   } while ( query == KMessageBox::Cancel );

  if ( destURL.isValid() )
    saveURL(parent, url, destURL, metadata, cacheId);
}

void KHTMLPopupGUIClient::saveURL( QWidget* parent, const KUrl &url, const KUrl &destURL,
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
                KSaveFile destFile(destURL.toLocalFile());
                if (destFile.open())
                {
                    QDataStream stream ( &destFile );
                    KHTMLPageCache::self()->saveData(cacheId, &stream);
                    saved = true;
                }
            }
            else
            {
                // save to temp file, then move to final destination.
                KTemporaryFile destFile;
                if (destFile.open())
                {
                    QDataStream stream ( &destFile );
                    KHTMLPageCache::self()->saveData(cacheId, &stream);
                    KUrl url2 = KUrl();
                    url2.setPath(destFile.fileName());
                    KIO::file_move(url2, destURL, -1, KIO::Overwrite);
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
            KConfigGroup cfg = KSharedConfig::openConfig("konquerorrc", KConfig::NoGlobals)->group("HTML Settings");
            QString downloadManger = cfg.readPathEntry("DownloadManager", QString());
            if (!downloadManger.isEmpty())
            {
                // then find the download manager location
                kDebug(1000) << "Using: "<<downloadManger <<" as Download Manager";
                QString cmd = KStandardDirs::findExe(downloadManger);
                if (cmd.isEmpty())
                {
                    QString errMsg=i18n("The Download Manager (%1) could not be found in your $PATH ", downloadManger);
                    QString errMsgEx= i18n("Try to reinstall it  \n\nThe integration with Konqueror will be disabled.");
                    KMessageBox::detailedSorry(0,errMsg,errMsgEx);
                    cfg.writePathEntry("DownloadManager",QString());
                    cfg.sync ();
                }
                else
                {
                    downloadViaKIO = false;
                    KUrl cleanDest = destURL;
                    cleanDest.setPass( QString() ); // don't put password into commandline
                    cmd += ' ' + KShell::quoteArg(url.url()) + ' ' +
                           KShell::quoteArg(cleanDest.url());
                    kDebug(1000) << "Calling command  "<<cmd;
                    KRun::runCommand(cmd, parent->topLevelWidget());
                }
            }
          }

          if ( downloadViaKIO )
          {
              KIO::Job *job = KIO::file_copy( url, destURL, -1, KIO::Overwrite );
              job->setMetaData(metadata);
              job->addMetaData("MaxCacheSize", "0"); // Don't store in http cache.
              job->addMetaData("cache", "cache"); // Use entry from cache if available.
              job->uiDelegate()->setAutoErrorHandlingEnabled( true );
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

const QList<KParts::ReadOnlyPart*> KHTMLPartBrowserHostExtension::frames() const
{
  return m_part->frames();
}

bool KHTMLPartBrowserHostExtension::openUrlInFrame(const KUrl &url, const KParts::OpenUrlArguments& arguments, const KParts::BrowserArguments &browserArguments)
{
  return m_part->openUrlInFrame( url, arguments, browserArguments );
}

KParts::BrowserHostExtension* KHTMLPartBrowserHostExtension::findFrameParent( KParts::ReadOnlyPart
      *callingPart, const QString &frame )
{
    KHTMLPart *parentPart = m_part->findFrameParent(callingPart, frame);
    if (parentPart)
       return parentPart->browserHostExtension();
    return 0;
}


// defined in khtml_part.cpp
extern const int KDE_NO_EXPORT fastZoomSizes[];
extern const int KDE_NO_EXPORT fastZoomSizeCount;

KHTMLZoomFactorAction::KHTMLZoomFactorAction( KHTMLPart *part, bool direction, const QString &icon, const QString &text, QObject *parent )
    : KSelectAction( text, parent )
{
    setIcon( KIcon( icon ) );

    setToolBarMode(MenuMode);
    setToolButtonPopupMode(QToolButton::DelayedPopup);

    init(part, direction);
}

void KHTMLZoomFactorAction::init(KHTMLPart *part, bool direction)
{
    m_direction = direction;
    m_part = part;

    // xgettext: no-c-format
    addAction( i18n( "Default Font Size (100%)" ) );

    int m = m_direction ? 1 : -1;
    int ofs = fastZoomSizeCount / 2;       // take index of 100%

    // this only works if there is an odd number of elements in fastZoomSizes[]
    for ( int i = m; i != m*(ofs+1); i += m )
    {
        int num = i * m;
        QString numStr = QString::number( num );
        if ( num > 0 ) numStr.prepend( QLatin1Char('+') );

        // xgettext: no-c-format
        addAction( i18n( "%1%" ,  fastZoomSizes[ofs + i] ) );
    }

    connect( selectableActionGroup(), SIGNAL( triggered(QAction*) ), this, SLOT( slotTriggered(QAction*) ) );
}

KHTMLZoomFactorAction::~KHTMLZoomFactorAction()
{
}

void KHTMLZoomFactorAction::slotTriggered(QAction* action)
{
    int idx = selectableActionGroup()->actions().indexOf(action);

    if (idx == 0)
        m_part->setFontScaleFactor(100);
    else
        m_part->setFontScaleFactor(fastZoomSizes[fastZoomSizeCount/2 + (m_direction ? 1 : -1)*idx]);
    setCurrentAction( 0L );
}

#include "khtml_ext.moc"

