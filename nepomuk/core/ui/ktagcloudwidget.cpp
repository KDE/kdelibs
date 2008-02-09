/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ktagcloudwidget.h"
#include "kblocklayout.h"

#include <kurllabel.h>

#include <QtGui/QBoxLayout>


class KTagCloudWidget::Private
{
public:
    Private()
        : minFontSize(8),
          maxFontSize(22) {
    }

    QMap<QString,int> tagMap;
    QMap<QString,KUrlLabel*> labelMap;
    QMap<QString,double> weightMap;

    KBlockLayout* mainLay;

    int minFontSize;
    int maxFontSize;

    void buildWeightMap() {
        weightMap.clear();

        int max = 0;
        int min = 0;

        // determine max
        for( QMap<QString,int>::const_iterator it = tagMap.constBegin();
             it != tagMap.constEnd(); ++it ) {
            max = qMax( max, it.value() );
            min = qMin( min, it.value() );
        }

        // normalize weights
        for( QMap<QString,int>::const_iterator it = tagMap.constBegin();
             it != tagMap.constEnd(); ++it ) {
            weightMap[it.key()] = (double)(it.value() - min) / (double)qMax(max - min, 1);
        }
    }

    KUrlLabel* createLabel( QWidget* parent, const QString& tag ) {
        KUrlLabel* label = new KUrlLabel( tag, tag, parent );
        QFont font( label->font() );
        font.setPointSize( minFontSize + (int)((double)(maxFontSize-minFontSize)*weightMap[tag]) );
        if( weightMap[tag] > 0.8 )
            font.setBold( true );
        label->setFont( font );
        label->setUnderline(false);
        return label;
    }
};


KTagCloudWidget::KTagCloudWidget( QWidget* parent )
    : QWidget( parent ),
      d( new Private() )
{
    d->mainLay = new KBlockLayout( this );
    d->mainLay->setAlignment( Qt::AlignJustify|Qt::AlignVCenter );
}


KTagCloudWidget::~KTagCloudWidget()
{
    delete d;
}


void KTagCloudWidget::setMaxFontSize( int pointSize )
{
    d->maxFontSize = pointSize;
    buildTagCloud();
}


void KTagCloudWidget::setMinFontSize( int pointSize )
{
    d->minFontSize = pointSize;
    buildTagCloud();
}


int KTagCloudWidget::tagWeight( const QString& tag ) const
{
    return d->tagMap[tag];
}


void KTagCloudWidget::addTag( const QString& tag, int weight )
{
    d->tagMap[tag] = weight;
    buildTagCloud();
}


void KTagCloudWidget::addTags( const QMap<QString,int>& tags )
{
    for( QMap<QString,int>::const_iterator it = tags.constBegin();
         it != tags.constEnd(); ++it )
        d->tagMap[it.key()] = it.value();
    buildTagCloud();
}


void KTagCloudWidget::clear()
{
    d->tagMap.clear();
    buildTagCloud();
}


void KTagCloudWidget::resizeEvent( QResizeEvent* e )
{
    QWidget::resizeEvent( e );
}


void KTagCloudWidget::buildTagCloud()
{
    // Brute force: delete all existing labels
    for( QMap<QString,KUrlLabel*>::iterator it = d->labelMap.begin();
         it != d->labelMap.end(); ++it )
        delete it.value();

    d->labelMap.clear();

    d->buildWeightMap();

    // now rebuild the cloud
    QStringList tags;
    for( QMap<QString,int>::const_iterator it = d->tagMap.constBegin();
         it != d->tagMap.constEnd(); ++it ) {
        tags.append( it.key() );
        d->labelMap[it.key()] = d->createLabel( this, it.key() );
        connect( d->labelMap[it.key()], SIGNAL(leftClickedUrl(const QString&)),
                 this, SIGNAL(tagClicked(const QString&)) );
    }

    // for now we display the tags sorted alphabetically
    qSort( tags.begin(), tags.end() );

    int x = 0;
    foreach( const QString &tag, tags ) {
        KUrlLabel* label = d->labelMap[tag];
        if( x + label->width() > width() ) {
            // new line
            x = 0;
        }
        d->mainLay->addWidget( label );
        x += label->width();
    }
}

#include "ktagcloudwidget.moc"
