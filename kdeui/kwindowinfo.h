// -*- c++ -*-

/*
 *   copyright            : (C) 2001-2002 by Richard Moore
 *   License              : This file is released under the terms of the LGPL, version 2.
 *   email                : rich@kde.org
 */

#ifndef KWINDOWINFO_H
#define KWINDOWINFO_H

#include <qobject.h>
#include <qpixmap.h>
#include <qstring.h>

/**
 * Displays messages in the window icon and title. The message is usually
 * transient with the original title and icon being restored automatically
 * after a specified time. The simplest use displays a text message in the
 * window title:
 * <pre>
 *    KWindowInfo::showMessage( this, &quot;Message Body&quot; );
 * </pre>
 * This more complex example changes the window icon, as well as
 * displaying the text. In addition, this example overrides the
 * default timeout to ensure the message is only displayed for 1
 * second.
 * <pre>
 *    QPixmap px;
 *    px.load( &quot;lo16-app-logtracker.png&quot; );
 *    KWindowInfo::showMessage( this, &quot;Message Body&quot;, px, 1000 );
 * </pre>
 * If the parent window inherits KSystemTray then KWindowInfo changes the
 * pixmap and tooltip of the system window to display the message.
 *
 * @author Richard Moore, rich@kde.org
 * @version $Id$
 * @since 3.1
*/
class KWindowInfo : public QObject
{
    Q_OBJECT

public:
    /**
     * Creates a KWindowInfo with the specified parent.
     */
    KWindowInfo( QWidget *parent, const char *name=0 );

    /**
     * Cleans up.
     */
    virtual ~KWindowInfo();

    /** 
     * Returns true iff the object should delete itself when it resets.
     */
    bool autoDelete() const { return autoDel; }

    /**
     * Set to true if you want the object to delete itself when the message
     * timeout occurs.
     */
    void setAutoDelete( bool enable ) { autoDel = enable; }

    /** 
     * Utility method to display a title bar message for the specified
     * window.
     */
    static void showMessage( QWidget *window, const QString &text, int timeout = -1 );

    /** 
     * Utility method to display a title bar message and icon for the
     * specified window.
     */
    static void showMessage( QWidget *window, const QString &text,
			     const QPixmap &pix, int timeout = -1 );

public slots:
    /**
     * Shows the specified text in the window title.
     */
    void message( const QString &text ); 

    /**
     * Shows the specified text in the window title, and sets the window icon.
     */
    void message( const QString &text, const QPixmap &pix ); 

    /**
     * Shows the specified text in the window title for the specified time.
     */
    void message( const QString &text, int timeout ); 

    /**
     * Shows the specified icon and text in the window title and WM
     * icon, for the specified time. The time is a delay specified in
     * milliseconds, or one of the two special values. The special
     * values are -1 which means the default timeout should be used,
     * and 0 which means the message is permanent.
     */
    void message( const QString &text, const QPixmap &pix, int timeout ); 

    /**
     * Shows the specified text in the window title with no timeout.
     */
    void permanent( const QString &text ); 

    /**
     * Shows the specified text and icon in the window title with no timeout.
     */
    void permanent( const QString &text, const QPixmap &pix );

protected:
    /**
     * Displays the message in the titlebar/icon.
     */
    virtual void display( const QString &text, const QPixmap &pix );

protected slots:
    /**
     * Saves the window title and icon.
     */
    virtual void save();

    /**
     * Resets the window title and icon to the saved values. If
     * auto-delete is enabled then the object is deleted.
     */
    virtual void restore();

private:
    QWidget *win;
    QPixmap oldIcon;
    QPixmap oldMiniIcon;
    QString oldText;
    bool autoDel;

    /* @internal */
    class Private *d;
};

#endif // KWINDOWINFO_H

