
#include "khtml_iface.h"
#include "khtml_part.h"
#include "khtmlview.h"
#include "khtml_ext.h"
#include <kio/global.h>
#include <qapplication.h>
#include <qvariant.h>

KHTMLPartIface::KHTMLPartIface( KHTMLPart *_part )
    : DCOPObject( _part->dcopObjectId() ), part(_part)
{

}

void KHTMLPartIface::setJScriptEnabled( bool enable )
{
    part->setJScriptEnabled(enable);
}

bool KHTMLPartIface::jScriptEnabled() const
{
    return part->jScriptEnabled();
}

KHTMLPartIface::~KHTMLPartIface()
{

}

bool KHTMLPartIface::closeURL()
{
    return part->closeURL();
}

bool KHTMLPartIface::metaRefreshEnabled() const
{
    return part->metaRefreshEnabled();
}

void KHTMLPartIface::setDNDEnabled( bool b )
{
    part->setDNDEnabled(b);
}

bool KHTMLPartIface::dndEnabled() const
{
    return part->dndEnabled();
}

void KHTMLPartIface::setJavaEnabled( bool enable )
{
    part->setJavaEnabled( enable );
}

bool KHTMLPartIface::javaEnabled() const
{
    return part->javaEnabled();
}

void KHTMLPartIface::setPluginsEnabled( bool enable )
{
    part->setPluginsEnabled( enable );
}

bool KHTMLPartIface::pluginsEnabled() const
{
    return part->pluginsEnabled();
}

void KHTMLPartIface::setAutoloadImages( bool enable )
{
    part->setAutoloadImages( enable );
}

bool KHTMLPartIface::autoloadImages() const
{
    return part->autoloadImages();
}

void KHTMLPartIface::setOnlyLocalReferences(bool enable)
{
    part->setOnlyLocalReferences(enable);
}

void KHTMLPartIface::setMetaRefreshEnabled( bool enable )
{
    part->setMetaRefreshEnabled(enable);
}

bool KHTMLPartIface::onlyLocalReferences() const
{
    return part->onlyLocalReferences();
}

bool KHTMLPartIface::setEncoding( const QString &name )
{
    return part->setEncoding(name);
}

QString KHTMLPartIface::encoding() const
{
    return part->encoding();
}

void KHTMLPartIface::setFixedFont( const QString &name )
{
    part->setFixedFont(name);

}
bool KHTMLPartIface::gotoAnchor( const QString &name )
{
    return part->gotoAnchor(name);
}

void KHTMLPartIface::activateNode()
{
    KParts::ReadOnlyPart* p = part->currentFrame();
    if ( p && p->widget() ) {
        QKeyEvent ev( QKeyEvent::KeyPress, Qt::Key_Return, '\n', 0, "\n" );
        QApplication::sendEvent( p->widget(), &ev );
    }
}

void KHTMLPartIface::selectAll()
{
    part->selectAll();
}

QString KHTMLPartIface::lastModified() const
{
    return part->lastModified();
}

void KHTMLPartIface::debugRenderTree()
{
    part->slotDebugRenderTree();
}

void KHTMLPartIface::viewDocumentSource()
{
    part->slotViewDocumentSource();
}

void KHTMLPartIface::saveBackground(const QString &destination)
{
    KURL back = part->backgroundURL();
    if (back.isEmpty())
        return;

    KIO::MetaData metaData;
    metaData["referrer"] = part->referrer();
    KHTMLPopupGUIClient::saveURL( back, destination, metaData );
}

void KHTMLPartIface::saveDocument(const QString &destination)
{
    KURL srcURL( part->url() );

    if ( srcURL.fileName(false).isEmpty() )
        srcURL.setFileName( "index.html" );

    KIO::MetaData metaData;
    // Referrer unknown?
    KHTMLPopupGUIClient::saveURL( srcURL, destination, metaData, part->cacheId() );
}

void KHTMLPartIface::setUserStyleSheet(const QString &styleSheet)
{
    part->setUserStyleSheet(styleSheet);
}

QString KHTMLPartIface::selectedText() const
{
    return part->selectedText();
}

void KHTMLPartIface::viewFrameSource()
{
    part->slotViewFrameSource();
}

QString KHTMLPartIface::evalJS(const QString &script) {
    return part->executeScript(script).toString();
}
