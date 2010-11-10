/* This file is part of the KDE libraries
   Copyright (C) 2010 Teo Mrnjavac <teo@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kaboutapplicationpersonlistdelegate_p.h"

#include "kaboutapplicationpersonmodel_p.h"
#include "kaboutapplicationpersonlistview_p.h"
#include "kdeui/widgets/ktoolbar.h"
#include "kdeui/actions/kaction.h"
#include "kdeui/icons/kicon.h"

#include <kdecore/io/kdebug.h>
#include <kdecore/kernel/kstandarddirs.h>
#include <kdecore/kernel/ktoolinvocation.h>

#include <QtGui/QApplication>
#include <QtGui/QPainter>

namespace KDEPrivate
{

static const int AVATAR_HEIGHT = 50;
static const int AVATAR_WIDTH = 50;
static const int MAIN_LINKS_HEIGHT = 32;
static const int SOCIAL_LINKS_HEIGHT = 26;
static const int MAX_SOCIAL_LINKS = 9;

KAboutApplicationPersonListDelegate::KAboutApplicationPersonListDelegate(
        QAbstractItemView *itemView,
        QObject *parent )
    : KWidgetItemDelegate( itemView, parent )
{
}

QList< QWidget *> KAboutApplicationPersonListDelegate::createItemWidgets() const
{
    QList< QWidget *> list;

    QLabel *textLabel = new QLabel( itemView() );
    list.append( textLabel );


    KToolBar *mainLinks = new KToolBar( itemView(), false, false );

    KAction *emailAction = new KAction( KIcon( "internet-mail" ),
                                        i18n("Email contributor"),
                                        mainLinks );
    emailAction->setVisible( false );
    mainLinks->addAction( emailAction );
    KAction *homepageAction = new KAction( KIcon( "applications-internet" ),
                                           i18n("Visit contributor's homepage"),
                                           mainLinks );
    homepageAction->setVisible( false );
    mainLinks->addAction( homepageAction );
    KAction *visitProfileAction = new KAction( KIcon( "get-hot-new-stuff" ), "", mainLinks );
    visitProfileAction->setVisible( false );
    mainLinks->addAction( visitProfileAction );

    list.append( mainLinks );


    KToolBar *socialLinks = new KToolBar( itemView(), false, false );
    for( int i = 0; i < MAX_SOCIAL_LINKS; ++i ) {
        KAction *action = new KAction( KIcon( "applications-internet" ), "", socialLinks );
        action->setVisible( false );
        socialLinks->addAction( action );
    }

    list.append( socialLinks );

    connect( mainLinks, SIGNAL( actionTriggered( QAction * ) ),
             this, SLOT( launchUrl( QAction * ) ) );
    connect( socialLinks, SIGNAL( actionTriggered( QAction * ) ),
             this, SLOT( launchUrl( QAction * ) ) );

    return list;
}

void KAboutApplicationPersonListDelegate::updateItemWidgets( const QList<QWidget *> widgets,
                                                             const QStyleOptionViewItem &option,
                                                             const QPersistentModelIndex &index ) const
{
    int margin = option.fontMetrics.height() / 2;

    KAboutApplicationPersonProfile profile = index.data().value< KAboutApplicationPersonProfile >();


    QRect widgetsRect;
    if( qobject_cast< const KAboutApplicationPersonModel * >( index.model() )->hasAvatarPixmaps() ) {
        widgetsRect = QRect( option.rect.left() + AVATAR_WIDTH + 3 * margin,
                             margin,
                             option.rect.width() - AVATAR_WIDTH - 4 * margin,
                             option.fontMetrics.height() * 2 );
    }
    else {
        widgetsRect = QRect( option.rect.left() + margin,
                             margin,
                             option.rect.width() - 2*margin,
                             option.fontMetrics.height() * 2 );
    }

    //Let's fill in the text first...
    QLabel *label = qobject_cast< QLabel * >( widgets.at( TextLabel ) );
    label->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    QString text;
    text += "<b>" + profile.name() +"</b>";
    text += profile.task().isEmpty() ? "" : "<br><i>" + profile.task() + "</i>";
    text += profile.location().isEmpty() ? "" : "<br>" + profile.location();

    int labelRows = text.count( "<br>" ) + 1;
    label->move( widgetsRect.left(), widgetsRect.top() );
    label->resize( widgetsRect.width(), option.fontMetrics.height() * labelRows + margin );
    label->setContentsMargins( 0, 0, 0, 0 );
    label->setAlignment( Qt::AlignBottom | Qt::AlignLeft );

    label->setText( text );

    //And now we fill in the main links (email + homepage + OCS profile)...
    KToolBar *mainLinks = qobject_cast< KToolBar * >( widgets.at( MainLinks ) );
    mainLinks->setIconSize( QSize( 22, 22 ) );
    mainLinks->setContentsMargins( 0, 0, 0, 0 );
    mainLinks->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    KAction *action;
    action = qobject_cast< KAction * >( mainLinks->actions().at( EmailAction ) );
    action->setToolTip( profile.email() );
    action->setData( QString( "mailto:" + profile.email() ) );
    action->setVisible( true );
    if( !profile.homepage().isEmpty() ) {
        action = qobject_cast< KAction * >( mainLinks->actions().at( HomepageAction ) );
        action->setToolTip( profile.homepage().url() );
        action->setData( profile.homepage().url() );
        action->setVisible( true );
    }
    if( !profile.ocsProfileUrl().isEmpty() ) {
        action = qobject_cast< KAction * >( mainLinks->actions().at( VisitProfileAction ) );
        KAboutApplicationPersonModel *model = qobject_cast< KAboutApplicationPersonModel * >( itemView()->model() );
        action->setToolTip( i18n( "Visit contributor's profile on %1\n%2",
                                  model->providerName(),
                                  profile.ocsProfileUrl() ) );
        action->setData( profile.ocsProfileUrl() );
        action->setVisible( true );
    }
    mainLinks->resize( QSize( mainLinks->sizeHint().width(), MAIN_LINKS_HEIGHT ) );
    mainLinks->move( widgetsRect.left(), widgetsRect.top() + label->height() );

    //Finally, the social links...
    KToolBar *socialLinks = qobject_cast< KToolBar * >( widgets.at( SocialLinks ) );
    socialLinks->setIconSize( QSize( 16, 16 ) );
    socialLinks->setContentsMargins( 0, 0, 0, 0 );
    socialLinks->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    int currentSocialLinkAction = 0;
    foreach( KAboutApplicationPersonProfileOcsLink link, profile.ocsLinks() ) {
        if( !profile.homepage().isEmpty() && profile.homepage() == link.url() )
            continue;   //We skip it if it's the same as the homepage from KAboutData

        action = qobject_cast< KAction * >( socialLinks->actions().at( currentSocialLinkAction ) );
        action->setToolTip( i18n( "Visit contributor's profile on %1\n%2",
                                  link.prettyType(),
                                  link.url().url() ) );
        action->setIcon( link.icon() );
        action->setData( link.url().url() );
        action->setVisible( true );

        currentSocialLinkAction++;
        if( currentSocialLinkAction > MAX_SOCIAL_LINKS - 1 )
            break;
    }

    socialLinks->resize( QSize( socialLinks->sizeHint().width(), SOCIAL_LINKS_HEIGHT ) );
    socialLinks->move( widgetsRect.left() + mainLinks->width(),
                       widgetsRect.top() + label->height() +
                       ( MAIN_LINKS_HEIGHT - SOCIAL_LINKS_HEIGHT ) / 2 );

    itemView()->reset();
}

QSize KAboutApplicationPersonListDelegate::sizeHint( const QStyleOptionViewItem &option,
                                                     const QModelIndex &index ) const
{
    KAboutApplicationPersonProfile profile = index.data().value< KAboutApplicationPersonProfile >();
    bool hasLocation = !profile.location().isEmpty();
    bool hasAvatar = !profile.avatar().isNull();

    int textHeight = hasLocation ? option.fontMetrics.height() * 3
                                 : option.fontMetrics.height() * 2;

    int margin = option.fontMetrics.height() / 2;

    int height = hasAvatar ? qMax( textHeight + MAIN_LINKS_HEIGHT + 2*margin,
                                   AVATAR_HEIGHT + 2*margin )
                           : textHeight + MAIN_LINKS_HEIGHT + 2*margin;

    QSize metrics( option.fontMetrics.height() * 7, height );
    return metrics;
}

void KAboutApplicationPersonListDelegate::paint( QPainter *painter,
                                                 const QStyleOptionViewItem &option,
                                                 const QModelIndex &index) const
{
    int margin = option.fontMetrics.height() / 2;

    QStyle *style = QApplication::style();
    style->drawPrimitive(QStyle::PE_Widget, &option, painter, 0);

    const KAboutApplicationPersonModel * model = qobject_cast< const KAboutApplicationPersonModel * >(index.model());

    if ( model->hasAvatarPixmaps() ) {
        int height = option.rect.height();
        QPoint point( option.rect.left() + 2 * margin,
                      option.rect.top() + ( (height - AVATAR_HEIGHT) / 2) );

        KAboutApplicationPersonProfile profile = index.data().value< KAboutApplicationPersonProfile >();

        if( !profile.avatar().isNull() ) {
            QPixmap pixmap = profile.avatar();

            point.setX( ( AVATAR_WIDTH - pixmap.width() ) / 2 + 5 );
            point.setY( option.rect.top() + ( ( height - pixmap.height() ) / 2 ) );
            painter->drawPixmap( point, pixmap );

            QPoint framePoint( point.x() - 5, point.y() - 5 );
            QPixmap framePixmap = QPixmap( KStandardDirs::locate( "data", "kdeui/pics/thumb_frame.png" ) );
            painter->drawPixmap( framePoint, framePixmap.scaled( pixmap.width() + 10, pixmap.height() + 10 ) );
        }
    }
}

void KAboutApplicationPersonListDelegate::launchUrl( QAction *action ) const
{
    QString url = action->data().toString();
    if( !url.isEmpty() )
        KToolInvocation::invokeBrowser( url );
}

} //namespace KDEPrivate
