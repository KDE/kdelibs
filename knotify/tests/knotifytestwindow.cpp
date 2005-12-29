/*
 * Copyright (C) 2005-2006 by Olivier Goffart <ogoffart at kde.org>
 */

#include "knotifytestwindow.h"
#include "knotification.h"
#include "knotifyconfigwidget.h"

#include <kstatusbar.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kstdaction.h>
#include <kaction.h>
#include <kstdaccel.h>
#include <kxmlguifactory.h>
#include <kdialogbase.h>

// ------------------------------------------------------------------------

KNotifyTestWindow::KNotifyTestWindow(QWidget *parent)
  : KMainWindow(parent)
{
	QWidget *w=new QWidget(this); 
	view.setupUi(w);
	statusBar()->message(i18n("Test program for KNotify"));
	setCaption( i18n("Test program for KNotify") );

	setCentralWidget(w);
  
	// set up the actions
	KStdAction::quit( this, SLOT( close() ), actionCollection() );
	KStdAction::keyBindings( guiFactory(), SLOT( configureShortcuts() ), actionCollection() );

	createGUI();
	
	connect ( view.b_online , SIGNAL(clicked()) , this , SLOT(slotSendOnlineEvent()));
	connect ( view.b_message , SIGNAL(clicked()) , this , SLOT(slotSendMessageEvent()));
	connect ( view.b_confG ,  SIGNAL(clicked()) , this , SLOT(slotConfigureG()));
	connect ( view.b_confC ,  SIGNAL(clicked()) , this , SLOT(slotConfigureC()));
	
}

void KNotifyTestWindow::slotSendOnlineEvent()
{
	KNotification::ContextList contexts;
	contexts.append( qMakePair( QString("group") , view.c_group->currentText() ) );
	KNotification::event("online" , i18n( "the contact %1 is now online" ).arg( view.c_name->text() ) ,
						 QPixmap() , this , QStringList() , contexts );
}

void KNotifyTestWindow::slotSendMessageEvent( )
{
	KNotification::ContextList contexts;
	contexts.append( qMakePair( QString("group") , view.c_group->currentText() ) );
	KNotification *n = KNotification::event("message" , i18n( "new message : %1" ).arg( view.c_text->text() ) ,
											QPixmap() , this , QStringList(i18n("Read")) , contexts );
	connect( n , SIGNAL(activated(unsigned int )), this , SLOT(slotMessageRead()));
}

void KNotifyTestWindow::slotMessageRead( )
{
	KMessageBox::information ( this , view.c_text->text() , i18n("reading message") );
}

void KNotifyTestWindow::slotConfigureG( )
{
	KNotifyConfigWidget::configure(this);
}

void KNotifyTestWindow::slotConfigureC( )
{
	KDialogBase dialog(this);
	KNotifyConfigWidget *w=new KNotifyConfigWidget(&dialog);
	w->setApplication(QString::null , "group", view.c_group->currentText());
	dialog.setMainWidget(w);
	if(dialog.exec())
	{
		w->save();
	}
}




#include "knotifytestwindow.moc"


