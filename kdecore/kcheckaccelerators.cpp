/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
    Copyright (C) 1998, 1999, 2000 KDE Team

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

// $Id$

#define INCLUDE_MENUITEM_DEF
#include <qmenudata.h>

#include "config.h"

#include "kcheckaccelerators.h"

#include <qpopupmenu.h>
#include <qapplication.h>
#include <qdialog.h>
#include <qlayout.h>
#include <qtextview.h>
#include <qobjectlist.h>
#include <qmenubar.h>
#include <qtabbar.h>
#include <qpushbutton.h>
#include <qmetaobject.h>
#include <qcheckbox.h>

#include <kconfig.h>
#include <kglobal.h>
#include <kshortcut.h>
#include <klocale.h>

/*

 HOWTO:

 This class allows translators (and application developers) to check for accelerator
 conflicts in menu and widgets. Put the following in your kdeglobals (or the config
 file for the application you're testing):

 [Development]
 CheckAccelerators=F12
 AutoCheckAccelerators=false
 AlwaysShowCheckAccelerators=false

 The checking can be either manual or automatic. To perform manual check, press
 the keyboard shortcut set to 'CheckAccelerators' (here F12). If automatic checking
 is enabled by setting 'AutoCheckAccelerators' to true, check will be performed every
 time the GUI changes. It's possible that in certain cases the check will be
 done also when no visible changes in the GUI happen or the check won't be done
 even if the GUI changed (in the latter case, use manual check ). Automatic
 checks can be anytime disabled by the checkbox in the dialog presenting
 the results of the check. If you set 'AlwaysShowCheckAccelerators' to true,
 the dialog will be shown even if the automatic check didn't find any conflicts,
 and all submenus will be shown, even those without conflicts.

 The dialog first lists the name of the window, then all results for all menus
 (if the window has a menubar) and then result for all controls in the active
 window (if there are any checkboxes etc.). For every submenu and all controls
 there are shown all conflicts grouped by accelerator, and a list of all used
 accelerators.
*/

KCheckAccelerators::KCheckAccelerators( QObject* parent )
    : QObject( parent, "kapp_accel_filter" ), block( false )
{
    parent->installEventFilter( this );
    KConfigGroupSaver saver( KGlobal::config(), "Development" );
    QString sKey = KGlobal::config()->readEntry( "CheckAccelerators" ).stripWhiteSpace();
    if( !sKey.isEmpty() ) {
      KShortcut cuts( sKey );
      if( cuts.count() > 0 )
        key = cuts.seq(0).qt();
    }
    alwaysShow = KGlobal::config()->readBoolEntry( "AlwaysShowCheckAccelerators", false );
    autoCheck = KGlobal::config()->readBoolEntry( "AutoCheckAccelerators", true );
    connect( &autoCheckTimer, SIGNAL( timeout()), SLOT( autoCheckSlot()));
}

bool KCheckAccelerators::eventFilter( QObject * , QEvent * e) {
    if ( block )
        return false;
    if ( e->type() == QEvent::Accel ) {
        if ( ( static_cast<QKeyEvent *>(e) )->key() == key ) {
    	    block = true;
	    checkAccelerators( false );
	    block = false;
	    ( static_cast<QKeyEvent *>(e) )->accept();
	    return true;
	}
    }
    if( autoCheck
        && ( e->type() == QEvent::ChildInserted || e->type() == QEvent::ChildRemoved )) {
        autoCheckTimer.start( 100, true ); // 100 ms
    }
    return false;
}

void KCheckAccelerators::autoCheckSlot()
    {
    if( block || QWidget::mouseGrabber() || QWidget::keyboardGrabber() || QApplication::activePopupWidget()) {
        autoCheckTimer.start( 100, true );
        return;
    }
    block = true;
    checkAccelerators( true );
    block = false;
    }

void KCheckAccelerators::findAccel( const QString& item, const QString &txt, AccelMap &accels ) {
    QChar c;
    int search_pos = 0;
    for(;;) {
        int i = txt.find( "&", search_pos );
        if ( i == -1 )
            return;
        c = txt[ i + 1 ];
        if ( !c.isNull() && c != '&')
            break;
        search_pos = i + 2; // search also after '&&'
    }
    c = c.lower();
    AccelMap::Iterator it = accels.find( c );
    AccelInfo info;
    info.item  = item;
    info.string = txt;
    if ( it == accels.end() ) {
        AccelInfoList list;
        list.append( info );
        accels.insert( c, list );
    } else {
        AccelInfoList &list = it.data();
        list.append( info );
    }
}

void KCheckAccelerators::checkMenuData( const QString& prefix, QMenuData* m ) {
    AccelMap accels;
    QMenuItem* mi;
    int i;
    QString s;
    for ( i = 0; i < (int) m->count(); i++ ) {
        mi = m->findItem( m->idAt( i ) );
        s = mi->text();
        if ( s.contains( '\t' ) )
    	    s = s.left( s.find( '\t' ) );
	findAccel( prefix.isEmpty() ? s : prefix + "/" + s, s, accels );
    }

    menuAccels[ prefix ] = accels;

    for ( i = 0; i < (int) m->count(); i++ ) {
        mi = m->findItem( m->idAt( i ) );
        if ( mi->popup() ) {
    	    s = mi->text();
	    if ( s.contains( '\t' ) )
	        s = s.left( s.find( '\t' ) );
	    checkMenuData( prefix.isEmpty() ? s : prefix + "/" + s, mi->popup());
	}
    }
}

void KCheckAccelerators::checkMenuData( QMenuData* m ) {
    checkMenuData( "", m );
}

void KCheckAccelerators::checkAccelerators( bool automatic ) {
    QWidget* actWin = qApp->activeWindow();
    if ( !actWin )
        return;
    QMap<QChar, AccelInfoList > accels;
    QObjectList *l = actWin->queryList( "QWidget" );
    if ( !l )
        return;
    QMenuBar* mbar = 0;
    QObject *p;
    for ( QObject *o = l->first(); o; o = l->next() ) {
        if ( ( static_cast<QWidget *>(o) )->isVisibleTo( actWin ) ) {
    	    QWidget *w = static_cast<QWidget *>(o);
	    if ( w->inherits( "QMenuBar" ) ) {
	        mbar = static_cast<QMenuBar *>(w);
                for( unsigned int i = 0;
                     i < mbar->count();
                     ++i )
                    findAccel( w->className(), mbar->text( mbar->idAt( i )), accels );
            }
	    if (w->inherits("QLineEdit") || w->inherits("QComboBox") || w->inherits("QTextEdit") || w->inherits("QTextView") )
		continue;

	    // Skip widgets that are children of non-visible objects
	    p = w->parent();
	    while ( p && p->inherits("QWidget") && static_cast<QWidget *>(p)->isVisible() )
		p = p->parent();
	    if ( p )
		continue;

	    QMetaObject *mo = w->metaObject();
	    const QMetaProperty* text = mo->property( mo->findProperty( "text", TRUE ), TRUE );
	    const QMetaProperty* title = mo->property( mo->findProperty( "title", TRUE ), TRUE );
	    if ( text )
	        findAccel( w->className(), w->property( "text" ).toString(), accels );
	    if ( title )
	        findAccel( w->className(), w->property( "title" ).toString(), accels );

    	    if ( w->inherits( "QTabBar" ) ) {
    	        QTabBar *tbar = static_cast<QTabBar *>(w);
		for ( int i = 0; i < tbar->count(); i++ )
		    findAccel( tbar->className(), tbar->tabAt( i )->text(), accels );
	    }
	}
    }
    delete l;

    QString s;

    bool was_clash = false;
    int num_clashes = 0;
    QString used;
    for ( QMap<QChar,AccelInfoList>::Iterator it = accels.begin(); it != accels.end(); ++it  ) {
        AccelInfoList list = it.data();
        if( used.isEmpty())
            used = it.key();
        else {
            used += ", ";
            used += it.key();
        }

        if ( list.count() <= 1 )
      	    continue;

	if ( ++num_clashes == 1 ) {
	    s += "<table border>";
	    s += "<tr><th>" + i18n( "Accel" ) + "</th><th>" + i18n( "String" ) + "</th><th>" + i18n( "Widget" ) + "</th></tr>";
	}
	AccelInfoList::Iterator ait = list.begin();
	s += "<tr><td rowspan=" + QString::number( list.count() ) + "><large><b>" + it.key() + "</b></large></td>";
	s += "<td>";
	s += (*ait).string;
	s += "</td><td>";
	s += (*ait).item;
	s += "</td></tr>";

	for ( ait++; ait != list.end(); ++ait ) {
	    s += "<tr><td>";
	    s += (*ait).string;
	    s += "</td><td>";
	    s += (*ait).item;
	    s += "</td></tr>";
	}
    }
    if ( num_clashes  ) {
        s += "</table>";
        s.prepend( "<h3>" + i18n( "One clash detected", "%n clashes detected", num_clashes ) + "</h3>" );
    } else {
        s += "<h3>" + i18n( "No clashes detected" ) + "</h3>";
    }

    s += "<h3>" + i18n( "Used accelerators:" ) + "</h3> " + ( used.isEmpty() ? i18n( "None" ) : used );
    was_clash |= ( num_clashes > 0 );

    if ( mbar ) {
	checkMenuData( mbar );
        QString s2;
        for( QMap<QString,AccelMap>::Iterator mit = menuAccels.begin(); mit != menuAccels.end(); ++mit ) {
            if( mit.key().isEmpty()) // don't list the menubar itself, it's already handled
                continue;           //  together with other controls, do only submenus here
    	    num_clashes = 0;
            used = "";
            QString m;
	    for ( AccelMap::Iterator it = (*mit).begin(); it != (*mit).end(); ++it  ) {
	        AccelInfoList list = it.data();
                if( used.isEmpty())
                    used = it.key();
                else {
                    used += ", ";
                    used += it.key();
                }
	        if ( list.count() <= 1 )
	            continue;

    	        if ( ++num_clashes == 1 ) {
	            m += "<table border>";
	            m += "<tr><th>" + i18n( "Accel" ) + "</th><th>" + i18n( "Menu Item" ) + "</th></tr>";
	        }
	        AccelInfoList::Iterator ait = list.begin();
    	        m += "<tr><td rowspan=" + QString::number( list.count() ) + "><large><b>" + it.key() + "</b></large></td>";
    	        m += "<td>";
	        m += (*ait).item;
	        m += "</td></tr>";

	        for ( ait++; ait != list.end(); ++ait ) {
		    m += "<tr><td>";
	            m += (*ait).item;
		    m += "</td></tr>";
	        }
	    }
    	    if ( num_clashes  ) {
    	        m += "</table>";
                m.prepend( "<h3>" + i18n( "One clash detected", "%n clashes detected", num_clashes ) + "</h3>" );
            }

            if( num_clashes || alwaysShow ) {
                m.prepend( "<h2>" + i18n( "Submenu" ) + " " + mit.key() + "</h2>" );
                m += "<h3>" + i18n( "Used accelerators:" ) + "</h3> " + ( used.isEmpty() ? i18n( "None" ) : used );
            }
            s2 += m;
            was_clash |= ( num_clashes > 0 );
        }

        if( s2.isEmpty())
            s2 = "<h3>" + i18n( "No clashes detected" ) + "</h3>";
	s2.prepend( "<h2>" + i18n( "Menu" ) + "</h2>" );
	s2 += "<h2>" + i18n( "Other control elements" ) + "</h2>";
	s.prepend( s2 );
    }

    if( automatic && !alwaysShow && !was_clash )
        return;

    s.prepend( QString("<h2><em>") + actWin->caption() + "<em></h2>" );

    QDialog dlg( actWin, "kapp_accel_check_dlg", true );
    dlg.setCaption( i18n( "Dr. Klash' Accelerator Diagnosis" ));
    dlg.resize( 500, 460 );
    QVBoxLayout* layout = new QVBoxLayout( &dlg, 11, 6 );
    layout->setAutoAdd( TRUE );
    QTextView* view = new QTextView( &dlg );
    QCheckBox* disableAutoCheck = NULL;
    if( automatic )
        disableAutoCheck = new QCheckBox( i18n( "&Disable automatic checking" ), &dlg );
    QPushButton* btnClose = new QPushButton( i18n( "&Close" ), &dlg );
    btnClose->setDefault( true );
    connect( btnClose, SIGNAL( clicked() ), &dlg, SLOT( close() ) );
    view->setText( s );
    view->setFocus();
    dlg.exec();
    if( disableAutoCheck != NULL && disableAutoCheck->isChecked())
        autoCheck = false;
    // dlg will be destroyed before returning
}

#include "kcheckaccelerators.moc"
