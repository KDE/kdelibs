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
#include "kinformationlabel.h"

// Qt includes
#include <QtCore/QLatin1String>
#include <QtCore/QTimer>
#include <QtGui/QLabel>
#include <QtGui/QHBoxLayout>
#include <QtGui/QMouseEvent>

// KDE includes
#include <kdebug.h>
#include <kicon.h>

class KInformationLabel::Private
{
public:
    Private(KInformationLabel *parent)
     : q(parent), iconLabel(0), messageLabel(0), iconType(KInformationLabel::Information),
       autoHideTimeout(0)
    {}

    KInformationLabel *q;
    QLabel *iconLabel;
    QLabel *messageLabel;

    KInformationLabel::Icon iconType;
    QIcon icon;
    int autoHideTimeout;

    /**
     * @brief Get the icon name from the icon type
     * @param type icon type from the enum
     * @return named icon as QString
     */
    QString iconTypeToIconName(KInformationLabel::Icon type);
    /**
     * @brief Update the icon for the label
     * @param icon a stock icon loaded using KIcon
     */
    void updateIcon(const KIcon &icon);

    void showWidget();
    void _k_timeoutFinished();
};

KInformationLabel::KInformationLabel(QWidget *parent)
 : QFrame(parent), d(new Private(this))
{
    // Set QFrame parameters
    setAutoFillBackground(true);
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Plain);
    setBackgroundRole(QPalette::Base);

    // Create icon label
    d->iconLabel = new QLabel(this);
    d->iconLabel->installEventFilter(this);
    d->iconLabel->setFixedSize(24, 24);

    // Create message label
    d->messageLabel = new QLabel(this);
    d->messageLabel->installEventFilter(this);
    d->messageLabel->setAlignment(Qt::AlignLeft);
    d->messageLabel->setStyleSheet( QLatin1String("QLabel { font-weight: bold }") );

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setMargin(3);
    mainLayout->addWidget(d->iconLabel,0,Qt::AlignVCenter);
    mainLayout->addWidget(d->messageLabel,0,Qt::AlignVCenter);
    mainLayout->addStretch(1);

    setLayout(mainLayout);

    // By default, the label is hidden for view and will only be
    // visible when a message
    setVisible(false);

    installEventFilter(this);

    // Set default icon (for designer)
    setIconType( KInformationLabel::Information );
}

KInformationLabel::~KInformationLabel()
{
    delete d;
}

bool KInformationLabel::eventFilter(QObject *object, QEvent *event)
{
    // Hide message label on click
    if( event->type() == QEvent::MouseButtonPress ) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if( mouseEvent && mouseEvent->button() == Qt::LeftButton ) {
            setVisible(false);
            return true;
        }
    }

    return QFrame::eventFilter(object, event);
}

QString KInformationLabel::text() const
{
    return d->messageLabel->text();
}

KInformationLabel::Icon KInformationLabel::iconType() const
{
    return d->iconType;
}

QIcon KInformationLabel::icon() const
{
    return d->icon;
}

int KInformationLabel::autoHideTimeout() const
{
    return d->autoHideTimeout;
}

void KInformationLabel::setText(const QString &text)
{
    if ( !text.isEmpty() ) {
        d->messageLabel->setText( text );
        d->showWidget();
    }
    else {
        setVisible(false);
    }
}

void KInformationLabel::setIconType(KInformationLabel::Icon iconType)
{
    d->iconType = iconType;
    d->updateIcon( KIcon(d->iconTypeToIconName(iconType)) );
}

void KInformationLabel::setIcon(const QIcon &icon)
{
    d->icon = icon;
    if ( d->iconType == KInformationLabel::Custom ) {
        d->updateIcon( KIcon(icon) );
    }
}

void KInformationLabel::setAutoHideTimeout(int msecs)
{
    d->autoHideTimeout = msecs;
}

void KInformationLabel::Private::updateIcon(const KIcon &icon)
{
    iconLabel->setPixmap( icon.pixmap(24, 24) );
}

QString KInformationLabel::Private::iconTypeToIconName(KInformationLabel::Icon iconType)
{
    QString icon;

    switch (iconType) {
        case KInformationLabel::Information:
            icon = QLatin1String("dialog-information");
            break;
        case KInformationLabel::Error:
            icon = QLatin1String("dialog-error");
            break;
        case KInformationLabel::Warning:
            icon = QLatin1String("dialog-warning");
            break;
        case KInformationLabel::Custom:
            break;
    }

    return icon;
}

void KInformationLabel::Private::_k_timeoutFinished()
{
    q->setVisible(false);
}

void KInformationLabel::Private::showWidget()
{
    q->setVisible(true);
    if ( autoHideTimeout > 0 ) {
        QTimer::singleShot(autoHideTimeout, q, SLOT(_k_timeoutFinished()));
    }
}

#include "kinformationlabel.moc"
// kate: space-indent on; indent-width 4; encoding utf-8; replace-tabs on;
