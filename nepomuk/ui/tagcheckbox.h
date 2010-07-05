/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2010 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _NEPOMUK_TAG_CHECKBOX_H_
#define _NEPOMUK_TAG_CHECKBOX_H_

#include <QtGui/QWidget>

#include "tag.h"
#include "tagwidget_p.h"

class QMouseEvent;
class QLabel;

namespace Nepomuk {
class TagCheckBox : public QWidget
{
    Q_OBJECT

public:
    TagCheckBox( const Tag& tag, TagWidgetPrivate* tagWidget, QWidget* parent = 0 );
    ~TagCheckBox();

    Tag tag() const { return m_tag; }

    bool isChecked() const;

public Q_SLOTS:
    void setChecked( bool checked );

Q_SIGNALS:
    void tagClicked( const Nepomuk::Tag& tag );
    void tagStateChanged( const Nepomuk::Tag& tag, int state );

protected:
    void leaveEvent( QEvent* event );
    bool eventFilter( QObject* watched, QEvent* event );

private Q_SLOTS:
    void slotStateChanged( int state );

private:
    QRect tagRect() const;
    void enableUrlHover( bool enabled );

    // two modes: checkbox and simple label
    QLabel* m_label;
    class CheckBoxWithPublicInitStyleOption;
    CheckBoxWithPublicInitStyleOption* m_checkBox;
    QWidget* m_child;

    Tag m_tag;
    bool m_urlHover;
    TagWidgetPrivate* m_tagWidget;
};
}

#endif
