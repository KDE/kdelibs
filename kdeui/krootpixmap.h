/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesktop.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * You can Freely distribute this program under the GNU Library General
 * Public License. See the file "COPYING.LIB" for the exact licensing terms.
 */

#ifndef KROOTPIXMAP_H
#define KROOTPIXMAP_H

#include <qobject.h>
#include <qcolor.h>
#include <qwindowdefs.h>
#include <qrect.h>
#include <kdelibs_export.h>
#include <kwinmodule.h>

#ifndef Q_WS_QWS //FIXME

class QPixmap;
class QWidget;
class QTimer;
class KSharedPixmap;
class KRootPixmapData;
class QPixmap;

/**
 * Creates pseudo-transparent widgets.
 *
 * A pseudo-transparent widget is a widget with its background pixmap set to
 * that part of the desktop background that it is currently obscuring. This
 * gives a transparency effect.
 *
 * To create a transparent widget, construct a KRootPixmap and pass it a
 * pointer to your widget. That's it! Moving, resizing and background changes
 * are handled automatically.
 *
 * Instead of using the default behavior, you can ask KRootPixmap
 * to emit a backgroundUpdated(const QPixmap &) signal whenever
 * the background needs updating by using setCustomPainting(bool).
 * Alternatively by reimplementing updateBackground(KSharedPixmap*)
 * you can take complete control of the behavior.
 *
 * @author Geert Jansen <jansen@kde.org>
 */
class KDEUI_EXPORT KRootPixmap: public QObject
{
    Q_OBJECT

public:
    /**
     * Constructs a KRootPixmap. The KRootPixmap will be created as a child
     * of the target widget so it will be deleted automatically when the
     * widget is destroyed.
     *
     * @param target A pointer to the widget that you want to make pseudo
     * transparent.
     */
    KRootPixmap( QWidget *target );

    /**
     * Constructs a KRootPixmap where the parent QObject and target QWidget are
     * different.
     */
    KRootPixmap( QWidget *target, QObject *parent );

    /**
     * Destructs the object.
     */
    virtual ~KRootPixmap();

    /**
     * Checks if pseudo-transparency is available.
     * @return @p true if transparency is available, @p false otherwise.
     */
    bool isAvailable() const;

    /**
     * Returns true if the KRootPixmap is active.
     */
    bool isActive() const { return m_bActive; }

    /**
     * Returns the number of the current desktop.
     */
    int currentDesktop() const;

    /**
     * Returns true if custom painting is enabled, false otherwise.
     * @see setCustomPainting(bool)
     */
    bool customPainting() const { return m_bCustomPaint; }


    /** @since 3.2
     * @return the fade color.
     */
    const QColor &color() const { return m_FadeColor; }

    /** @since 3.2
     * @return the color opacity.
     */
    double opacity() const { return m_Fade; }

public Q_SLOTS:
    /**
     * Starts background handling.
     */
    virtual void start();

    /**
     * Stops background handling.
     */
    virtual void stop();

    /**
     * Sets the fade effect.
     *
     * This effect will fade the background to the
     * specified color.
     * @param opacity A value between 0 and 1, indicating the opacity
     * of the color. A value of 0 will not change the image, a value of 1
     * will use the fade color unchanged.
     * @param color The color to fade to.
     */
    void setFadeEffect(double opacity, const QColor &color);

    /**
     * Repaints the widget background. Normally, you shouldn't need this
     * as it is handled automatically.
     *
     * @param force Force a repaint, even if the contents did not change.
     */
    void repaint( bool force );

    /**
     * Repaints the widget background. Normally, you shouldn't need this
     * as it is handled automatically. This is equivalent to calling
     * repaint( false ).
     */
    void repaint();

    /**
     * Enables custom handling of the background painting. If custom
     * painting is enabled then KRootPixmap will emit a
     * backgroundUpdated() signal when the background for the
     * target widget changes, instead of applying the new background.
     */
    void setCustomPainting( bool enable ) { m_bCustomPaint = enable; }

    /**
     * Asks KDesktop to export the desktop background as a KSharedPixmap.
     * This method uses DCOP to call KBackgroundIface/setExport(int).
     */
    void enableExports();

    /**
     * Returns the name of the shared pixmap (only needed for low level access)
     */
    static QString pixmapName(int desk);
Q_SIGNALS:
    /**
     * Emitted when the background needs updating and custom painting
     * (see setCustomPainting(bool) ) is enabled.
     *
     * @param pm A pixmap containing the new background.
     */
    void backgroundUpdated( const QPixmap &pm );

protected:
    /**
     * Reimplemented to filter the events from the target widget and
     * track its movements.
     */
    virtual bool eventFilter(QObject *, QEvent *);

    /**
     * Called when the pixmap has been updated. The default implementation
     * applies the fade effect, then sets the target's background, or emits
     * backgroundUpdated(const QPixmap &) depending on the painting mode.
     */
    virtual void updateBackground( KSharedPixmap * );

private Q_SLOTS:
    void slotBackgroundChanged(int);
    void slotDone(bool);
    void desktopChanged(int desktop);
    void desktopChanged( WId window, unsigned int properties );

private:
    bool m_bActive, m_bInit, m_bCustomPaint;
    int m_Desk;

    double m_Fade;
    QColor m_FadeColor;

    QRect m_Rect;
    QWidget *m_pWidget;
    QTimer *m_pTimer;
    KSharedPixmap *m_pPixmap;
    KRootPixmapData *d;

    void init();
};

#endif // ! Q_WS_QWS
#endif // KROOTPIXMAP_H

