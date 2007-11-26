/*
 * Copyright (C) 2005-2006 by Olivier Goffart <ogoffart at kde.org>
 */

#include "knotifytestwindow.h"
#include "knotification.h"
#include "knotifyconfigwidget.h"

#include <kstatusbar.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kstandardaction.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kstandardshortcut.h>
#include <kxmlguifactory.h>
#include <kdialog.h>
#include <kstandarddirs.h>

// ------------------------------------------------------------------------

KNotifyTestWindow::KNotifyTestWindow(QWidget *parent)
  : KXmlGuiWindow(parent) , m_nbNewMessage(0)
{
	QWidget *w=new QWidget(this);
	view.setupUi(w);
//	statusBar()->message(i18n("Test program for KNotify"));
	setCaption( i18n("Test program for KNotify") );

        KGlobal::dirs()->addResourceDir( "data", KDESRCDIR );

	setCentralWidget(w);

	// set up the actions
        actionCollection()->addAction(KStandardAction::Quit, this, SLOT( close() ));
        actionCollection()->addAction(KStandardAction::KeyBindings, guiFactory(), SLOT( configureShortcuts() ));

	createGUI();

	connect ( view.b_online , SIGNAL(clicked()) , this , SLOT(slotSendOnlineEvent()));
	connect ( view.b_message , SIGNAL(clicked()) , this , SLOT(slotSendMessageEvent()));
	connect ( view.b_read , SIGNAL(clicked()) , this , SLOT(slotMessageRead()));
	connect ( view.b_confG ,  SIGNAL(clicked()) , this , SLOT(slotConfigureG()));
	connect ( view.b_confC ,  SIGNAL(clicked()) , this , SLOT(slotConfigureC()));

}

void KNotifyTestWindow::slotSendOnlineEvent()
{
	KNotification::ContextList contexts;
	contexts.append( qMakePair( QString("group") , view.c_group->currentText() ) );
        KNotification *n = new KNotification( "online", this );
        n->setText(i18n("the contact %1 is now online",  view.c_name->text() ));
        n->setContexts(contexts);
        n->sendEvent();
}

void KNotifyTestWindow::slotSendMessageEvent( )
{
	m_nbNewMessage++;
	if(!m_readNotif)
	{
		KNotification *n=new KNotification( "message", this, KNotification::Persistent );
		n->setText(i18n( "new message : %1" ,  view.c_text->toPlainText() ));
		n->setActions( QStringList( i18n("Read") ) );
		connect( n , SIGNAL(activated(unsigned int )), this , SLOT(slotMessageRead()));

		m_readNotif=n;
	}
	else
	{
		m_readNotif->setText(i18n("%1 new messages", m_nbNewMessage));
	}

	KNotification::ContextList cl;
	cl << qMakePair( QString("group") , view.c_group->currentText() );
	m_readNotif->setContexts( cl );
	m_readNotif->sendEvent();
}

void KNotifyTestWindow::slotMessageRead( )
{
	m_nbNewMessage=0;
	if(m_readNotif)
		m_readNotif->close();
	KMessageBox::information ( this , view.c_text->toPlainText() , i18n("reading message") );
}

void KNotifyTestWindow::slotConfigureG( )
{
	KNotifyConfigWidget::configure(this);
}

void KNotifyTestWindow::slotConfigureC( )
{
	KDialog dialog(this);
	KNotifyConfigWidget *w=new KNotifyConfigWidget(&dialog);
	w->setApplication(QString() , "group", view.c_group->currentText());
	dialog.setMainWidget(w);
	if(dialog.exec())
	{
		w->save();
	}
}




#include "knotifytestwindow.moc"


