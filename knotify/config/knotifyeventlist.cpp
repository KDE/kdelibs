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

#include <kdebug.h>
#include <klocale.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <QtGui/QItemDelegate>
#include <QtGui/QPainter>

//BEGIN KNotifyEventListDelegate

class KNotifyEventList::KNotifyEventListDelegate : public QItemDelegate
{
	public:
		KNotifyEventListDelegate(QObject *parent = 0);

		virtual void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
	private:
};

KNotifyEventList::KNotifyEventListDelegate::KNotifyEventListDelegate(QObject *parent)
	: QItemDelegate(parent)
{
}

void KNotifyEventList::KNotifyEventListDelegate::paint( QPainter* painter,
		 const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
	if (index.column() != 2)
		return QItemDelegate::paint(painter, option, index);

	QVariant displayData = index.data(Qt::UserRole);
	QString prstring=displayData.toString();

	QItemDelegate::paint(painter, option, index);

// 	kDebug(300) << prstring;

	QRect rect=option.rect;


	int mc_x=0;
	QSize iconsSize = option.decorationSize;
	foreach(QString key , prstring.split ("|"))
	{
		QIcon icon;
		if(key == "Sound" )
			icon = KIcon("speaker");
		else if(key == "Popup" )
			icon = KIcon("document-properties");
		else if(key == "Execute" )
			icon = KIcon("exec");
		else if(key == "Taskbar" )
			icon = KIcon("services");
		else if(key == "Logfile" )
			icon = KIcon("text-plain");
		else
			continue;

		icon.paint(painter, rect.left() + mc_x + 4, rect.top() + (rect.height() - iconsSize.height()) / 2, iconsSize.width(), iconsSize.height());
		mc_x += iconsSize.width() + 4;
	}

}

//END KNotifyEventListDelegate

KNotifyEventList::KNotifyEventList(QWidget *parent)
 : QTreeWidget(parent)  , config(0)
{
  QStringList headerLabels;
  headerLabels << i18nc( "Title of the notified event", "Title" ) << i18nc( "Description of the notified event", "Description" ) << i18nc( "State of the notified event", "State" );
  setHeaderLabels( headerLabels );

  setItemDelegate(new KNotifyEventListDelegate(this));
  setRootIsDecorated(false);
  setAlternatingRowColors(true);

  connect(this, SIGNAL(currentItemChanged( QTreeWidgetItem * , QTreeWidgetItem *  )) , this , SLOT(slotSelectionChanged( QTreeWidgetItem * , QTreeWidgetItem *)));
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
        config->addConfigSources(KGlobal::dirs()->findAllResources("data",
                                 appname + '/' + appname + ".notifyrc"));
        config->reparseConfiguration();

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


KNotifyEventListItem::KNotifyEventListItem( QTreeWidget * parent, const QString & eventName,
				const QString & name, const QString & description , KConfig *config)
	: QTreeWidgetItem(parent) ,
	m_config(eventName, config )
{
  setText( 0, name );
  setText( 1, description );
  /*setSizeHint ( 2 , QSize(22,22) );
  setSizeHint ( 3 , QSize(22,22) );
  setSizeHint ( 4 , QSize(22,22) );
  setSizeHint ( 5 , QSize(22,22) );
  setSizeHint ( 6 , QSize(22,22) );*/
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
	setData(2 , Qt::UserRole , m_config.readEntry( "Action" ));
/*	QString prstring=m_config.readEntry( "Action" );
	QStringList actions=prstring.split ("|");

	QPixmap pexec = SmallIcon("exec");
	QPixmap pstderr = SmallIcon("terminal");
	QPixmap pmessage = SmallIcon("document-properties");
	QPixmap plogfile = SmallIcon("log");
	QPixmap psound = SmallIcon("sound");
	QPixmap ptaskbar = SmallIcon("kicker");

	setIcon(2 , actions.contains("Sound") ? QIcon(psound) : QIcon() );
	setIcon(3 , actions.contains("Popup") ? QIcon(pmessage) : QIcon() );
	setIcon(4 , actions.contains("Execute") ? QIcon(pexec) : QIcon() );
	setIcon(5 , actions.contains("Taskbar") ? QIcon(ptaskbar) : QIcon() );
	setIcon(6 , actions.contains("Logfile") ? QIcon(plogfile) : QIcon() );*/
}

#include "knotifyeventlist.moc"
