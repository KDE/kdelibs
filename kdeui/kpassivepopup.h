// -*- c++ -*-

/*
 *   Copyright            : (C) 2001-2002 by Richard Moore
 *   Copyright            : (C) 2004-2005 by Sascha Cunz
 *   License              : This file is released under the terms of the LGPL, version 2.
 *   Email                : rich@kde.org
 *   Email                : sascha.cunz@tiscali.de
 */

#ifndef KPASSIVEPOPUP_H
#define KPASSIVEPOPUP_H

#include <qframe.h>

#include <kdelibs_export.h>
#include <kvbox.h>

class QBoxLayout;
class QTimer;
class QLabel;
class KVBox;

/**
 * @short A dialog-like popup that displays messages without interupting the user.
 *
 * The simplest uses of KPassivePopup are by using the various message() static
 * methods. The position the popup appears at depends on the type of the parent window:
 *
 * @li Normal Windows: The popup is placed adjacent to the icon of the window.
 * @li System Tray Windows: The popup is placed adjact to the system tray window itself.
 * @li Skip Taskbar Windows: The popup is placed adjact to the window
 *     itself if it is visible, and at the edge of the desktop otherwise.
 *
 * You also have the option of calling show with a QPoint as a parameter that
 * removes the automatic placing of KPassivePopup and shows it in the point you want.
 *
 * The most basic use of KPassivePopup displays a popup containing a piece of text:
 * \code
 *    KPassivePopup::message( "This is the message", this );
 * \endcode
 * We can create popups with titles and icons too, as this example shows:
 * \code
 *    QPixmap px;
 *    px.load( "hi32-app-logtracker.png" );
 *    KPassivePopup::message( "Some title", "This is the main text", px, this );
 * \endcode
 * For more control over the popup, you can use the setView(QWidget *) method
 * to create a custom popup.
 * \code
 *    KPassivePopup *pop = new KPassivePopup( parent );
 *
 *    QVBox *vb = new QVBox( pop );
 *    (void) new QLabel( vb, "<b>Isn't this great?</b>" );
 *
 *    QHBox *box = new QHBox( vb );
 *    (void) new QPushButton( box, "Yes" );
 *    (void) new QPushButton( box, "No" );
 *
 *    pop->setView( vb );
 *    pop->show();
 * \endcode
 *
 * @since 3.1
 * @author Richard Moore, rich@kde.org
 * @author Sascha Cunz, sascha.cunz@tiscali.de
 */
class KDEUI_EXPORT KPassivePopup : public QFrame
{
    Q_OBJECT
    Q_PROPERTY (bool autoDelete READ autoDelete WRITE setAutoDelete )
    Q_PROPERTY (int timeout READ timeout WRITE setTimeout )

public:
    /**
     * Styles that a KPassivePopup can have.
     * @since 3.5
     */
    enum PopupStyle
    {
        Boxed,             ///< Information will appear in a framed box (default)
        Balloon,           ///< Information will appear in a comic-alike balloon
	CustomStyle=128    ///< Ids greater than this are reserved for use by subclasses
    };

    /**
     * Creates a popup for the specified widget.
     */
    KPassivePopup( QWidget *parent=0, Qt::WFlags f = 0 );

    /**
     * Creates a popup for the specified window.
     */
    KPassivePopup( WId parent );

    /**
     * Creates a popup for the specified widget.
     * @since 3.5
     */
    KPassivePopup( int popupStyle, QWidget *parent=0, Qt::WFlags f=0 );

    /**
     * Creates a popup for the specified window.
     * @since 3.5
     */
    KPassivePopup( int popupStyle, WId parent, Qt::WFlags f=0 );

    /**
     * Cleans up.
     */
    virtual ~KPassivePopup();

    /**
     * Sets the main view to be the specified widget (which must be a child of the popup).
     */
    void setView( QWidget *child );

    /**
     * Creates a standard view then calls setView(QWidget*) .
     */
    void setView( const QString &caption, const QString &text = QString() );

    /**
     * Creates a standard view then calls setView(QWidget*) .
     */
    virtual void setView( const QString &caption, const QString &text, const QPixmap &icon );

    /**
     * Returns a widget that is used as standard view if one of the
     * setView() methods taking the QString arguments is used.
     * You can use the returned widget to customize the passivepopup while
     * keeping the look similar to the "standard" passivepopups.
     *
     * After customizing the widget, pass it to setView( QWidget* )
     *
     * @param caption The window caption (title) on the popup
     * @param text The text for the popup
     * @param icon The icon to use for the popup
     * @param parent The parent widget used for the returned QVBox. If left 0L,
     * then "this", i.e. the passive popup object will be used.
     *
     * @return a QVBox containing the given arguments, looking like the
     * standard passivepopups.
     * @see setView( QWidget * )
     * @see setView( const QString&, const QString& )
     * @see setView( const QString&, const QString&, const QPixmap& )
     */
    KVBox * standardView( const QString& caption, const QString& text,
                          const QPixmap& icon, QWidget *parent = 0L );

    /**
     * Returns the main view.
     */
    QWidget *view() const { return msgView; }

    /**
     * Returns the delay before the popup is removed automatically.
     */
    int timeout() const { return hideDelay; }

    /**
     * Enables / disables auto-deletion of this widget when the timeout
     * occurs.
     * The default is false. If you use the class-methods message(),
     * auto-delection is turned on by default.
     */
    virtual void setAutoDelete( bool autoDelete );

    /**
     * @returns true if the widget auto-deletes itself when the timeout occurs.
     * @see setAutoDelete
     */
    bool autoDelete() const { return m_autoDelete; }

    /**
     * Sets the anchor of this balloon. The balloon tries automatically to adjust
     * itself somehow around the point.
     * @since 3.5
     */
    void setAnchor( const QPoint& anchor );

    // TODO KDE4: give all the statics method a const QPoint p = QPoint() that in
    // case the point is not null calls the show(const QPoint &p) method instead
    // the show() one.
    /**
     * Convenience method that displays popup with the specified  message  beside the
     * icon of the specified widget.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static KPassivePopup *message( const QString &text, QWidget *parent );

    /**
     * Convenience method that displays popup with the specified caption and message
     * beside the icon of the specified widget.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static KPassivePopup *message( const QString &caption, const QString &text,
				   QWidget *parent );

    /**
     * Convenience method that displays popup with the specified icon, caption and
     * message beside the icon of the specified widget.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static KPassivePopup *message( const QString &caption, const QString &text,
				   const QPixmap &icon,
				   QWidget *parent, int timeout = -1 );

    /**
     * Convenience method that displays popup with the specified icon, caption and
     * message beside the icon of the specified window.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static KPassivePopup *message( const QString &caption, const QString &text,
				   const QPixmap &icon,
				   WId parent, int timeout = -1 );

    /**
     * Convenience method that displays popup with the specified popup-style and message beside the
     * icon of the specified widget.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static KPassivePopup *message( int popupStyle, const QString &text, QWidget *parent );

    /**
     * Convenience method that displays popup with the specified popup-style, caption and message
     * beside the icon of the specified widget.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static KPassivePopup *message( int popupStyle, const QString &caption, const QString &text,
				   QWidget *parent );

    /**
     * Convenience method that displays popup with the specified popup-style, icon, caption and
     * message beside the icon of the specified widget.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static KPassivePopup *message( int popupStyle, const QString &caption, const QString &text,
				   const QPixmap &icon,
				   QWidget *parent, int timeout = -1 );

    /**
     * Convenience method that displays popup with the specified popup-style, icon, caption and
     * message beside the icon of the specified window.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static KPassivePopup *message( int popupStyle, const QString &caption, const QString &text,
				   const QPixmap &icon,
				   WId parent, int timeout = -1 );


public slots:
    /**
     * Sets the delay for the popup is removed automatically. Setting the delay to 0
     * disables the timeout, if you're doing this, you may want to connect the
     * clicked() signal to the hide() slot.
     * Setting the delay to -1 makes it use the default value.
     *
     * @see timeout
     */
    void setTimeout( int delay );

    /**
     * Reimplemented to reposition the popup.
     */
    virtual void show();

    /**
     * Shows the popup in the given point
     * @since 3.5
     */
    void show(const QPoint &p);

signals:
    /**
     * Emitted when the popup is clicked.
     */
    void clicked();

    /**
     * Emitted when the popup is clicked.
     */
    void clicked( const QPoint &pos );

protected:
    /**
     * This method positions the popup.
     */
    virtual void positionSelf();

    /**
     * Reimplemented to destroy the object when autoDelete() is
     * enabled.
     */
    virtual void hideEvent( QHideEvent * );

    /**
     * Moves the popup to be adjacent to the icon of the specified rectangle.
     */
    void moveNear( const QRect &target );

    /**
     * Reimplemented to detect mouse clicks.
     */
    virtual void mouseReleaseEvent( QMouseEvent *e );

    /**
     * If no relative window (eg taskbar button, system tray window) is
     * available, use this rectangle (pass it to moveNear()).
     * Basically KWinModule::workArea() with width and height set to 0
     * so that moveNear uses the upper-left position.
     * @return The QRect to be passed to moveNear() if no other is
     * available.
     */
    QRect defaultArea() const;

    /**
     * Updates the transparency mask. Unused if PopupStyle == Boxed
     * @since 3.5
     */
    void updateMask();

    /**
     * Overwrite to paint the border when PopupStyle == Balloon.
     * Unused if PopupStyle == Boxed
     */
    virtual void paintEvent( QPaintEvent* pe );

private:
    void init( int popupStyle );

    WId window;
    QWidget *msgView;
    QBoxLayout *topLayout;
    int hideDelay;
    QTimer *hideTimer;

    QLabel *ttlIcon;
    QLabel *ttl;
    QLabel *msg;

    bool m_autoDelete;

    /* @internal */
    class Private;
    Private *d;
};

#endif // KPASSIVEPOPUP_H

// Local Variables:
// c-basic-offset: 4
// End:

