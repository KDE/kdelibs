/*
 * Copyright 2008 by Montel Laurent <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#ifndef PLASMA_POPUPAPPLET_H
#define PLASMA_POPUPAPPLET_H

#include <plasma/applet.h>
#include <plasma/plasma_export.h>
#include <plasma/plasma.h>

class QGraphicsProxyWidget;
class QGraphicsLinearLayout;

namespace Plasma
{

class Dialog;
class IconWidget;
class PopupAppletPrivate;

/**
 * Allows applets to automatically 'collapse' into an icon when put in an panel, and is a convenient
 * base class for any applet that wishes to use extenders.
 *
 * Applets that subclass this class should implement either widget() or graphicsWidget() to return a
 * widget that will be displayed in the applet if the applet is in a Planar or MediaCenter form
 * factor. If the applet is put in a panel, an icon will be displayed instead, which shows the
 * widget in a popup when clicked.
 *
 * If you use this class as a base class for your extender using applet, the extender will
 * automatically be used for the popup; reimplementing graphicsWidget() is unnecessary in this case.
 * If you need a popup that does not steal window focus when openend or used, set window flag
 * Qt::X11BypassWindowManagerHint the widget returned by widget() or graphicsWidget().
 */

class PLASMA_EXPORT PopupApplet : public Plasma::Applet
{
    Q_OBJECT
    Q_PROPERTY(Qt::AlignmentFlag popupAlignment READ popupAlignment WRITE setPopupAlignment)

public:
    PopupApplet(QObject *parent, const QVariantList &args);
    ~PopupApplet();

    /**
     * @arg icon the icon that has to be displayed when the applet is in a panel.
     *           Passing in a null icon means that the popup applet itself
     *           will provide an interface for when the PopupApplet is not showing
     *           the widget() or graphicsWidget() directly.
     */
    void setPopupIcon(const QIcon &icon);

    /**
     * @arg icon the icon that has to be displayed when the applet is in a panel.
     *           Passing in an empty QString() means that the popup applet itself
     *           will provide an interface for when the PopupApplet is not showing
     *           the widget() or graphicsWidget() directly.

     * If you set a popup icon you must also set a minimum size of the applet. When the applet
     * is smaller than this minimum size, it will be displayed as that icon.
     */
    void setPopupIcon(const QString &iconName);

    /**
     * @return the icon that is displayed when the applet is in a panel.
     */
    QIcon popupIcon() const;

    /**
     * Implement either this function or graphicsWidget().
     * @return the widget that will get shown in either a layout, in the applet or in a Dialog,
     * depending on the form factor of the applet.
     */
    virtual QWidget *widget();
    void setWidget(QWidget * widget);

    /**
     * Implement either this function or widget().
     * @return the widget that will get shown in either a layout, in the applet or in a Dialog,
     * depending on the form factor of the applet.
     * If you set a popup icon you must also set a minimum size of the applet. When the applet
     * is smaller than this minimum size, it will be displayed as that icon.
     */
    virtual QGraphicsWidget *graphicsWidget();
    void setGraphicsWidget(QGraphicsWidget * widget);

    /**
     * @return the placement of the popup relating to the applet
     */
    Plasma::PopupPlacement popupPlacement() const;

    /**
     * Sets the default alignment of the popup relative to the applet
     * @arg alignment the alignment to use; Qt::AlignLeft or Qt::AlignRight
     * @since 4.6
     */
    void setPopupAlignment(Qt::AlignmentFlag alignment);

    /**
     * @return the default alignment of the popup relative to the applet
     * @since 4.6
     */
    Qt::AlignmentFlag popupAlignment() const;

    /**
     * Sets whether or not the dialog popup that gets created should be a "passive" popup
     * that does not steal focus from other windows or not.
     *
     * @arg passive true if the dialog should be treated as a passive popup
     */
    void setPassivePopup(bool passive);

    /**
     * @return true if the dialog will be treated as a passive poup
     */
    bool isPassivePopup() const;

    /**
     * @return true if the applet is popped up
     */
    bool isPopupShowing() const;

    /**
     * @return true if the applet is collapsed to an icon
     * @since 4.6
     */
    bool isIconified() const;


public Q_SLOTS:
    /**
     * Hides the popup.
     */
    void hidePopup();

    /**
     * Shows the dialog showing the widget if the applet is in a panel.
     * @arg displayTime the time in ms that the popup should be displayed, defaults to 0 which means
     * always (until the user closes it again, that is).
     */
    void showPopup(uint displayTime = 0);

    /**
     * Toggles the popup.
     */
    void togglePopup();

protected:
    /**
     * This event handler can be reimplemented in a subclass to receive an
     * event before the popup is shown or hidden.
     * @arg show true if the popup is going to be shown, false if the popup
     * is going to be hidden.
     * Note that showing and hiding the popup on click is already done in PopupApplet.
     */
    virtual void popupEvent(bool show);

    /**
     * Reimplemented from QGraphicsLayoutItem
     */
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint = QSizeF()) const;

    /**
     * Reimplemented from QGraphicsLayoutItem
     */
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

    /**
     * Reimplemented from QGraphicsLayoutItem
     */
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    /**
     * Reimplemented from QGraphicsLayoutItem
     */
    bool eventFilter(QObject *watched, QEvent *event);

    /**
     * Reimplemented from QGraphicsLayoutItem
     */
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);

    /**
     * Reimplemented from QGraphicsLayoutItem
     */
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);

    /**
     * Reimplemented from QGraphicsLayoutItem
     */
    void dropEvent(QGraphicsSceneDragDropEvent *event);

    /**
     * Reimplemented from QGraphicsLayoutItem
     */
    void timerEvent(QTimerEvent *event);

private:
    /**
     * @internal This constructor is to be used with the Package loading system.
     *
     * @param parent a QObject parent; you probably want to pass in 0
     * @param args a list of strings containing two entries: the service id
     *      and the applet id
     * @since 4.3
     */
    PopupApplet(const QString &packagePath, uint appletId, const QVariantList &args);

    Q_PRIVATE_SLOT(d, void internalTogglePopup())
    Q_PRIVATE_SLOT(d, void hideTimedPopup())
    Q_PRIVATE_SLOT(d, void clearPopupLostFocus())
    Q_PRIVATE_SLOT(d, void dialogSizeChanged())
    Q_PRIVATE_SLOT(d, void dialogStatusChanged(bool))
    Q_PRIVATE_SLOT(d, void updateDialogPosition())
    Q_PRIVATE_SLOT(d, void appletActivated())
    Q_PRIVATE_SLOT(d, void iconSizeChanged(int))
    Q_PRIVATE_SLOT(d, void statusChangeWhileShown(Plasma::ItemStatus status))

    friend class Applet;
    friend class AppletPrivate;
    friend class Extender;
    friend class PopupAppletPrivate;
    PopupAppletPrivate * const d;
};

} // Plasma namespace

#endif /* POPUPAPPLET_H */

