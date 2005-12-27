/*
 * Copyright (C) 2005-2006 by Olivier Goffart <ogoffart at kde.org>
 */

#ifndef __KNotifyTestWINDOW_H__
#define __KNotifyTestWINDOW_H__

#include <kmainwindow.h>
#include "knotifytestview.h"

class KNotifyTestWindow : public KMainWindow
{
	Q_OBJECT
	
	public:
		KNotifyTestWindow( QWidget* parent = 0);
	
	private:
		Ui::KNotifyTestView view;

	public slots:
		void slotSendOnlineEvent();
		void slotSendMessageEvent();
		void slotMessageRead();
		
		void slotConfigureG();
		void slotConfigureC();
};



#endif
