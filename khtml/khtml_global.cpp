/* This file is part of the KDE project
 *
 * Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>
 * Copyright (C) 2007 David Faure <faure@kde.org>
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

#include "khtml_global.h"
#include "khtml_part.h"
#include "khtml_settings.h"

#include "css/cssstyleselector.h"
#include "css/css_mediaquery.h"
#include "html/html_imageimpl.h"
#include "rendering/render_style.h"
#include "rendering/break_lines.h"
#include "misc/htmlnames.h"
#include "misc/loader.h"
#include "misc/arena.h"
#include "misc/paintbuffer.h"

#include <QtCore/QLinkedList>

#include <kcomponentdata.h>
#include <kiconloader.h>
#include <kaboutdata.h>
#include <klocale.h>

#include <assert.h>

#include <kdebug.h>

// SVG
#include "svg/SVGNames.h"

KHTMLGlobal *KHTMLGlobal::s_self = 0;
unsigned long int KHTMLGlobal::s_refcnt = 0;
KComponentData *KHTMLGlobal::s_componentData = 0;
KIconLoader *KHTMLGlobal::s_iconLoader = 0;
KAboutData *KHTMLGlobal::s_about = 0;
KHTMLSettings *KHTMLGlobal::s_settings = 0;

static QLinkedList<KHTMLPart*> *s_parts = 0;
static QLinkedList<DOM::DocumentImpl*> *s_docs = 0;

KHTMLGlobal::KHTMLGlobal()
{
    assert(!s_self);
    s_self = this;
    ref();

    khtml::Cache::init();

    khtml::NamespaceFactory::initIdTable();
    khtml::PrefixFactory::initIdTable();
    khtml::LocalNameFactory::initIdTable();
    DOM::emptyLocalName = DOM::LocalName::fromId(0);
    DOM::emptyPrefixName = DOM::PrefixName::fromId(0);
    DOM::emptyNamespaceName = DOM::NamespaceName::fromId(DOM::emptyNamespace);
    WebCore::SVGNames::init();
}

KHTMLGlobal::~KHTMLGlobal()
{
    //kDebug(6000) << this;
    if ( s_self == this )
    {
        finalCheck();
        delete s_iconLoader;
        delete s_componentData;
        delete s_about;
        delete s_settings;
        delete KHTMLSettings::avFamilies;
        if (s_parts) {
            assert(s_parts->isEmpty());
            delete s_parts;
        }
        if (s_docs) {
            assert(s_docs->isEmpty());
            delete s_docs;
        }

        s_iconLoader = 0;
        s_componentData = 0;
        s_about = 0;
        s_settings = 0;
        s_parts = 0;
        s_docs = 0;
        KHTMLSettings::avFamilies = 0;

        // clean up static data
        khtml::CSSStyleSelector::clear();
        khtml::RenderStyle::cleanup();
        khtml::RenderObject::cleanup();
        khtml::PaintBuffer::cleanup();
        khtml::MediaQueryEvaluator::cleanup();
        khtml::Cache::clear();
        khtml::cleanup_thaibreaks();
        khtml::ArenaFinish();
    }
    else
        deref();
}

void KHTMLGlobal::ref()
{
    if ( !s_refcnt && !s_self )
    {
        //kDebug(6000) << "Creating KHTMLGlobal instance";
        // we can't use a staticdeleter here, because that would mean
        // that the KHTMLGlobal instance gets deleted from within a qPostRoutine, called
        // from the QApplication destructor. That however is too late, because
        // we want to destruct a KComponentData object, which involves destructing
        // a KConfig object, which might call KGlobal::dirs() (in sync()) which
        // probably is not going to work ;-)
        // well, perhaps I'm wrong here, but as I'm unsure I try to stay on the
        // safe side ;-) -> let's use a simple reference counting scheme
        // (Simon)
        new KHTMLGlobal; // does initial ref()
    } else {
        ++s_refcnt;
    }
    //kDebug(6000) << "s_refcnt=" << s_refcnt;
}

void KHTMLGlobal::deref()
{
    //kDebug(6000) << "s_refcnt=" << s_refcnt - 1;
    if ( !--s_refcnt && s_self )
    {
        delete s_self;
        s_self = 0;
    }
}

void KHTMLGlobal::registerPart( KHTMLPart *part )
{
    //kDebug(6000) << part;
    if ( !s_parts )
        s_parts = new QLinkedList<KHTMLPart*>;

    if ( !s_parts->contains( part ) ) {
        s_parts->append( part );
        ref();
    }
}

void KHTMLGlobal::deregisterPart( KHTMLPart *part )
{
    //kDebug(6000) << part;
    assert( s_parts );

    if ( s_parts->removeAll( part ) ) {
        if ( s_parts->isEmpty() ) {
            delete s_parts;
            s_parts = 0;
        }
        deref();
    }
}

void KHTMLGlobal::registerDocumentImpl( DOM::DocumentImpl *doc )
{
    //kDebug(6000) << doc;
    if ( !s_docs )
        s_docs = new QLinkedList<DOM::DocumentImpl*>;

    if ( !s_docs->contains( doc ) ) {
        s_docs->append( doc );
        ref();
    }
}

void KHTMLGlobal::deregisterDocumentImpl( DOM::DocumentImpl *doc )
{
    //kDebug(6000) << doc;
    assert( s_docs );

    if ( s_docs->removeAll( doc ) ) {
        if ( s_docs->isEmpty() ) {
            delete s_docs;
            s_docs = 0;
        }
        deref();
    }
}

const KComponentData &KHTMLGlobal::componentData()
{
  assert( s_self );

  if ( !s_componentData )
  {
    s_about = new KAboutData( "khtml", 0, ki18n( "KHTML" ), "4.0",
                              ki18n( "Embeddable HTML component" ),
                              KAboutData::License_LGPL );
    s_about->addAuthor(ki18n("Lars Knoll"), KLocalizedString(), "knoll@kde.org");
    s_about->addAuthor(ki18n("Antti Koivisto"), KLocalizedString(), "koivisto@kde.org");
    s_about->addAuthor(ki18n("Waldo Bastian"), KLocalizedString(), "bastian@kde.org");
    s_about->addAuthor(ki18n("Dirk Mueller"), KLocalizedString(), "mueller@kde.org");
    s_about->addAuthor(ki18n("Peter Kelly"), KLocalizedString(), "pmk@kde.org");
    s_about->addAuthor(ki18n("Torben Weis"), KLocalizedString(), "weis@kde.org");
    s_about->addAuthor(ki18n("Martin Jones"), KLocalizedString(), "mjones@kde.org");
    s_about->addAuthor(ki18n("Simon Hausmann"), KLocalizedString(), "hausmann@kde.org");
    s_about->addAuthor(ki18n("Tobias Anton"), KLocalizedString(), "anton@stud.fbi.fh-darmstadt.de");

    s_componentData = new KComponentData( s_about );
  }

  return *s_componentData;
}

KIconLoader *KHTMLGlobal::iconLoader()
{
  if ( !s_iconLoader )
  {
    s_iconLoader = new KIconLoader(componentData().componentName(), componentData().dirs());
  }

  return s_iconLoader;
}

KHTMLSettings *KHTMLGlobal::defaultHTMLSettings()
{
  assert( s_self );
  if ( !s_settings )
    s_settings = new KHTMLSettings();

  return s_settings;
}

void KHTMLGlobal::finalCheck()
{
#ifndef NDEBUG
    if (s_refcnt) {
        if (s_parts && !s_parts->isEmpty()) {
            Q_FOREACH(KHTMLPart *part, *s_parts) {
                kWarning(6000) << "Part" << part->url() << "was not deleted";
            }
        }
        if (s_docs && !s_docs->isEmpty()) {
            Q_FOREACH(DOM::DocumentImpl *doc, *s_docs) {
                kWarning(6000) << "Document" << doc->URL() << "was not deleted";
            }
        }
    }
#endif
}
