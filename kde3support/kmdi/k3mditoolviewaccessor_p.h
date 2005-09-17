//----------------------------------------------------------------------------
//    filename             : k3mditoolviewaccessor_p.h
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


#ifndef K3MDI_TOOLVIEW_ACCESSOR_PRIVATE_H_
#define K3MDI_TOOLVIEW_ACCESSOR_PRIVATE_H_


#include <qwidget.h>
#include <k3dockwidget.h>
#include <qpointer.h>
#include <kaction.h>

class KDE3SUPPORT_EXPORT K3MdiToolViewAccessorPrivate {
public:
	K3MdiToolViewAccessorPrivate() {
		widgetContainer=0;
		widget=0;
	}
	~K3MdiToolViewAccessorPrivate() {
		delete action;
		if (!widgetContainer.isNull()) widgetContainer->undock();
		delete (K3DockWidget*)widgetContainer;
	}
	QPointer<K3DockWidget> widgetContainer;
	QWidget* widget;
	QPointer<KAction> action;
};


#endif


