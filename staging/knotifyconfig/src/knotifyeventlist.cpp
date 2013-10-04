/* This file is part of the KDE libraries
   Copyright (C) 2005 Olivier Goffart <ogoffart at kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "knotifyeventlist.h"

#include <QDebug>
#include <klocalizedstring.h>
#include <kconfig.h>
#include <kconfiggroup.h>

#include <QStandardPaths>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QHeaderView>
#include <QFontMetrics>

//BEGIN KNotifyEventListDelegate

class KNotifyEventList::KNotifyEventListDelegate : public QStyledItemDelegate
{
	public:
		KNotifyEventListDelegate(QObject *parent = 0);

		virtual void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
	private:
};

KNotifyEventList::KNotifyEventListDelegate::KNotifyEventListDelegate(QObject *parent)
	: QStyledItemDelegate(parent)
{
}

void KNotifyEventList::KNotifyEventListDelegate::paint( QPainter* painter,
		 const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
	if (index.column() != 0)
		return QStyledItemDelegate::paint(painter, option, index);

	QVariant displayData = index.data(Qt::UserRole);
	QString prstring=displayData.toString();

	QStyledItemDelegate::paint(painter, option, index);

// 	qDebug() << prstring;

	QRect rect=option.rect;

	QStringList optionsList = prstring.split ('|');
	QList<QIcon> iconList;
	iconList << ( optionsList.contains("Sound")   ? QIcon::fromTheme("media-playback-start") : QIcon() );
	iconList << ( optionsList.contains("Popup")   ? QIcon::fromTheme("dialog-information")   : QIcon() );
	iconList << ( optionsList.contains("Logfile") ? QIcon::fromTheme("text-x-generic")       : QIcon() );
	iconList << ( optionsList.contains("Taskbar") ? QIcon::fromTheme("services")             : QIcon() );
	iconList << ( optionsList.contains("Execute") ? QIcon::fromTheme("system-run")           : QIcon() );
	if( KNotifyConfigElement::have_kttsd() )
		iconList << ( optionsList.contains("KTTS") ? QIcon::fromTheme("text-speak") : QIcon() );

	int mc_x=0;

	int iconWidth = option.decorationSize.width();
	int iconHeight = option.decorationSize.height();
	foreach(const QIcon &icon, iconList)
	{
		icon.paint(painter, rect.left() + mc_x + 4, rect.top() + (rect.height() - iconHeight) / 2, iconWidth, iconHeight);
		mc_x += iconWidth + 4;
	}
}

//END KNotifyEventListDelegate

KNotifyEventList::KNotifyEventList(QWidget *parent)
 : QTreeWidget(parent)  , config(0)
{
	QStringList headerLabels;
	headerLabels << i18nc( "State of the notified event", "State" ) << i18nc( "Title of the notified event", "Title" ) << i18nc( "Description of the notified event", "Description" );
	setHeaderLabels( headerLabels );

	setItemDelegate(new KNotifyEventListDelegate(this));
	setRootIsDecorated(false);
	setAlternatingRowColors(true);

	//Extract icon size as the font height (as h=w on icons)
	QStyleOptionViewItem iconOption;
	iconOption.initFrom( this );
	int iconWidth = iconOption.fontMetrics.height() -2 ; //1px margin top & bottom
	setIconSize( QSize(iconWidth, iconWidth) );

	header()->setSectionResizeMode( 0, QHeaderView::Fixed );
	header()->resizeSection( 0, KNotifyConfigElement::have_kttsd() ? (iconWidth+4)*6: (iconWidth+4)*5 );
	header()->setSectionResizeMode( 1, QHeaderView::ResizeToContents );

	connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)) , this , SLOT(slotSelectionChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
}


KNotifyEventList::~KNotifyEventList()
{
	delete config;
}

void KNotifyEventList::fill( const QString & appname , const QString & context_name ,const QString & context_value )
{
	m_elements.clear();
	clear();
	delete config;
	config = new KConfig(appname + ".notifyrc" , KConfig::NoGlobals);
        config->addConfigSources(QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                           appname + '/' + appname + ".notifyrc"));

	QStringList conflist = config->groupList();
	QRegExp rx("^Event/([^/]*)$");
	conflist=conflist.filter( rx );

	foreach (const QString& group , conflist )
	{
		KConfigGroup cg(config, group);
		rx.indexIn(group);
		QString id=rx.cap(1);

		if(!context_name.isEmpty())
		{
			QStringList contexts = cg.readEntry("Contexts", QStringList());
			if(!contexts.contains(context_name))
				continue;

			id=id+'/'+context_name+'/'+context_value;
		}
		QString name = cg.readEntry("Name");
		QString description = cg.readEntry("Comment");

		m_elements << new KNotifyEventListItem(this, id, name, description, config );
	}

	resizeColumnToContents(2);
}

void KNotifyEventList::save( )
{
	foreach( KNotifyEventListItem *it , m_elements )
	{
		it->save();
	}
}

void KNotifyEventList::slotSelectionChanged(  QTreeWidgetItem *current , QTreeWidgetItem *previous)
{
	Q_UNUSED( current );

	KNotifyEventListItem *it=dynamic_cast<KNotifyEventListItem *>(currentItem());
	if(it)
		emit eventSelected( it->configElement() );
	else
		emit eventSelected( 0l );

	it=dynamic_cast<KNotifyEventListItem *>(previous);
	if(it)
		it->update();
}

void KNotifyEventList::updateCurrentItem()
{
	KNotifyEventListItem *it=dynamic_cast<KNotifyEventListItem *>(currentItem());
	if(it)
		it->update();
}

QSize KNotifyEventList::sizeHint() const
{
    int fontSize = fontMetrics().height();
    return QSize(48 * fontSize, 12 * fontSize);
}


KNotifyEventListItem::KNotifyEventListItem( QTreeWidget * parent, const QString & eventName,
				const QString & name, const QString & description , KConfig *config)
	: QTreeWidgetItem(parent) ,
	m_config(eventName, config )
{
	setText( 1, name );
	setToolTip( 1, description );
	setText( 2, description );
	setToolTip( 2, description );
	update();
}

KNotifyEventListItem::~KNotifyEventListItem()
{
}

void KNotifyEventListItem::save()
{
	m_config.save();
}

void KNotifyEventListItem::update()
{
	setData(0 , Qt::UserRole , m_config.readEntry( "Action" ));
}

