/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
    Copyright (C) 1998, 1999, 2000 KDE Team
    Copyright (C) 2008 Nick Shaforostoff <shaforostoff@kde.ru>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
 */

#define INCLUDE_MENUITEM_DEF

#include "kcheckaccelerators.h"

#include <config.h>

#include <QApplication>
#include <QCheckBox>
#include <QDialog>
#include <QShortcutEvent>
#include <QMouseEvent>
#include <QLayout>
#include <QMenuBar>
#include <QMetaObject>
#include <QPushButton>
#include <QTabBar>

#include <QLabel>
#include <QComboBox>
#include <QGroupBox>
#include <QClipboard>
#include <QProcess>

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kcomponentdata.h>
#include <klocale.h>
#include <kshortcut.h>
#include <ktextbrowser.h>

#include "kacceleratormanager.h"
#include <kconfiggroup.h>

void KCheckAccelerators::initiateIfNeeded(QObject* parent)
{
    KConfigGroup cg( KGlobal::config(), "Development" );
    QString sKey = cg.readEntry( "CheckAccelerators" ).trimmed();
    int key=0;
    if( !sKey.isEmpty() ) {
      KShortcut cuts( sKey );
      if( !cuts.isEmpty() )
        key = cuts.primary()[0];
    }
    bool autoCheck = cg.readEntry( "AutoCheckAccelerators", true );
    bool copyWidgetText = cg.readEntry( "CopyWidgetText", false );

    if (!copyWidgetText && key==0 && !autoCheck)
        return;

    new KCheckAccelerators(parent, key, autoCheck, copyWidgetText);
}

KCheckAccelerators::KCheckAccelerators(QObject* parent, int key_, bool autoCheck_, bool copyWidgetText_)
    : QObject(parent)
    , key(key_)
    , block(false)
    , autoCheck(autoCheck_)
    , copyWidgetText(copyWidgetText_)
    , drklash(0)
{
    setObjectName( "kapp_accel_filter" );

    KConfigGroup cg( KGlobal::config(), "Development" );
    alwaysShow = cg.readEntry( "AlwaysShowCheckAccelerators", false );
    copyWidgetTextCommand = cg.readEntry( "CopyWidgetTextCommand", "" );

    parent->installEventFilter( this );
    connect( &autoCheckTimer, SIGNAL(timeout()), SLOT(autoCheckSlot()));
}

bool KCheckAccelerators::eventFilter(QObject* obj, QEvent* e)
{
    if ( block )
        return false;

    switch ( e->type() ) { // just simplify debuggin
    case QEvent::ShortcutOverride:
        if ( key && (static_cast<QKeyEvent*>(e)->key() == key) ) {
    	    block = true;
	    checkAccelerators( false );
	    block = false;
	    e->accept();
	    return true;
	}
        break;
    case QEvent::ChildAdded:
    case QEvent::ChildRemoved:
        // Only care about widgets; this also avoids starting the timer in other threads
        if ( !static_cast<QChildEvent *>(e)->child()->isWidgetType() )
            break;
        // fall-through
    case QEvent::Resize:
    case QEvent::LayoutRequest:
    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate:
        if( autoCheck ) {
            autoCheckTimer.setSingleShot( true );
            autoCheckTimer.start( 20 ); // 20 ms
        }
        break;
    //case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
        if ( copyWidgetText && static_cast<QMouseEvent*>(e)->button() == Qt::MidButton ) {
            //kWarning()<<"obj"<<obj;
            QWidget* w=static_cast<QWidget*>(obj)->childAt(static_cast<QMouseEvent*>(e)->pos());
            if (!w)
                w=static_cast<QWidget*>(obj);
            if (!w)
                return false;
            //kWarning()<<"MouseButtonDblClick"<<w;
            QString text;
            if (qobject_cast<QLabel*>(w))
                text=static_cast<QLabel*>(w)->text();
            else if (qobject_cast<QAbstractButton*>(w))
                text=static_cast<QAbstractButton*>(w)->text();
            else if (qobject_cast<QComboBox*>(w))
                text=static_cast<QComboBox*>(w)->currentText();
            else if (qobject_cast<QTabBar*>(w))
                text=static_cast<QTabBar*>(w)->tabText(  static_cast<QTabBar*>(w)->tabAt(static_cast<QMouseEvent*>(e)->pos())  );
            else if (qobject_cast<QGroupBox*>(w))
                text=static_cast<QGroupBox*>(w)->title();
            else if (qobject_cast<QMenu*>(obj))
            {
                QAction* a=static_cast<QMenu*>(obj)->actionAt(static_cast<QMouseEvent*>(e)->pos());
                if (!a)
                    return false;
                text=a->text();
                if (text.isEmpty())
                    text=a->iconText();
            }
            if (text.isEmpty())
                return false;

            if (static_cast<QMouseEvent*>(e)->modifiers() == Qt::ControlModifier)
                text.remove('&');

            //kWarning()<<KGlobal::activeComponent().catalogName()<<text;
            if (copyWidgetTextCommand.isEmpty())
            {
                QClipboard *clipboard = QApplication::clipboard();
                clipboard->setText(text);
            }
            else
            {
                QProcess* script=new QProcess(this);
                script->start(copyWidgetTextCommand.arg(text).arg(KGlobal::activeComponent().catalogName()));
                connect(script,SIGNAL(finished(int,QProcess::ExitStatus)),script,SLOT(deleteLater()));
            }
            e->accept();
            return true;

            //kWarning()<<"MouseButtonDblClick"<<static_cast<QWidget*>(obj)->childAt(static_cast<QMouseEvent*>(e)->globalPos());
	}
        return false;
    case QEvent::Timer:
    case QEvent::MouseMove:
    case QEvent::Paint:
        return false;
    default:
        // kDebug(125) << "KCheckAccelerators::eventFilter " << e->type() << " " << autoCheck;
        break;
    }
    return false;
}

void KCheckAccelerators::autoCheckSlot()
{
    if( block )
    {
        autoCheckTimer.setSingleShot( true );
        autoCheckTimer.start( 20 );
        return;
    }
    block = true;
    checkAccelerators( !alwaysShow );
    block = false;
}

void KCheckAccelerators::createDialog(QWidget *actWin, bool automatic)
{
    if ( drklash )
        return;

    drklash = new QDialog( actWin );
    drklash->setAttribute( Qt::WA_DeleteOnClose );
    drklash->setObjectName( "kapp_accel_check_dlg" );
    drklash->setWindowTitle( i18nc("@title:window", "Dr. Klash' Accelerator Diagnosis" ));
    drklash->resize( 500, 460 );
    QVBoxLayout* layout = new QVBoxLayout( drklash );
    drklash_view = new KTextBrowser( drklash );
    layout->addWidget( drklash_view);
    QCheckBox* disableAutoCheck = NULL;
    if( automatic )  {
        disableAutoCheck = new QCheckBox( i18nc("@option:check","Disable automatic checking" ), drklash );
        connect(disableAutoCheck, SIGNAL(toggled(bool)), SLOT(slotDisableCheck(bool)));
        layout->addWidget( disableAutoCheck );
    }
    QPushButton* btnClose = new QPushButton( i18nc("@action:button", "Close" ), drklash );
    btnClose->setDefault( true );
    layout->addWidget( btnClose );
    connect( btnClose, SIGNAL(clicked()), drklash, SLOT(close()) );
    if (disableAutoCheck)
        disableAutoCheck->setFocus();
    else
        drklash_view->setFocus();
}

void KCheckAccelerators::slotDisableCheck(bool on)
{
    autoCheck = !on;
    if (!on)
        autoCheckSlot();
}

void KCheckAccelerators::checkAccelerators( bool automatic )
{
    QWidget* actWin = qApp->activeWindow();
    if ( !actWin )
        return;

    KAcceleratorManager::manage(actWin);
    QString a, c, r;
    KAcceleratorManager::last_manage(a, c,  r);

    if (automatic) // for now we only show dialogs on F12 checks
        return;

    if (c.isEmpty() && r.isEmpty() && (automatic || a.isEmpty()))
        return;

    QString s;

    if ( ! c.isEmpty() )  {
        s += i18n("<h2>Accelerators changed</h2>");
        s += "<table border><tr><th><b>Old Text</b></th><th><b>New Text</b></th></tr>"
             + c + "</table>";
    }

    if ( ! r.isEmpty() )  {
        s += i18n("<h2>Accelerators removed</h2>");
        s += "<table border><tr><th><b>Old Text</b></th></tr>" + r + "</table>";
    }

    if ( ! a.isEmpty() )  {
        s += i18n("<h2>Accelerators added (just for your info)</h2>");
        s += "<table border><tr><th><b>New Text</b></th></tr>" + a + "</table>";
    }

    createDialog(actWin, automatic);
    drklash_view->setHtml(s);
    drklash->show();
    drklash->raise();

    // dlg will be destroyed before returning
}

#include "kcheckaccelerators.moc"
