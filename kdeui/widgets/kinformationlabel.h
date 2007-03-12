/*  This file is part of the KDE libraries
    Copyright (C) 2007 Michaël Larouche <larouche@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 2
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KDEUI_KINFORMATIONLABEL_H
#define KDEUI_KINFORMATIONLABEL_H

#include <QtGui/QFrame>
#include <QtGui/QIcon>
#include <kdelibs_export.h>

class QMouseEvent;
/**
 * @brief A specialized label to display informative message
 *
 * KInformationLabel is used to display informative message only when needed.
 * This is useful to display a warning or error message in a dialog to notify
 * the user of a problem without using a modal dialog like a message box.
 *
 * @section Behavior
 * KInformationLabel has a special behavior. When first instancied,
 * It is not visible to the user untill a message is affected to it.
 *
 * After the message being shown, the user can click on the label to hide
 * it. Also, you can set a autohide timeout using the timeout property.
 * By default, no timeout is specified so the label is shown untill the
 * user click on it.
 *
 * To hide the widget using code, just set an empty text.
 *
 * @section Usage
 * KInformationLabel is available as a widget in Qt Designer. Usage is very
 * simple, you just need to set a text to the label. 4 types of messages can
 * be set to the label, an informative message, an error message, a warning
 * message and a custom message with a custom icon. See the example below:
 * @code
KInformationLabel *label = new KInformationLabel(parent);
label->setIconType(KInformation::Error);
label->setText("Sample Error Message");
 * @endcode
 *
 * To set a custom message and icon, you need to set the icon property
 * using setIcon() and iconType property to Custom using setIconType
 * @code
KInformationLabel *label = new KInformationLabel(parent);
label->setIconType(KInformation::Custom);
label->setIcon( KIcon("go-home") );
label->setText("Custom message using go-home icon");
 * @endcode
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class KDEUI_EXPORT KInformationLabel : public QFrame
{
    Q_OBJECT
    /**
     * @brief Text used in the label
     */
    Q_PROPERTY(QString text READ text WRITE setText)
    /**
     * @brief Icon type for the message
     */
    Q_PROPERTY(Icon iconType READ iconType WRITE setIconType)
    /**
     * @brief Icon for the Custom icon type
     */
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
    /**
     * @brief Autohide timeout for the label. Set to 0 to disable autohide
     */
    Q_PROPERTY(int autoHideTimeout READ autoHideTimeout WRITE setAutoHideTimeout)
    Q_ENUMS(Icon)

public:
    /**
     * Type of icons which can be displayed by the label
     */
    enum Icon
    {
        Information, ///< Icon for an informative message
        Error, ///< Icon for an error message
        Warning, ///< Icon for a warning message
        Custom ///< Set a custom icon using setIconName()
    };

    /**
     * Constructor
     * @param parent Parent widget
     */
    explicit KInformationLabel(QWidget *parent = 0);
    /**
     * Destructor
     */
    ~KInformationLabel();

    /**
     * @brief Get the current text set in the label
     * @return current text as QString
     */
    QString text() const;

    /**
     * @brief Get the current icon type
     * @return current icon type
     */
    KInformationLabel::Icon iconType() const;

    /**
     * @brief Get the current icon name.
     * @return current icon name
     */
    QIcon icon() const;

    /**
     * @brief Get the current timeout value in miliseconds
     * @return timeout value in msecs.
     */
    int autoHideTimeout() const;

    /**
     * @brief Set an icon type to the label
     *
     * When icon type is set to Custom, it use the current
     * icon name to update the pixmap in the label.
     * @param iconType current icon type. See Icon enum
     */
    void setIconType(KInformationLabel::Icon iconType);

    /**
     * @brief Set an icon for Custom icon type
     *
     * When the label is in Custom mode, it also
     * update the current pixmap.
     * @param icon a QIcon or a KIcon
     */
    void setIcon(const QIcon &icon);

public Q_SLOTS:
    /**
     * @brief Set a message to the label
     *
     * When calling this method, the widget get visible.
     *
     * If the text is empty, the widget hide itself.
     *
     * @param text text to show. use an empty string to hide the widget
     */
    void setText(const QString &text);

    /**
     * @brief Set the autohide timeout of the label
     *
     * Set value to 0 to disable autohide.
     * @param msecs timeout value in milliseconds
     */
    void setAutoHideTimeout(int msecs);

protected:
    bool eventFilter(QObject *object, QEvent *event);

private:
    Q_DISABLE_COPY(KInformationLabel)
    Q_PRIVATE_SLOT(d, void _k_timeoutFinished())

    class Private;
    Private * const d;
};

#endif

// kate: space-indent on; indent-width 4; encoding utf-8; replace-tabs on;
