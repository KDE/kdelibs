/**
 * This file is part of the HTML widget for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */
#include <config.h>
#include <klocale.h>

#include <kdebug.h>

#include "rendering/render_applet.h"
#include "rendering/render_canvas.h"
#include "xml/dom_docimpl.h"
#include "khtmlview.h"
#include "khtml_part.h"

#include <qlabel.h>

#ifndef Q_WS_QWS // We don't have Java in Qt Embedded

#include "java/kjavaappletwidget.h"
#include "misc/htmltags.h"
#include "html/html_objectimpl.h"

using namespace khtml;
using namespace DOM;

RenderApplet::RenderApplet(HTMLElementImpl *applet, const QMap<QString, QString> &args )
    : RenderWidget(applet)
{
    // init RenderObject attributes
    setInline(true);

    KJavaAppletContext *context = 0;
    KHTMLView *_view = applet->getDocument()->view();
    if ( _view ) {
        KHTMLPart *part = _view->part();
        context = part->createJavaContext();
    }

    if ( context ) {
        //kdDebug(6100) << "RenderApplet::RenderApplet, setting QWidget" << endl;
        setQWidget( new KJavaAppletWidget(context, _view->viewport()) );
        processArguments(args);
    }
}

RenderApplet::~RenderApplet()
{
}

short RenderApplet::intrinsicWidth() const
{
    int rval = 150;

    if( m_widget )
        rval = ((KJavaAppletWidget*)(m_widget))->sizeHint().width();

    return rval > 10 ? rval : 50;
}

int RenderApplet::intrinsicHeight() const
{
    int rval = 150;

    if( m_widget )
        rval = m_widget->sizeHint().height();

    return rval > 10 ? rval : 50;
}

void RenderApplet::layout()
{
    //kdDebug(6100) << "RenderApplet::layout" << endl;

    KHTMLAssert( needsLayout() );
    KHTMLAssert( minMaxKnown() );

    calcWidth();
    calcHeight();

    KJavaAppletWidget *tmp = static_cast<KJavaAppletWidget*>(m_widget);
    if ( tmp ) {
        NodeImpl *child = element()->firstChild();

        while(child) {

            if(child->id() == ID_PARAM) {
                HTMLParamElementImpl *p = static_cast<HTMLParamElementImpl *>(child);
                if(tmp->applet())
                    tmp->applet()->setParameter( p->name(), p->value());
            }
            child = child->nextSibling();
        }
        //kdDebug(6100) << "setting applet widget to size: " << m_width << ", " << m_height << endl;
        m_widget->resize(m_width-borderLeft()-borderRight()-paddingLeft()-paddingRight(),
                         m_height-borderTop()-borderBottom()-paddingTop()-paddingBottom());
        tmp->showApplet();
    }

    setNeedsLayout(false);
}

void RenderApplet::processArguments(const QMap<QString, QString> &args)
{
    KJavaAppletWidget *w = static_cast<KJavaAppletWidget*>(m_widget);
    KJavaApplet* applet = w ? w->applet() : 0;

    if ( applet ) {
        applet->setBaseURL( args[QString::fromLatin1("baseURL") ] );
        applet->setAppletClass( args[QString::fromLatin1("code") ] );

	QString str = args[QString::fromLatin1("codeBase") ];
        if( !str.isEmpty() )
            applet->setCodeBase( str );

	str = args[QString::fromLatin1("name") ];
        if( !str.isNull() )
            applet->setAppletName( str );
        else
            applet->setAppletName( args[QString::fromLatin1("code") ] );

	str = args[QString::fromLatin1("archive") ];
        if( !str.isEmpty() )
            applet->setArchives( args[QString::fromLatin1("archive") ] );
    }
}

#endif
