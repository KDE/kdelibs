/****************************************************************************
    Definition of QXEmbed class

   Copyright (C) 1999-2000 Troll Tech AS

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*****************************************************************************/

#ifndef QXEMBED_H
#define QXEMBED_H

#include <qwidget.h>

#ifndef Q_WS_QWS

/*
  Documentation for this class is in the cpp-file!
 */


class QXEmbedData;

/**
 * A QXEmbed widget serves as an embedder that can manage one single
 * embedded X-window. These so-called client windows can be arbitrary
 * QWidgets.
 * 
 * There are two different ways of using QXEmbed, from the embedder's
 * or from the client's side.  When using it from the embedder's side,
 * you already know the window identifier of the window that should be
 * embedded. Simply call @ref #embed() with this identifier as parameter.
 *
 * Embedding from the client's side requires that the client knows the
 * window identifier of the respective embedder widget. Use either
 * @ref #embedClientIntoWindow() or the high-level wrapper
 * @ref #processClientCmdline().
 * 
 * If a window has been embedded succesfully, @ref #embeddedWinId() returns
 * its id.
 * 
 * Reimplement the change handler @ref #windowChanged() to catch embedding or
 * the destruction of embedded windows. In the latter case, the
 * embedder also emits a signal @ref #embeddedWindowDestroyed() for
 * convenience.
 *
 * @short The QXEmbed widget is a graphical socket that can embed an external X-Window.
*/
class QXEmbed : public QWidget
{
    Q_OBJECT

public:

    /**
     *
     * Constructs a xembed widget.
     *
     * The parent, name and f arguments are passed to the QFrame
     * constructor.
     */
    QXEmbed( QWidget *parent=0, const char *name=0, WFlags f = 0 );

    /**
     * Destructor. Cleans up the focus if necessary.
     */
    ~QXEmbed();

    static void initialize();

    /**
     * Embeds the window with the identifier w into this xembed widget.
     * 
     * This function is useful if the server knows about the client window
     * that should be embedded.  Often it is vice versa: the client knows
     * about its target embedder. In that case, it is not necessary to call
     * embed(). Instead, the client will call the static function
     * @ref #embedClientIntoWindow().
     * 
     * @param w the identifier of the window to embed
     * @see #embeddedWinId()
     */
    void embed( WId w );

    /**
     * Returns the window identifier of the embedded window, or 0 if no
     * window is embedded yet.
     *
     * @return the id of the embedded window (0 if no window is embedded)
     */
    WId embeddedWinId() const;

    /**
     * A function for clients that embed themselves. The widget 
     * client will be embedded in the window window. The application has
     * to ensure that window is the handle of the window identifier of
     * an QXEmbed widget.
     * 
     * @short #processClientCmdline()
     */
    static void embedClientIntoWindow( QWidget* client, WId window );

    /**
     * A utility function for clients that embed theirselves. The widget
     * client will be embedded in the window that is passed as
     * -embed command line argument.
     * 
     * The function returns TRUE on sucess or FALSE if no such command line
     * parameter is specified.
     *
     * @see #embedClientIntoWindow()
     */
    static bool processClientCmdline( QWidget* client, int& argc, char ** argv );


    /**
     * Returns a size sufficient for the embedded window
     */
    QSize sizeHint() const;

    /**
     * Returns the minimum size specified by the embedded window.
     */
    QSize minimumSizeHint() const;

    /**
     * Specifies that this widget can use additional space, and that it can
     * survive on less than @ref #sizeHint().
    */
    QSizePolicy sizePolicy() const;

    bool eventFilter( QObject *, QEvent * );

    void setAutoDelete( bool );
    bool autoDelete() const;

    bool customWhatsThis() const;
    void enterWhatsThisMode(); // temporary, fix in Qt (Matthias, Mon Jul 17 15:20:55 CEST 2000  )

signals:
    /**
     * This signal is emitted when the embedded window has been destroyed.
     *
     * @see #embeddedWinId()
     */
    void embeddedWindowDestroyed();

protected:
    bool event( QEvent * );
    void keyPressEvent( QKeyEvent * );
    void keyReleaseEvent( QKeyEvent * );
    void focusInEvent( QFocusEvent * );
    void focusOutEvent( QFocusEvent * );
    void resizeEvent(QResizeEvent *);
    void showEvent( QShowEvent * );
    bool x11Event( XEvent* );

    /**
     * A change handler that indicates that the embedded window has been
     * changed.  The window handle can also be retrieved with
     * @ref #embeddedWinId().
     *
     * @param w the handle of the window that changed
     */
    virtual void windowChanged( WId w );

    bool focusNextPrevChild( bool next );

private:
    WId window;
    QXEmbedData* d;
    void sendSyntheticConfigureNotifyEvent();
};


#endif
#endif
