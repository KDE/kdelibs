//----------------------------------------------------------------------------
//    filename             : k3mditoolviewaccessor.h
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 08/2003       by Joseph Wenninger (jowenn@kde.org)
//    changes              : ---
//    patches              : ---
//
//    copyright            : (C) 2003 by Joseph Wenninger (jowenn@kde.org)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Library General Public License as
//    published by the Free Software Foundation; either version 2 of the
//    License, or (at your option) any later version.
//
//----------------------------------------------------------------------------
#ifndef _K3MDITOOLVIEWACCESSOR_H_
#define _K3MDITOOLVIEWACCESSOR_H_

#include <qwidget.h>
#include <qpixmap.h>
#include <qrect.h>
#include <qapplication.h>
#include <qdatetime.h>

#include <k3dockwidget.h>

namespace K3MDIPrivate
{
class K3MDIGUIClient;
}


class KDE3SUPPORT_EXPORT K3MdiToolViewAccessor : public QObject
{
	Q_OBJECT


	friend class K3MdiMainFrm;
	friend class K3MDIPrivate::K3MDIGUIClient;

private:
	/**
	* Internally used by K3MdiMainFrm to store a temporary information that the method
	* activate() is unnecessary and that it can by escaped.
	* This saves from unnecessary calls when activate is called directly.
	*/
	bool m_bInterruptActivation;
	/**
	* Internally used to prevent cycles between K3MdiMainFrm::activateView() and K3MdiChildView::activate().
	*/
	bool m_bMainframesActivateViewIsPending;
	/**
	*
	*/
	bool m_bFocusInEventIsPending;

private:
	K3MdiToolViewAccessor( class K3MdiMainFrm *parent , QWidget *widgetToWrap, const QString& tabToolTip = 0, const QString& tabCaption = 0 );
	K3MdiToolViewAccessor( class K3MdiMainFrm *parent );
public:
	~K3MdiToolViewAccessor();
	QWidget *wrapperWidget();
	QWidget *wrappedWidget();
	void place( K3DockWidget::DockPosition pos = K3DockWidget::DockNone, QWidget* pTargetWnd = 0L, int percent = 50 );
	void placeAndShow( K3DockWidget::DockPosition pos = K3DockWidget::DockNone, QWidget* pTargetWnd = 0L, int percent = 50 );
	void show();
public Q_SLOTS:
	void setWidgetToWrap( QWidget* widgetToWrap, const QString& tabToolTip = 0, const QString& tabCaption = 0 );
	void hide();
private:
	class K3MdiToolViewAccessorPrivate *d;
	class K3MdiMainFrm *mdiMainFrm;

protected:
	bool eventFilter( QObject *o, QEvent *e );
};


#endif //_K3MDITOOLVIEWACCESSOR_H_ 
// kate: space-indent off; tab-width 4; replace-tabs off; indent-mode csands;

