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

#include "tagcheckbox.h"
#include "tagwidget.h"
#include "tagwidget_p.h"

#include <kcursor.h>
#include <kcolorscheme.h>
#include <kglobalsettings.h>

#include <QtGui/QMouseEvent>
#include <QtGui/QStyleOptionButton>
#include <QtGui/QHBoxLayout>
#include <QtGui/QCheckBox>
#include <QtGui/QLabel>


class Nepomuk::TagCheckBox::CheckBoxWithPublicInitStyleOption : public QCheckBox
{
public:
    CheckBoxWithPublicInitStyleOption( const QString& text, QWidget* parent )
        : QCheckBox( text, parent ) {
    }

    void initStyleOption( QStyleOptionButton* so ) {
        QCheckBox::initStyleOption( so );
    }
};


Nepomuk::TagCheckBox::TagCheckBox( const Tag& tag, TagWidgetPrivate* tagWidget, QWidget* parent )
    : QWidget( parent ),
      m_label(0),
      m_checkBox(0),
      m_tag(tag),
      m_urlHover(false),
      m_tagWidget(tagWidget)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setMargin(0);
    if( tagWidget->m_flags & TagWidget::MiniMode ) {
        m_label = new QLabel( tag.genericLabel(), this );
        m_label->setMouseTracking(true);
        m_child = m_label;
    }
    else {
        m_checkBox = new CheckBoxWithPublicInitStyleOption( tag.genericLabel(), this );
        m_child = m_checkBox;
        connect(m_checkBox, SIGNAL(stateChanged(int)), this, SLOT(slotStateChanged(int)));
    }
    m_child->installEventFilter( this );
    m_child->setMouseTracking(true);
    layout->addWidget( m_child );
}


Nepomuk::TagCheckBox::~TagCheckBox()
{
}


bool Nepomuk::TagCheckBox::isChecked() const
{
    if( m_checkBox )
        return m_checkBox->isChecked();
    else
        return true;
}


void Nepomuk::TagCheckBox::setChecked( bool checked )
{
    if( m_checkBox )
        m_checkBox->setChecked( checked );
}


void Nepomuk::TagCheckBox::leaveEvent( QEvent* event )
{
    QWidget::leaveEvent( event );
    enableUrlHover( false );
}


bool Nepomuk::TagCheckBox::eventFilter( QObject* watched, QEvent* event )
{
    if( watched == m_child ) {
        switch( event->type() ) {
        case QEvent::MouseMove: {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            if( !(m_tagWidget->m_flags & TagWidget::DisableTagClicking) )
                enableUrlHover( tagRect().contains(me->pos()) );
        }

        case QEvent::MouseButtonRelease: {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            if( !(m_tagWidget->m_flags & TagWidget::DisableTagClicking) &&
                me->button() == Qt::LeftButton &&
                tagRect().contains(me->pos()) ) {
                emit tagClicked( m_tag );
                return true;
            }
        }

        default:
            // do nothing
            break;
        }
    }

    return QWidget::eventFilter( watched, event );
}


void Nepomuk::TagCheckBox::slotStateChanged( int state )
{
    emit tagStateChanged( m_tag, state );
}


QRect Nepomuk::TagCheckBox::tagRect() const
{
    if( m_checkBox ) {
        QStyleOptionButton opt;
        m_checkBox->initStyleOption(&opt);
        return style()->subElementRect(QStyle::SE_CheckBoxContents,
                                       &opt,
                                       m_checkBox);
    }
    else {
        return QRect(QPoint(0, 0), m_label->size());
    }
}


void Nepomuk::TagCheckBox::enableUrlHover( bool enable )
{
    if( m_urlHover != enable ) {
        m_urlHover = enable;
        QFont f = font();
        if(enable)
            f.setUnderline(true);
        m_child->setFont(f);
        m_child->setCursor( enable ? Qt::PointingHandCursor : Qt::ArrowCursor );
    }
}

#include "tagcheckbox.moc"
