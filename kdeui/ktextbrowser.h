/*  This file is part of the KDE Libraries
 *  Copyright (C) 1999 Espen Sand (espensa@online.no)
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#ifndef _KTEXT_BROWSER_H_
#define _KTEXT_BROWSER_H_

#include <qtextbrowser.h>

/**
 * This class provides an extended QTextBrowser. By default it will 
 * invoke the system mailer or the system browser when a link is 
 * activated, or it can emit the signal @ref urlClick or @ref mailClick 
 * when a link is activated.
 * 
 * @short Extended QTextBrowser.
 * @author Espen Sand (espensa@online.no)
 * @version $Id$
 */

class KTextBrowser : public QTextBrowser
{
  Q_OBJECT
  
  public:
    /** 
     * Constructore wher you specify the face.
     * 
     * @param parent Parent of the widget.
     * @param name Widget name.
     * @param notifyClick True causes signals to be emitted.
     */
    KTextBrowser( QWidget *parent=0, const char *name=0, 
		  bool notifyClick=false );
  
    /**
     * Destructor
     */
    ~KTextBrowser( void );

    /**
     * Decides whether a click on a link should be handled internally
     * or if a signal should be emitted.
     * 
     * @param notifyClick True causes signals to be emitted.
     */
    void setNotifyClick( bool notifyClick );

  protected:
    /**
     * Emits the signal or takes care of the action itself.
     */
    virtual void viewportMouseReleaseEvent( QMouseEvent *e );  

    /**
     * Makes sure Key_Escape is ignored
     */
    virtual void keyPressEvent(QKeyEvent *e);

  protected slots:
    /**
     * Stores the current reference. This reference in used in 
     * @ref viewportMouseReleaseEvent.
     */
    void refChanged( const QString & );

  signals:
    /**
     * Emitted when a mail link has been activated and the widget has
     * been configured to emit the signal.
     * @param name The destination name. It is QString::null at the moment.
     * @param address The destination address.
     */
    void mailClick( const QString &name, const QString &address );
    
    /**
     * Emitted if @ref mailClick is not emitted and the widget has been 
     * configured to emit the signal.
     * @param url The destination address.
     */
    void urlClick( const QString &url );

  private:
    QString mActiveRef;
    bool    mNotifyClick;
};

#endif
