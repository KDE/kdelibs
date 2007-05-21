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

#include "ktitlewidget.h"

#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QTextDocument>

class KTitleWidget::Private
{
public:
    QHBoxLayout *titleWidgetLayout;
    QHBoxLayout *headerLayout;
    QLabel *imageLabel;
    QLabel *textLabel;
    QWidget *headerWidget;
};

KTitleWidget::KTitleWidget(QWidget *parent)
  : QWidget(parent),
    d(new Private)
{
    QFrame *titleFrame = new QFrame(this);
    titleFrame->setAutoFillBackground(true);
    titleFrame->setFrameShape(QFrame::StyledPanel);
    titleFrame->setFrameShadow(QFrame::Plain);
    titleFrame->setBackgroundRole(QPalette::Base);

    d->titleWidgetLayout = new QHBoxLayout(titleFrame);
    d->titleWidgetLayout->setMargin(0);

    // default image / text part start
    d->headerWidget = new QWidget(this);
    d->headerWidget->setVisible(false);
    d->headerLayout = new QHBoxLayout(d->headerWidget);
    d->headerLayout->setMargin(6);

    d->textLabel = new QLabel(d->headerWidget);
    d->headerLayout->addWidget(d->textLabel, 1);

    d->imageLabel = new QLabel(d->headerWidget);
    d->headerLayout->addWidget(d->imageLabel);

    d->titleWidgetLayout->addWidget(d->headerWidget);
    // default image / text part end

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(titleFrame);
    mainLayout->setMargin(0);
    setLayout(mainLayout);
}

KTitleWidget::~KTitleWidget()
{
    delete d;
}

void KTitleWidget::setWidget(QWidget *widget)
{
    d->titleWidgetLayout->addWidget(widget);
}

QString KTitleWidget::text() const
{
    return d->textLabel->text();
}

const QPixmap *KTitleWidget::pixmap() const
{
    return d->imageLabel->pixmap();
}

void KTitleWidget::setText(const QString &text, Qt::Alignment alignment)
{
    if (!Qt::mightBeRichText(text)) {
        d->textLabel->setStyleSheet("QLabel { font-weight: bold; }");
    }

    d->textLabel->setText(text);

    d->textLabel->setAlignment(alignment);
    d->headerWidget->setVisible(true);
}

void KTitleWidget::setPixmap(const QPixmap &pixmap, ImageAlignment alignment)
{
    if (alignment == ImageLeft) {
        d->headerLayout->removeWidget(d->textLabel); // remove text from 1st position...
        d->headerLayout->addWidget(d->textLabel, 1); // ... and move it to the end
    }

    if (alignment == ImageRight) {
        d->headerLayout->removeWidget(d->imageLabel);
        d->headerLayout->addWidget(d->imageLabel);
    }

    d->imageLabel->setPixmap(pixmap);
    d->headerWidget->setVisible(true);
}

#include "ktitlewidget.moc"
