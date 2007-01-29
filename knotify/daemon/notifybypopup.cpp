/*
   Copyright (C) 2005-2006 by Olivier Goffart <ogoffart at kde.org>


   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 */

#include "notifybypopup.h"
#include "knotifyconfig.h"

#include <kdebug.h>
#include <kpassivepopup.h>
#include <kiconloader.h>
#include <kdialog.h>
#include <khbox.h>
#include <kvbox.h>
#include <QLabel>
#include <QTextDocument>
#include <kconfiggroup.h>

NotifyByPopup::NotifyByPopup(QObject *parent) : KNotifyPlugin(parent)
{
}


NotifyByPopup::~NotifyByPopup()
{
	qDeleteAll(m_popups);
}

void NotifyByPopup::notify( int id, KNotifyConfig * config )
{
	kDebug(300) << k_funcinfo << id << endl;
	if(m_popups.contains(id))
	{
		//the popup is already shown
		finish(id);
		return;
	}

	m_popups[id]=showPopup(id,config);
}

void NotifyByPopup::slotPopupDestroyed( )
{
	const QObject *s=sender();
	if(!s)
		return;
	QHash<int,KPassivePopup*>::iterator it;
	for(it=m_popups.begin() ; it!=m_popups.end(); ++it   )
	{
		QObject *o=it.value();
		if(o && o == s)
		{
			finish(it.key());
			m_popups.remove(it.key());
			break;
		}
	}
}

void NotifyByPopup::slotLinkClicked( const QString &adr )
{
	unsigned int id=adr.section("/" , 0 , 0).toUInt();
	unsigned int action=adr.section("/" , 1 , 1).toUInt();

//	kDebug(300) << k_funcinfo << id << " " << action << endl;
        
	if(id==0 || action==0)
		return;
		
	emit actionInvoked(id,action);
}

void NotifyByPopup::close( int id )
{
	delete m_popups[id];
	m_popups.remove(id);
}

void NotifyByPopup::update(int id, KNotifyConfig * config)
{
	if(!m_popups.contains(id))
		return;
	KPassivePopup *p=m_popups[id];
	m_popups.remove(id);
	delete p;
	
	//FIXME:  the popup should not be closed, it should be updated as it.
	m_popups.insert(id, showPopup(id, config) );
}

KPassivePopup * NotifyByPopup::showPopup(int id,KNotifyConfig * config)
{
	const QString &appname=config->appname;
	
	KPassivePopup *pop = new KPassivePopup( config->winId );
	
	KConfigGroup globalgroup( &(*config->eventsfile), "Global" );
	QString iconName = globalgroup.readEntry( "IconName", appname );
	KIconLoader iconLoader( appname );
	QPixmap appIcon = iconLoader.loadIcon( iconName, K3Icon::Small );
	QString appCaption = globalgroup.readEntry( "Name", appname );

	KVBox *vb = pop->standardView( appCaption , config->pix.isNull() ? config->text : QString() , appIcon );
	KVBox *vb2=vb;

	if(!config->pix.isNull())
	{
		const QPixmap &pix=config->pix;
		KHBox *hb = new KHBox(vb);
		hb->setSpacing(KDialog::spacingHint());
		QLabel *pil=new QLabel(hb);
		pil->setPixmap( config->pix );
		pil->setScaledContents(true);
		if(pix.height() > 80 && pix.height() > pix.width() )
		{
			pil->setMaximumHeight(80);
			pil->setMaximumWidth(80*pix.width()/pix.height());
		}
		else if(pix.width() > 80 && pix.height() <= pix.width())
		{
			pil->setMaximumWidth(80);
			pil->setMaximumHeight(80*pix.height()/pix.width());
		}
		vb=new KVBox(hb);
		QLabel *msg = new QLabel( config->text, vb );
		msg->setAlignment( Qt::AlignLeft );
	}


	if ( !config->actions.isEmpty() )
	{
		QString linkCode=QString::fromLatin1("<p align=\"right\">");
		int i=0;
		foreach ( const QString & it , config->actions ) 
		{
			i++;
			linkCode+=QString::fromLatin1("&nbsp;<a href=\"%1/%2\">%3</a> ").arg( id ).arg( i ).arg( Qt::escape(it) );
		}
		linkCode+=QString::fromLatin1("</p>");
		QLabel *link = new QLabel(linkCode , vb );
		link->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
		link->setOpenExternalLinks(false);
		//link->setAlignment( AlignRight );
		QObject::connect(link, SIGNAL(linkActivated(const QString &)), this, SLOT(slotLinkClicked(const QString& ) ) );
		QObject::connect(link, SIGNAL(linkActivated(const QString &)), pop, SLOT(hide()));
	}

	pop->setAutoDelete( true );
	
	
	connect(pop, SIGNAL(destroyed()) , this, SLOT(slotPopupDestroyed()) );
	pop->setTimeout( 0 );
	pop->setView( vb2 );
	pop->show();
	return pop;
}

#include "notifybypopup.moc"

