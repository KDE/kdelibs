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

#include <kicon.h>
#include <kiconloader.h>

class KTitleWidget::Private
{
public:
//    QHBoxLayout *titleWidgetLayout;
    QGridLayout *headerLayout;
    QLabel *imageLabel;
    QLabel *textLabel;
    QLabel *commentLabel;
//    QWidget *headerWidget;
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

    // default image / text part start
    d->headerLayout = new QGridLayout(titleFrame);
    d->headerLayout->setColumnStretch(0, 1);
    d->headerLayout->setMargin(6);

    d->textLabel = new QLabel(titleFrame);
    d->headerLayout->addWidget(d->textLabel, 0, 0);

    d->imageLabel = new QLabel(titleFrame);
    d->headerLayout->addWidget(d->imageLabel, 0, 1, 1, 2);

    d->commentLabel = new QLabel(titleFrame);
    d->headerLayout->addWidget(d->commentLabel, 1, 0);

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
    d->headerLayout->addWidget(widget, 2, 0, 1, 2);
}

QString KTitleWidget::text() const
{
    return d->textLabel->text();
}

QString KTitleWidget::comment() const
{
    return d->commentLabel->text();
}

const QPixmap *KTitleWidget::pixmap() const
{
    return d->imageLabel->pixmap();
}

void KTitleWidget::setBuddy(QWidget *buddy)
{
    d->textLabel->setBuddy(buddy);
}

void KTitleWidget::setText(const QString &text, Qt::Alignment alignment)
{
    if (!Qt::mightBeRichText(text)) {
        d->textLabel->setStyleSheet("QLabel { font-weight: bold; }");
    }

    d->textLabel->setText(text);

    d->textLabel->setAlignment(alignment);
//    d->headerWidget->setVisible(true);
}

void KTitleWidget::setComment(const QString &comment, CommentType type)
{
    QString styleSheet;
    switch (type) {
        //FIXME: we need the usability color styles to implement different
        //       yet palette appropriate colours for the different use cases!
        //       also .. should we include an icon here,
        //       perhaps using the imageLabel?
        case InfoMessage:
        case WarningMessage:
        case ErrorMessage:
            styleSheet = QString("QLabel { font-color: %1; background: %2 }")
                                .arg(palette().color(QPalette::Active,
                                                     QPalette::HighlightedText).name(),
                                     palette().color(QPalette::Active,
                                                     QPalette::Highlight).name());
            break;
        case PlainMessage:
        default:
            break;
    }

    d->commentLabel->setStyleSheet(styleSheet);
    d->commentLabel->setText(comment);
}

void KTitleWidget::setPixmap(const QPixmap &pixmap, ImageAlignment alignment)
{
    d->headerLayout->removeWidget(d->textLabel);
    d->headerLayout->removeWidget(d->commentLabel);
    d->headerLayout->removeWidget(d->imageLabel);

    if (alignment == ImageLeft) {
        // swap the text and image labels around
        d->headerLayout->addWidget(d->imageLabel, 0, 0, 2, 1);
        d->headerLayout->addWidget(d->textLabel, 0, 1);
        d->headerLayout->addWidget(d->commentLabel, 1, 1);
        d->headerLayout->setColumnStretch(0, 0);
        d->headerLayout->setColumnStretch(1, 1);
    }

    if (alignment == ImageRight) {
        d->headerLayout->addWidget(d->textLabel, 0, 0);
        d->headerLayout->addWidget(d->commentLabel, 1, 0);
        d->headerLayout->addWidget(d->imageLabel, 0, 1, 2, 1);
        d->headerLayout->setColumnStretch(1, 0);
        d->headerLayout->setColumnStretch(0, 1);
    }

    d->imageLabel->setPixmap(pixmap);
//    d->headerWidget->setVisible(true);
}


void KTitleWidget::setPixmap(const QString &icon, ImageAlignment alignment)
{
    setPixmap(KIcon(icon), alignment);
}

void KTitleWidget::setPixmap(const QIcon& icon, ImageAlignment alignment)
{
    setPixmap(icon.pixmap(IconSize(K3Icon::Dialog)), alignment);
}

#include "ktitlewidget.moc"
