/*****************************************************************************
 * Copyright (C) 2008 by Sebastian Trueg <trueg@kde.org>                     *
 * Copyright (C) 2009 by Peter Penz <peter.penz@gmx.at>                      *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License as published by the Free Software Foundation; either              *
 * version 2 of the License, or (at your option) any later version.          *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#ifndef KCOMMENT_WIDGET
#define KCOMMENT_WIDGET

#include <QString>
#include <QWidget>

class QLabel;

/**
 * @brief Allows to edit and show a comment as part of KMetaDataWidget.
 */
class KCommentWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KCommentWidget(QWidget* parent = 0);
    virtual ~KCommentWidget();

    void setText(const QString& comment);
    QString text() const;

    /**
     * If set to true, the comment cannot be changed by the user.
     * Per default read-only is disabled.
     */
    // TODO: provide common interface class for metadatawidgets
    void setReadOnly(bool readOnly);
    bool isReadOnly() const;
    
    virtual QSize sizeHint() const;

signals:
    void commentChanged(const QString& comment);

protected:
    virtual bool event(QEvent* event);

private slots:
    void slotLinkActivated(const QString& link);

private:
    bool m_readOnly;
    QLabel* m_label;
    QLabel* m_sizeHintHelper; // see comment in KCommentWidget::sizeHint()
    QString m_comment;
};

#endif
