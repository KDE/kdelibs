/*  This file is part of the KDE Libraries
 *  Copyright (C) 2004 Peter Rockai (mornfall) <mornfall@danill.sk>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kwhatsthismanager_p.h"
#include <kdebug.h>

#include <QtCore/QVariant>
#include <QtGui/QApplication>
#include <QtGui/QWhatsThis>
#include <QtGui/QWhatsThisClickedEvent>

#include <klocale.h>
#include <ktoolinvocation.h>

KWhatsThisManager *KWhatsThisManager::s_instance = 0;

QString KWhatsThisManager::text() const
{
  QString txt = i18n ("<b>Not Defined</b><br/>There is no \"What's This\""
          " help assigned to this widget. If you want to help us to"
          " describe the widget, you are welcome to <a href=\"submit"
          "-whatsthis\">send us your own \"What's This\" help</a> for it.");
  return txt;
}

void KWhatsThisManager::clicked( const QString& href, QWidget *widget )
{
  if ( href == "submit-whatsthis" ) {
    QString body;
    body.append( QString( "Widget text: '%1'\n" ).arg( widget->property( "text" ).toString() ) );

    QString dsc = QString( "current --> %1" ).arg( widget->objectName() );
    dsc.append( QString( " (%1)\n" ).arg( widget->metaObject()->className() ) );

    QWidget *w;
    for ( w = widget; w && w != widget->topLevelWidget(); w = w->parentWidget() ) {
      dsc.append( w->objectName() );
      dsc.append( QString( " (%1)\n" ).arg( w->metaObject()->className() ) );
    }

    w = widget->topLevelWidget();

    if ( w ) {
      dsc.append( "toplevel --> " );
      dsc.append( w->objectName() );
      dsc.append( QString( " (%1)\n" ).arg( w->metaObject()->className() ) );
    }

    body.append( dsc );

    QString subject( "What's This submission: " );
    subject.append( qApp->argv()[ 0 ] );

    body.append( "\nPlease type in your what's this help between these lines: "
                 "\n--%-----------------------------------------------------------------------\n"
                 "\n--%-----------------------------------------------------------------------" );

    KToolInvocation::invokeMailer( "quality-whatsthis@kde.org", "", "", subject, body );
  }
}

void KWhatsThisManager::init()
{
  if ( s_instance )
    return;

  s_instance = new KWhatsThisManager;
}

KWhatsThisManager::KWhatsThisManager()
{
  // go away...
  qApp->installEventFilter( this );
}

bool KWhatsThisManager::eventFilter( QObject *object, QEvent *event )
{
    if ( event->type() == QEvent::WhatsThis ) {
        QHelpEvent *he = static_cast<QHelpEvent* >(event);
        QWidget *widget = qobject_cast<QWidget*>( object );
        if ( widget ) {
            QHelpEvent queryEvent(QEvent::QueryWhatsThis, he->pos(), he->globalPos());
            const bool sentEvent = QApplication::sendEvent(widget, &queryEvent);
            if (!sentEvent || !queryEvent.isAccepted()) {
                // No whats-this defined by the widget (at this position): show fallback one
                QWhatsThis::showText(he->globalPos(), text(), widget);
            }
        }
    } else if ( event->type() == QEvent::WhatsThisClicked ) {
        QWhatsThisClickedEvent *wte = static_cast<QWhatsThisClickedEvent* >(event);
        QWidget *widget = qobject_cast<QWidget*>( object );
        if ( widget ) {
            clicked( wte->href(), widget );
            return true;
        }
    }

    return false;
}

#include "kwhatsthismanager_p.moc"

