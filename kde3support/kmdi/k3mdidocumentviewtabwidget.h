//----------------------------------------------------------------------------
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Library General Public License as
//    published by the Free Software Foundation; either version 2 of the
//    License, or (at your option) any later version.
//
//----------------------------------------------------------------------------

#ifndef _K3MDI_DOCUMENT_VIEW_TAB_WIDGET_H_
#define _K3MDI_DOCUMENT_VIEW_TAB_WIDGET_H_

#include <ktabwidget.h>
#include <k3mdidefines.h>

class KPopupMenu;

//KDE4: Add a d pointer
/**
 * A reimplementation of KTabWidget for K3MDI
 */
class KDE3SUPPORT_EXPORT K3MdiDocumentViewTabWidget:
			public KTabWidget
{
	Q_OBJECT
public:
	K3MdiDocumentViewTabWidget( QWidget* parent, const char* name = 0 );
	~K3MdiDocumentViewTabWidget();

	/**
	 * Add a tab into the tabwidget
	 * \sa QTabWidget
	 * \sa KTabWidget
	 */
	virtual void addTab ( QWidget * child, const QString & label );

	/**
	 * Add a tab into the tabwidget
	 * \sa QTabWidget
	 * \sa KTabWidget
	 */
	virtual void addTab ( QWidget * child, const QIcon & iconset, const QString & label );

#if 0
//FIXME: Qt4 does not have QTab's
	/**
	 * Add a tab into the tabwidget
	 * \sa QTabWidget
	 * \sa KTabWidget
	 */
	virtual void addTab ( QWidget * child, QTab * tab );
#endif
	/**
	 * Insert a tab into the tabwidget with a label
	 * \sa QTabWidget
	 * \sa KTabWidget
	 */
	virtual void insertTab ( QWidget * child, const QString & label, int index = -1 );

	/**
	 * Inserts a tab into the tabwidget with an icon and label
	 * \sa QTabWidget
	 * \sa KTabWidget
	 */
	virtual void insertTab ( QWidget * child, const QIcon & iconset, const QString & label, int index = -1 );

#if 0
//FIXME: Qt4 does not have QTab's
	/**
	 * Inserts a tab into the tabwidget
	 * \sa QTabWidget
	 * \sa KTabWidget
	 */
	virtual void insertTab ( QWidget * child, QTab * tab, int index = -1 );
#endif

	/**
	 * Removes the tab from the tabwidget
	 * \sa QTabWidget
	 * \sa KTabWidget
	 */
	virtual void removePage ( QWidget * w );

	/**
	 * Set the tab widget's visibility and then make the change
	 * to match the new setting
	 */
	K3Mdi::TabWidgetVisibility tabWidgetVisibility();

	/** Get the tab widget's visibility */
	void setTabWidgetVisibility( K3Mdi::TabWidgetVisibility );

private slots:

	/** Close the tab specified by w */
	void closeTab( QWidget* w );
	
	/** Moves a tab. Reimplemented for internal reasons. */
	void moveTab( int from, int to );

public slots:

	/** Change the icon for the tab */
	void updateIconInView( QWidget*, QPixmap );

	/** Change the caption for the tab */
	void updateCaptionInView( QWidget*, const QString& );

private:

	/**
	 * Determine whether or not we should show the tab bar
	 * The tab bar is hidden if it's determined that it should be hidden
	 * and shown if it's determined that it should be shown
	 */
	void maybeShow();

	K3Mdi::TabWidgetVisibility m_visibility;
	
signals:
	void initiateTabMove(int, int);	
};



#endif 
// kate: space-indent off; tab-width 4; replace-tabs off; indent-mode csands;

