/*
    This file is part of the KDE libraries

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    $Id$

*/

#ifndef KTMAINWINDOW_H
#define KTMAINWINDOW_H

#include "kmainwindow.h"

/**
 * @deprecated
 * Compatibility top level widget class
 *
 * This class is intended to make porting of KDE-1.x applications
 * easier. For new applications, use its base class KMainWindow.
 *
 * @see KApplication
 * @short KDE top level main window
   @author Reginald Stadlbauer (reggie@kde.org) Stephan Kulow (coolo@kde.org), Matthias Ettrich (ettrich@kde.org), Chris Schlaeger (cs@kde.org), Sven Radej (radej@kde.org). Maintained by Sven Radej (radej@kde.org)

 */

class KTMainWindow : public KMainWindow
{
    Q_OBJECT

public:
    /**
     * Construct a main window.
     *
     * @param name The object name. For session management to work
     * properly, all main windows in the application have to have a
     * different name. When passing 0 (the default), KTMainWindow will create
     * such a name for you. So simply never pass anything else ;-)
     *
     * @param f Specify the widget flags. The default is WDestructiveClose.
     * It indicates that a main window is automatically destroyed when its
     * window is closed. Pass 0 if you do not want this behaviour.
     *
     * KTMainWindows must be created on the heap with 'new', like:
     *  <pre> KTMainWindow *ktmw = new KTMainWindow (...</pre>
     **/
    KTMainWindow( const char *name = 0, WFlags f = WDestructiveClose );

    /**
     * Destructor.
     *
     * Will also destroy the toolbars, and menubar if
     * needed.
     */
    virtual ~KTMainWindow();
    
    /**
     * Set the main client widget.
     *
     * This is the main widget for your application; it's geometry
     * will be automatically managed by KTMainWindow to fit the
     * client area, constrained by the positions of the menu, toolbars
     * and status bar. It can be fixed-width or Y-fixed.
     *
     * Only one client widget can be handled at a time.  Multiple calls
     * of @ref setView() will cause only the last widget to be added to be
     * properly handled. The layout management will not start before this
     * function has been called. It increases the application start
     * speed to call this function after all bars have been registered. The
     * presence of the view widget is mandatory for the class to operate.
     *
     * The widget must have been created with this instance of
     * KTMainWindow as its parent.
     */
    void setView( QWidget *w, bool = FALSE ) { setCentralWidget( w ); }

    /**
     * Retrieve the view widget.
     *
     * @see setView()
     **/
    QWidget *view() const { return centralWidget(); }

    /**
     * Enable or disable the status bar.
     */
    void enableStatusBar( KStatusBar::BarStatus stat = KStatusBar::Toggle );

    /**
     * Enable or disable the toolbar with the specified @id.
     *
     * If no id is specified, the default id of 0 is used.
     */
    void enableToolBar( KToolBar::BarStatus stat = KToolBar::Toggle, int id = 0 );

    /**
     * Enable or disable the toolbar with the specified name (as
     * determined by the XML UI framework).
     */
    void setEnableToolBar( KToolBar::BarStatus stat = KToolBar::Toggle, const char * name = "mainToolBar" );

    /**
     * Add a toolbar to the widget.
     *
     * A toolbar added to this widget will be automatically laid out
     * by it.
     *
     * The toolbar must have been created with this instance of
     * KTMainWindow as its parent.
     *
     * Usually you do not need this function. Just refer to a toolbar
     * with @ref toolBar(index) instead and the KTMainWindow will
     * create it for you. Anyway @ref addToolBar() is useful if you want
     * to pass additional arguments to the toolbar's constructor.
     */
    int addToolBar( KToolBar *toolbar, int index = -1 );

    /**
     * Retrieve a pointer to the toolbar with the specified id.
     *
     * If there is no such tool bar yet, it will be generated.
     **/
    KToolBar *toolBar( int id = 0 );

    /**
     * @deprecated
     */
    void setMenu (KMenuBar * ) {}

    /**
     * @deprecated
     */
    void setStatusBar ( KStatusBar * ) {};
    
    /**
     * @return @p true if the menubar exists.
     */
    bool hasMenuBar();
    /**
     * @return @p true if the statusbar exists.
     */
    bool hasStatusBar();
    /**
     * @return @p true if the specified toolbar exists.
     */
    bool hasToolBar( int id = 0 );


protected slots:
    /**
     * @deprecated
     */
    virtual void updateRects();
    
private:    
    QMap<int, KToolBar*> idBarMap;
};
    
#endif
