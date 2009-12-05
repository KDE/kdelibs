/*  This file is part of the KDE Libraries
 *  Copyright (C) 1999 Espen Sand (espensa@online.no)
 *  Copyright (C) 2006 Urs Wolfer <uwolfer at fwo.ch>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef KTEXTBROWSER_H
#define KTEXTBROWSER_H

#include <kdeui_export.h>

#include <QtGui/QTextBrowser>


/**
 * @short Extended QTextBrowser.
 *
 * An extended QTextBrowser.
 *
 * By default it will
 * invoke the system mailer or the system browser when a link is
 * activated, or it can emit the signal urlClick() or mailClick()
 * when a link is activated.
 *
 * If the link starts with the text "whatsthis:" a QWhatsThis
 * box will appear and then display the rest of the text.
 *
 * \image html ktextbrowser.png "KDE Text Browser"
 *
 * @author Espen Sand (espensa@online.no)
 */

class KDEUI_EXPORT KTextBrowser : public QTextBrowser
{
  Q_OBJECT
  Q_PROPERTY( bool notifyClick READ isNotifyClick WRITE setNotifyClick )

  public:
    /**
     * Creates a new text browser.
     *
     * @param parent Parent of the widget.
     * @param notifyClick @p true causes signals to be emitted.
     */
    explicit KTextBrowser( QWidget *parent = 0, bool notifyClick = false );

    /**
     * Destroys the text browser.
     */
    ~KTextBrowser();

    /**
     * Decide whether a click on a link should be handled internally
     * or if a signal should be emitted.
     *
     * @param notifyClick @p true causes signals to be emitted.
     */
    void setNotifyClick( bool notifyClick );

    /**
     * Returns whether a click on a link should be handled internally
     * or if a signal should be emitted.
     */
    bool isNotifyClick() const;

  protected:
    /**
     * Reimplemented to NOT set the source but to do the special handling
     * of links being clicked. Do not call this.
     *
     * If you need to set an initial source url in the text browser, call
     * the QTextBrowser method explicitly, like this:
     * <code>myTextBrowser->QTextBrowser::setSource(url)</code>
     */
    void setSource( const QUrl& name );

    /**
     * Makes sure Key_Escape is ignored
     */
    virtual void keyPressEvent( QKeyEvent *event );

    /**
     * Reimplemented to support Qt2 behavior (Ctrl-Wheel = fast scroll)
     */
    virtual void wheelEvent( QWheelEvent *event );

    /**
    * Re-implemented for internal reasons.  API not affected.
    *
    * See QLineEdit::createPopupMenu().
    */

    virtual void contextMenuEvent( QContextMenuEvent *event );

  Q_SIGNALS:
    /**
     * Emitted when a mail link has been activated and the widget has
     * been configured to emit the signal.
     *
     * @param name The destination name. It is QString() at the moment.
     * @param address The destination address.
     */
    void mailClick( const QString &name, const QString &address );

    /**
     * Emitted if mailClick() is not emitted and the widget has been
     * configured to emit the signal.
     *
     * @param url The destination address.
     */
    void urlClick( const QString &url );

  private:
    class Private;
    Private* const d;
};

#endif
