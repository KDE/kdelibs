/*
 * Copyright (C) 2005-2006 by Olivier Goffart <ogoffart at kde.org>
 */

#ifndef __KNotifyTestWINDOW_H__
#define __KNotifyTestWINDOW_H__

#include <kxmlguiwindow.h>
#include "ui_knotifytestview.h"
#include <QtCore/QPointer>

class KNotification;


class KNotifyTestWindow : public KXmlGuiWindow
{
	Q_OBJECT
	
	public:
		KNotifyTestWindow( QWidget* parent = 0);
	
	private:
		Ui::KNotifyTestView view;
		QPointer<KNotification> m_readNotif;
		int m_nbNewMessage;

	public Q_SLOTS:
		void slotSendOnlineEvent();
		void slotSendMessageEvent();
		void slotMessageRead();
		
		void slotConfigureG();
		void slotConfigureC();
};



#endif
