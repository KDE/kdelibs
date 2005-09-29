/* This file is part of the KDE libraries
   Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>

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

#include <kdebug.h>

#include "toolviewaccessor_p.h"
#include "guiclient.h"
#include "mainwindow.h"

#include "toolviewaccessor.h"
#include "toolviewaccessor.moc"

#include "toolviewaccessor_p.h"

namespace KMDI
{

ToolViewAccessor::ToolViewAccessor( KMDI::MainWindow *parent, QWidget *widgetToWrap, const QString& tabToolTip, const QString& tabCaption)
: QObject(parent)
{
	mdiMainFrm=parent;
	d=new KMDIPrivate::ToolViewAccessorPrivate();
	if (widgetToWrap->inherits("KDockWidget")) {
		d->widgetContainer=dynamic_cast<KDockWidget*>(widgetToWrap);
		d->widget=d->widgetContainer->getWidget();
	} else {
		d->widget=widgetToWrap;
        QString finalTabCaption;
        if (tabCaption == 0) {
            finalTabCaption = widgetToWrap->caption();
            if (finalTabCaption.isEmpty() && !widgetToWrap->icon()) {
                finalTabCaption = widgetToWrap->name();
            }
        }
        else {
            finalTabCaption = tabCaption;
        }
		d->widgetContainer= parent->createDockWidget( widgetToWrap->name(),
                                              (widgetToWrap->icon()?(*(widgetToWrap->icon())):QPixmap()),
                                              0L,  // parent
                                              widgetToWrap->caption(),
                                              finalTabCaption);
		d->widgetContainer->setWidget(widgetToWrap);
		if (tabToolTip!=0) {
			d->widgetContainer->setToolTipString(tabToolTip);
		}
	}

  //mdiMainFrm->m_toolViews.insert(d->widget,this);
	mdiMainFrm->m_guiClient->addToolView(this);
	d->widget->installEventFilter(this);
}

ToolViewAccessor::ToolViewAccessor( KMDI::MainWindow *parent) : QObject(parent) {
	mdiMainFrm=parent;
	d=new KMDIPrivate::ToolViewAccessorPrivate();
}

ToolViewAccessor::~ToolViewAccessor() {
  if (mdiMainFrm->m_toolViews)
  mdiMainFrm->m_toolViews->remove(d->widget);
	delete d;

}

QWidget *ToolViewAccessor::wrapperWidget() {
	if (!d->widgetContainer) {
		d->widgetContainer=mdiMainFrm->createDockWidget( "ToolViewAccessor::null",QPixmap());
		connect(d->widgetContainer,SIGNAL(widgetSet(QWidget*)),this,SLOT(setWidgetToWrap(QWidget*)));
	}
	return d->widgetContainer;
}

QWidget *ToolViewAccessor::wrappedWidget() {
	return d->widget;
}


void ToolViewAccessor::setWidgetToWrap(QWidget *widgetToWrap, const QString& tabToolTip, const QString& tabCaption)
{
	Q_ASSERT(!(d->widget));
	Q_ASSERT(!widgetToWrap->inherits("KDockWidget"));
	disconnect(d->widgetContainer,SIGNAL(widgetSet(QWidget*)),this,SLOT(setWidgetToWrap(QWidget*)));
	delete d->widget;
    d->widget=widgetToWrap;
	KDockWidget *tmp=d->widgetContainer;

    QString finalTabCaption;
    if (tabCaption == 0) {
        finalTabCaption = widgetToWrap->caption();
        if (finalTabCaption.isEmpty() && !widgetToWrap->icon()) {
            finalTabCaption = widgetToWrap->name();
        }
    }
    else {
        finalTabCaption = tabCaption;
    }

	if (!tmp) {
		tmp = mdiMainFrm->createDockWidget( widgetToWrap->name(),
			                        widgetToWrap->icon()?(*(widgetToWrap->icon())):QPixmap(),
			                        0L,  // parent
                                    widgetToWrap->caption(),
                                    finalTabCaption );
		d->widgetContainer= tmp;
		if (tabToolTip!=0) {
			d->widgetContainer->setToolTipString(tabToolTip);
		}
	}
    else {
		tmp->setCaption(widgetToWrap->caption());
		tmp->setTabPageLabel(finalTabCaption);
		tmp->setPixmap(widgetToWrap->icon()?(*(widgetToWrap->icon())):QPixmap());
		tmp->setName(widgetToWrap->name());
		if (tabToolTip!=0) {
			d->widgetContainer->setToolTipString(tabToolTip);
		}
	}
	tmp->setWidget(widgetToWrap);
	mdiMainFrm->m_toolViews->insert(widgetToWrap,this);
	mdiMainFrm->m_guiClient->addToolView(this);

	d->widget->installEventFilter(this);
}


bool ToolViewAccessor::eventFilter(QObject *o, QEvent *e) {
	if (e->type()==QEvent::IconChange) {
		d->widgetContainer->setPixmap(d->widget->icon()?(*d->widget->icon()):QPixmap());
	}
	return false;
}

void ToolViewAccessor::placeAndShow(KDockWidget::DockPosition pos, QWidget* pTargetWnd ,int percent)
{
	place(pos,pTargetWnd,percent);
	show();
}
void ToolViewAccessor::place(KDockWidget::DockPosition pos, QWidget* pTargetWnd ,int percent)
{
    Q_ASSERT(d->widgetContainer);
    if (!d->widgetContainer) return;
    if (pos == KDockWidget::DockNone) {
        d->widgetContainer->setEnableDocking(KDockWidget::DockNone);
        d->widgetContainer->reparent(mdiMainFrm, Qt::WType_TopLevel | Qt::WType_Dialog, QPoint(0,0), mdiMainFrm->isVisible());
    }
    else {   // add (and dock) the toolview as DockWidget view

        KDockWidget* pCover = d->widgetContainer;

        KDockWidget* pTargetDock = 0L;
        if (pTargetWnd->inherits("KDockWidget") || pTargetWnd->inherits("KDockWidget_Compat::KDockWidget")) {
            pTargetDock = (KDockWidget*) pTargetWnd;
        }

        // Should we dock to ourself?
        bool DockToOurself = false;
        if (mdiMainFrm->getMainDockWidget()) {
            if (pTargetWnd == mdiMainFrm->getMainDockWidget()->getWidget()) {
                DockToOurself = true;
                pTargetDock = mdiMainFrm->getMainDockWidget();
            }
            else if (pTargetWnd == mdiMainFrm->getMainDockWidget()) {
                DockToOurself = true;
                pTargetDock = mdiMainFrm->getMainDockWidget();
            }
        }
        // this is not inheriting QWidget*, its plain impossible that this condition is true
        //if (pTargetWnd == this) DockToOurself = true;
        if (!DockToOurself) if(pTargetWnd != 0L) {
            pTargetDock = mdiMainFrm->dockManager->findWidgetParentDock( pTargetWnd);
            if (!pTargetDock) {
                if (pTargetWnd->parentWidget()) {
                    pTargetDock = mdiMainFrm->dockManager->findWidgetParentDock( pTargetWnd->parentWidget());
                }
            }
        }
      /*  if (!pTargetDock || pTargetWnd == mdiMainFrm->getMainDockWidget()) {
            if (mdiMainFrm->m_managedDockPositionMode && (mdiMainFrm->m_pMdi || mdiMainFrm->m_documentTabWidget)) {
                KDockWidget *dw1=pTargetDock->findNearestDockWidget(pos);
                if (dw1)
                    pCover->manualDock(dw1,KDockWidget::DockCenter,percent);
                else
                    pCover->manualDock ( pTargetDock, pos, 20 );
                return;
            }
    }*/ //TODO
        pCover->manualDock( pTargetDock, pos, percent);
//check      pCover->show();
    }
}

void ToolViewAccessor::hide() {
	Q_ASSERT(d->widgetContainer);
	if (!d->widgetContainer) return;
	d->widgetContainer->undock();
}

void ToolViewAccessor::show() {
	Q_ASSERT(d->widgetContainer);
	if (!d->widgetContainer) return;
	d->widgetContainer->makeDockVisible();
}

}

// kate: space-indent on; indent-width 2; replace-tabs on;
