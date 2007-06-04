/* This file is part of the KDE libraries
   Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB. If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KTITLEWIDGET_H
#define KTITLEWIDGET_H

#include <kdeui_export.h>

#include <QtGui/QWidget>

/**
 * @short Standard title widget with a white background and round border.
 *
 * This class provides a widget often used for dialog titles.
 * \image html ktitlewidget.png "KTitleWidget with title and icon"
 *
 * @section Usage
 * KTitleWidget is very simple to use. You can either use its default text
 * (and pixmap) properties or display your own widgets in the title widget.
 *
 * A title text with a left aligned pixmap:
 * @code
KTitleWidget *titleWidget = new KTitleWidget(this);
titleWidget->setText(i18n("Title"));
titleWidget->setPixmap(KIcon("screen").pixmap(22, 22), KTitleWidget::ImageLeft);
 * @endcode
 *
 * Use it with a own widget:
 * @code
KTitleWidget *checkboxTitleWidget = new KTitleWidget(this);

QWidget *checkBoxTitleMainWidget = new QWidget(this);
QVBoxLayout *titleLayout = new QVBoxLayout(checkBoxTitleMainWidget);
titleLayout->setMargin(6);

QCheckBox *checkBox = new QCheckBox("Text Checkbox", checkBoxTitleMainWidget);
titleLayout->addWidget(checkBox);

checkboxTitleWidget->setWidget(checkBoxTitleMainWidget);
 * @endcode
 *
 * @see KPageView
 * @author Urs Wolfer \<uwolfer @ kde.org\>
 */

class KDEUI_EXPORT KTitleWidget : public QWidget
{
    Q_OBJECT
    Q_ENUMS(ImageAlignment)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QPixmap pixmap READ pixmap WRITE setPixmap)

public:
    /**
     * Possible title pixmap alignments.
     */
    enum ImageAlignment {
        ImageLeft /*<< Display the pixmap on the left */,
        ImageRight /*<< Display the pixmap on the right */
    };

    /**
     * Comment message types
     *
     * @li ImageLeft: Display the pixmap left
     * @li ImageRight: Display the pixmap right (default)
     */
    enum CommentType {
        PlainMessage /*<< Normal comment */,
        InfoMessage /*<< Information the user should be alerted to */,
        WarningMessage /*<< A warning the user should be alerted to */,
        ErrorMessage /*<< An error message */
    };

    /**
     * Constructs a progress bar with the given @param parent.
     */
    explicit KTitleWidget(QWidget *parent = 0);

    virtual ~KTitleWidget();

    /**
     * @param widget Widget displayed on the title widget.
     */
    void setWidget(QWidget *widget);

    /**
     * @return the text displayed in the title
     */
    QString text() const;

    /**
     * @return the text displayed in the comment below the title, if any
     */
    QString comment() const;

    /**
     * @return the pixmap displayed in the title
     */
    const QPixmap *pixmap() const;

    /**
     * Sets this label's buddy to buddy.
     * When the user presses the shortcut key indicated by the label in this
     * title widget, the keyboard focus is transferred to the label's buddy
     * widget.
     * @param buddy the widget to activate when the shortcut key is activated
     */
    void setBuddy(QWidget *buddy);

public Q_SLOTS:
    /**
     * @param text Text displayed on the label. It can either be plain text or rich text. If it
     * is plain text, the text is displayed as a bold title text.
     * @param alignment Alignment of the text. Default is left and vertical centered.
     */
    void setText(const QString &text, Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignVCenter);

    /**
     * @param comment Text displayed beneath the main title as a comment.
     *                It can either be plain text or rich text.
     */
    void setComment(const QString &comment, CommentType type = PlainMessage);

    /**
     * @param pixmap Pixmap displayed in the header. The pixmap is by default right, but
     * @param alignment can be used to display it also left.
     */
    void setPixmap(const QPixmap &pixmap, ImageAlignment alignment = ImageRight);

    /**
     * @param icon name of the icon to display in the header. The pixmap is by default right, but
     * @param alignment can be used to display it also left.
     */
    void setPixmap(const QString &icon, ImageAlignment alignment = ImageRight);

    /**
     * @param pixmap the icon to display in the header. The pixmap is by default right, but
     * @param alignment can be used to display it also left.
     */
    void setPixmap(const QIcon& icon, ImageAlignment alignment = ImageRight);

private:
    class Private;
    Private* const d;

    Q_DISABLE_COPY(KTitleWidget)
};

#endif
