//----------------------------------------------------------------------------
//    filename             : k3mditaskbar.h
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                           - 06/2000     stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                           2000-2003     maintained by the KDevelop project
//
//    copyright            : (C) 1999-2003 by Falk Brettschneider
//                                         and
//                                         Szymon Stefanek (stefanek@tin.it)
//    email                :  falkbr@kdevelop.org (Falk Brettschneider)
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

#ifndef _K3MDITASKBAR_H_
#define _K3MDITASKBAR_H_

#include <ktoolbar.h>
#include <q3ptrlist.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include "k3mdidefines.h"

class K3MdiMainFrm;
class K3MdiChildView;
class K3MdiTaskBar;

class K3MdiTaskBarButtonPrivate;

/**
  * @short Internal class.
  *
  * It's a special kind of QPushButton catching mouse clicks.
  * And you have the ability to abbreviate the text that it fits in the button.
  */
class KDE3SUPPORT_EXPORT K3MdiTaskBarButton : public QPushButton
{
	Q_OBJECT
	// methods
public:
	/**
	* Constructor (sets to toggle button, adds a tooltip (caption) and sets to NoFocus
	*/
	K3MdiTaskBarButton( K3MdiTaskBar *pTaskBar, K3MdiChildView *win_ptr );
	/**
	* Destructor
	*/
	~K3MdiTaskBarButton();
	/**
	* text() returns the possibly abbreviated text including the dots in it. But actualText() returns the full text.
	*/
	QString actualText() const;
	/**
	* Given the parameter newWidth this function possibly abbreviates the parameter string and sets a new button text.
	*/
	void fitText( const QString&, int newWidth );
	/**
	* Sets the text and avoids any abbreviation. Memorizes that text in m_actualText, too.
	*/
	void setText( const QString& );
Q_SIGNALS:
	/**
	* Emitted when the button has been clicked. Internally connected to setFocus of the according MDI view.
	*/
	void clicked( K3MdiChildView* );
	/**
	* Internally connected with K3MdiMainFrm::activateView
	*/
	void leftMouseButtonClicked( K3MdiChildView* );
	/**
	* Internally connected with K3MdiMainFrm::taskbarButtonRightClicked
	*/
	void rightMouseButtonClicked( K3MdiChildView* );
	/**
	* Emitted when the button text has changed. Internally connected with K3MdiTaskBar::layoutTaskBar
	*/
	void buttonTextChanged( int );
public Q_SLOTS:
	/**
	* A slot version of setText
	*/
	void setNewText( const QString& );
protected Q_SLOTS:
	/**
	* Reimplemented from its base class to catch right and left mouse button clicks
	*/
	void mousePressEvent( QMouseEvent* );

	// attributes
public:
	/**
	* The according MDI view
	*/
	K3MdiChildView *m_pWindow;
protected:
	/**
	* Internally we must remember the real text because the button text can be abbreviated.
	*/
	QString m_actualText;

private:
	K3MdiTaskBarButtonPrivate *d;
};


class K3MdiTaskBarPrivate;
/**
 * @short Internal class.
 *
 * It's a special kind of QToolBar that acts as taskbar for child views.
 * K3MdiTaskBarButtons can be added or removed dynamically.<br>
 * The button sizes are adjusted dynamically, as well.
 */
class KDE3SUPPORT_EXPORT K3MdiTaskBar : public KToolBar
{
	Q_OBJECT
public:
	/**
	* Constructor (NoFocus, minimum width = 1, an internal QPtrList of taskbar buttons (autodelete))
	*/
	K3MdiTaskBar( K3MdiMainFrm *parent, Qt::ToolBarDock dock );
	/**
	* Destructor (deletes the taskbar button list)
	*/
	~K3MdiTaskBar();
	/**
	*Add a new K3MdiTaskBarButton . The width doesn't change.
	* If there's not enough space, all taskbar buttons will be resized to a new smaller size.
	* Probably button texts must be abbreviated now.
	*/
	K3MdiTaskBarButton * addWinButton( K3MdiChildView *win_ptr );
	/**
	* Removes a K3MdiTaskBarButton and deletes it. If the rest of the buttons are smaller
	* than they usually are, all those buttons will be resized in a way that the new free size is used as well.
	*/
	void removeWinButton( K3MdiChildView *win_ptr, bool haveToLayoutTaskBar = true );
	/**
	* Returns the neighbor taskbar button of the taskbar button of the MDI view given by parameter
	* bRight specifies the side, of course left is used if bRight is false.
	*/
	K3MdiTaskBarButton * getNextWindowButton( bool bRight, K3MdiChildView *win_ptr );
	/**
	* Get the button belonging to the MDI view given as parameter.
	*/
	K3MdiTaskBarButton * getButton( K3MdiChildView *win_ptr );
	/**
	* Switch it on or off.
	*/
	void switchOn( bool bOn );
	/**
	* @return whether switched on or off.
	*/
	bool isSwitchedOn() const
	{
		return m_bSwitchedOn;
	};
protected:
	/**
	* Reimplemented from its base class to call layoutTaskBar, additionally.
	*/
	void resizeEvent( QResizeEvent* );
protected Q_SLOTS:
	/**
	* Checks if all buttons fits into this. If not, it recalculates all button widths
	* in a way that all buttons fits into the taskbar and have got equal width.
	* The text of the buttons will be abbreviated when nessecary, all buttons get a
	* fixed width and show() is called for each one.
	* If one drags the taskbar to a vertical orientation, the button width is set to 80 pixel.
	*/
	void layoutTaskBar( int taskBarWidth = 0 );
public Q_SLOTS:
	/**
	* Pushes the desired taskbar button down (switch on), the old one is released (switched off).
	* Actually it's a radiobutton group behavior.
	*/
	void setActiveButton( K3MdiChildView *win_ptr );
protected:
	/**
	* A list of taskbar buttons.
	* Note: Each button stands for one MDI view (toolviews doesn't have got a taskbar button).
	*/
	Q3PtrList<K3MdiTaskBarButton>* m_pButtonList;
	/**
	* The belonging MDI mainframe (parent widget of this)
	*/
	K3MdiMainFrm* m_pFrm;
	/**
	* The MDI view belonging to the currently pressed taskbar button
	*/
	K3MdiChildView* m_pCurrentFocusedWindow;
	/**
	* A stretchable widget used as 'space' at the end of a half filled taskbar
	*/
	QLabel* m_pStretchSpace;
	bool m_layoutIsPending;
	bool m_bSwitchedOn;

private:
	K3MdiTaskBarPrivate *d;
};

#endif //_K3MDITASKBAR_H_ 
// kate: space-indent off; tab-width 4; replace-tabs off; indent-mode csands;

