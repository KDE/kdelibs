//----------------------------------------------------------------------------
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Library General Public License as
//    published by the Free Software Foundation; either version 2 of the
//    License, or (at your option) any later version.
//
//----------------------------------------------------------------------------

#ifndef _KMDI_TABWIDGET_H_
#define _KMDI_TABWIDGET_H_

#include <ktabwidget.h>
#include <kmdi/global.h>

class KPopupMenu;

namespace KMDI
{

//KDE4: Add a d pointer
class TabWidget :
 public KTabWidget
{
	Q_OBJECT
public:
	TabWidget(QWidget* parent, const char* name=0);
	~TabWidget();

	virtual void addTab ( QWidget * child, const QString & label );
	virtual void addTab ( QWidget * child, const QIconSet & iconset, const QString & label );
	virtual void addTab ( QWidget * child, QTab * tab );
	virtual void insertTab ( QWidget * child, const QString & label, int index = -1 );
	virtual void insertTab ( QWidget * child, const QIconSet & iconset, const QString & label, int index = -1 );
	virtual void insertTab ( QWidget * child, QTab * tab, int index = -1 );
	virtual void removePage ( QWidget * w );

	KMDI::TabWidgetVisibility tabWidgetVisibility();
	void setTabWidgetVisibility( KMDI::TabWidgetVisibility );

private slots:
	void closeTab(QWidget* w);
public slots:
	void updateIconInView(QWidget*,QPixmap);
	void updateCaptionInView(QWidget*,const QString&);
private:
	void maybeShow();
	KMDI::TabWidgetVisibility m_visibility;
};

}

#endif
