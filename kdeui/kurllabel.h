/* This file is part of the KDE libraries
   Copyright (C) 1998 Kurt Granroth (granroth@kde.org)

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
*/
/*
 * $Id$
 *
 */
#ifndef _KURLLABEL_H
#define _KURLLABEL_H

#include <qpalette.h>
#include <qlabel.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qmovie.h>
#include <qpainter.h>
#include <qbitmap.h>

/**
 * A label class that supports displaying an active hyperlink.
 *
 * KURLLabel is a drop-in replacement for QLabel that handles text
 * in a fashion similar to how an HTML widget handles hyperlinks.  The
 * text can be underlined (or not) and set to different colors.  It
 * can also "glow" (cycle colors) when the mouse passes over it.
 *
 * KURLLabel also provides signals for several events, including
 * the mouse leaving and entering the text area and all forms of
 * mouse clicking.
 *
 * A typical usage would be something like so:
 *
 * <PRE>
 *     KURLLabel *address = new KURLLabel(this);
 *     address->setText("My homepage");
 *     address->setURL("http://www.home.com/~me");
 *     connect(address, SIGNAL(leftClickedURL(const QString&)),
 *                      SLOT(processMyURL(const QString&)));
 * </PRE>
 *
 * In this example, the text "My homepage" would be displayed
 * as blue, underlined text.  When the mouse passed over it, 
 * it would "glow" red.  When the user clicks on the text, the
 * signal @ref leftClickedURL() would be emitted with "http://www.home.com/~me"
 * as its argument.
 *
 * @short A drop-in replacement for QLabel that displays hyperlinks.
 * @author Kurt Granroth <granroth@kde.org>
 * @version 0.5.3
 */
class KURLLabel : public QLabel
{
    Q_OBJECT
    Q_ENUMS( TextAlignment )
    Q_PROPERTY( TextAlignment textAlignment READ textAlignment WRITE setTextAlignment )
    Q_PROPERTY( QString url READ url WRITE setURL )
    Q_PROPERTY( QString text READ text WRITE setText )
    Q_PROPERTY( QPixmap pixmap READ pixmap WRITE setPixmap )
    
public:

    enum TextAlignment
    {
      Bottom,
      Left,
      Top,
      Right
    };

    /**
     * Constructor.
     *
     *  Use this exactly like you would QLabel.
     */
    KURLLabel(QWidget *parent=0, const char *name=0, WFlags f=0);

    /**
     * Destructor.
     */
    virtual ~KURLLabel();

    /**
     * Retreive the URL. 
     *
     * This will be the same as @ref text() if 
     * @ref setURL() is not used.
     *
     * @return The URL.
     */
    const QString url() const;

    /**
     * Retrieve the current text.
     *
     * @see setText()
     *
     * @return The current text.
     */
    const QString text() const;

    /**
     * Retrieve the current pixmap.
     *
     * @see setPixmap()
     *
     * @return The current pixmap.
     */
    const QPixmap* pixmap() const;

    /**
     * Retrieve the current text alignment.
     */
    TextAlignment textAlignment() const;
    
    /**
     * Retrieve the recommended size for this label.
     */
    QSize sizeHint() const;
  
        /**
     * Enable or disable "transparent mode".
     *
     * If transparent mode 
     * is enabled, the label copies its own background from its 
     * parent widget so that it seems to be transparent.
     * Transparent mode is disabled by default.
     * Please note that the method does not repaint the widget.
     * Changes take effect on the next repainting.
     * Transparent widgets do not (currently) work if there is another 
     * widget (a frame, for example) layered between this widget and its
     * parent in Z-order.
     */
         void setTransparentMode(bool state);
public slots:
    /**
     * Turn on or off the "glow" feature. 
     *
     * When this is on, the
     * text will switch to the selected color whenever the mouse
     * passes over it.  By default, it is @true.
     */
    void setGlow(bool glow = true);

    /**
     * Turn on or off the "float" feature.
     *
     *  This feature is very
     * similar to the "glow" feature in that the color of the
     * label switches to the selected color when the cursor passes
     * over it.  In addition, underlining is turned on for as
     * long as the mouse is overhead.  Note that if "glow" and
     * underlining are both already turned on, this feature
     * will have no visible effect.  By default, it is @p false.
     */
    void setFloat(bool do_float = true);

    /**
     * Turn on or off the custom cursor feature.
     *
     *  When this is on, the
     * cursor will change to a custom cursor (default is a "pointing
     * hand") whenever the cursor passes over the label.  By default,
     * it is on.
     */
    void setUseCursor(bool use_cursor, const QCursor* cursor = 0);

    /**
     * Turn on or off the tool tip feature.
     *
     *  When this is on, the
     * URL will be displayed as a tooltip whenever the mouse passes
     * passes over it.  By default, it is @p false.
     */
    void setUseTips(bool tips = true);

    /**
     * Specifies what text to display when tooltips are turned on.
     *
     * If this is not used, the tip will default to the URL.
     *
     * @see setUseTips()
     */
    void setTipText(const QString& tip);

    /**
     * Set the text alignment.
     */
    void setTextAlignment(TextAlignment align);

    /**
     * Turn on or off the underlining.
     *
     *  When this is on, the
     * text will be underlined.  By default, it is @p true.
     */
    void setUnderline(bool underline = true);

    /**
     * Set the highlight color.
     *
     * This is the default foreground
     * color (non-selected).  By default, it is @p blue.
     */
    void setHighlightedColor(const QColor& highcolor);

    /**
     * This is an overloaded version for convenience.
     *
     * @see setHighlightedColor()
     */
    void setHighlightedColor(const QString& highcolor);

    /**
     * Set the selected color.
     *
     * This is the color the text will change
     * to when either a mouse passes over it and "glow" mode is on or
     * when it is selected (clicked).  By default, it is @p red.
     */
    void setSelectedColor(const QColor& selcolor);

    /**
     * This is an overloaded version for convenience.
     *
     * @see setSelectedColor()
     */
    void setSelectedColor(const QString& selcolor);

    /**
     * Set the background color.
     *
     *  By default, it is set to the
     * KDE background color.
     */
    void setBackgroundColor(const QColor& bgcolor);

    /**
     * This is an overloaded version for convenience.
     *
     * @see setBackgroundColor()
     */
    void setBackgroundColor(const QString& bgcolor);

    /**
     * Set the font for the label.
     */
    void setFont(const QFont& font);

    /**
     * Set the label contents to @p text.
     *
     * @see text()
     */
    void setText(const QString& text);

    /**
     * Set the pixmap.
     *
     * Unlike QLabel, this can co-exist with 
     * @ref setText().  It cannot be used along with @ref setMovie(),
     * however.
     *
     * @see pixmap()
     */
    void setPixmap(const QPixmap& pixmap);

    /**
     * Set the "alt" pixmap. 
     *
     * This pixmap will be displayed when the
     * cursor passes over the label.  The effect is similar to the
     * trick done with 'onMouseOver' in javascript.
     *
     * @see altPixmap()
     */
    void setAltPixmap(const QPixmap& pixmap);

    /**
     * Set the movie.
     *
     *  Cannot be used with @ref setPixmap ()
     *
     * @see #movie
     */
    void setMovie(const QMovie& movie);

    /**
     * Sets the URL for this label to @p url.
     *
     * @see #url
     */
    void setURL(const QString& url);

signals:
    /**
     * The mouse has passed over the label.
     *
     * @param url The URL for this label.
     */ 
    void enteredURL(const QString& url);

    /**
     * The mouse has passed over the label.
     */ 
    void enteredURL();

    /**
     * The mouse is no longer over the label.
     *
     * @param url The URL for this label.
     */ 
    void leftURL(const QString& url);

    /**
     * The mouse is no longer over the label.
     */ 
    void leftURL();

    /**
     * The user clicked the left mouse button on this label.
     *
     * @param url The URL for this label.
     */ 
    void leftClickedURL(const QString& url);

    /**
     * The user clicked the left mouse button on this label.
     */ 
    void leftClickedURL();

    /**
     * The user clicked the right mouse button on this label.
     *
     * @param url The URL for this label.
     */ 
    void rightClickedURL(const QString& url);

    /**
     * The user clicked the left mouse button on this label.
     */ 
    void rightClickedURL();

    /**
     * The user clicked the middle mouse button on this label.
     *
     * @param url The URL for this label.
     */ 
    void middleClickedURL(const QString& url);

    /**
     * The user clicked the left mouse button on this label.
     */ 
    void middleClickedURL();

protected:
    /**
     * Draws the text, pixmap, and/or movie
     */
    void drawContents(QPainter *);

    /**
     * Used to "glow" the text when it is selected.
     */
    void timerEvent(QTimerEvent *);

    /**
     * emits the @ref #enteredURL() signal.  If glow is
     * on, it sets the selected color. 
     */
    void m_enterEvent();

    /**
     * emits the @ref #leftURL() signal.  If glow is
     * on, it sets the normal color. 
     */
    void m_leaveEvent();

    /**
     * Processes "true" leave events since @ref #mouseMoveEvent cannot
     */
    void leaveEvent(QEvent *event);

    /**
     * Tracks if the cursor is above the text as well as the mouse state.
     * It emits either the @ref #enteredURL(), @ref #leftURL(),
     * @ref #leftClickedURL(), @ref #middleClickedURL(), or @ref #rightClickedURL()
     * as appropriate.
     */
    void mouseMoveEvent(QMouseEvent *);

    /**
     * emits either the @ref #leftClickedURL(), @ref #rightClickedURL(),
     * or @ref #middleClickedURL() signal depending
     * on which one the user clicked.  Changes the color to selected
     * to indicate that it was selected.  Starts a timer to deselect
     * it.
     */
    void mousePressEvent(QMouseEvent *);

    /**
     * An overloaded repaint event that handles the background in 
     * transparent mode. It sets a background pixmap that is obtained 
     * from the widgets parent and calls the QLabel repaint handler 
     * after that.
     */
    void paintEvent(QPaintEvent*);

private:
    QRect m_textRect() const;
    QRect m_pixmapRect() const;
    void  m_resetPalette();

    TextAlignment m_textAlign;

    QString m_url;
    QString m_tipText;

    QPixmap m_altPixmap;
    QPixmap m_origPixmap;
    QPixmap m_unselPixmap;
    QPixmap m_pixmap;

    QString m_text;
    QMovie  m_movie;

    QCursor m_customCursor;

    QColor m_hc;
    QColor m_bc;
    QColor m_sc;
    QPalette m_nsp;
    QPalette m_sp;

    bool m_float;
    bool m_tips;
    bool m_glow;
    bool m_underline;
    bool m_inRegion;
    bool m_haveCursor;
    bool m_transparent; // true when transparent mode is enabled

    class KURLLabelPrivate;
    KURLLabelPrivate *d;
};

#endif // _KURLLABEL_H
