/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef KMMAINVIEW_H
#define KMMAINVIEW_H

#if !defined( _KDEPRINT_COMPILE ) && defined( __GNUC__ )
#warning internal header, do not use except if you are a KDEPrint developer
#endif

#include <qwidget.h>
#include <kdeprint/kpreloadobject.h>
#include <kmainwindow.h>

class KMManager;
class KMPrinterView;
class KMPrinter;
class KMPages;
class KActionCollection;
class QMenu;
class QTimer;
class QSplitter;
class KToolBar;
class KAction;
class PluginComboBox;
class QBoxLayout;
class MessageWindow;
class QMenuBar;

/**
 * @internal
 * This class is internal to KDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a KDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class KDEPRINT_EXPORT KMMainView : public QWidget, public KPReloadObject
{
	Q_OBJECT
public:
	KMMainView(QWidget *parent = 0, const char *name = 0, KActionCollection *coll = 0);
	~KMMainView();

	void setOrientation(int);
	int orientation() const;
	void setViewType(int);
	int viewType() const;
	void enableToolbar(bool on = true);
	KAction* action(const char *name);
	void showPrinterInfos(bool on);
	bool printerInfosShown() const;

public slots:
	void slotTimer();
	void slotShowPrinterInfos(bool);
	void slotChangePrinterState();
	void slotRemove();
	void slotConfigure();
	void slotAdd();
	void slotHardDefault();
	void slotSoftDefault();
	void slotTest();
	void slotServerRestart();
	void slotServerConfigure();
	void slotManagerConfigure();
	void slotAddSpecial();
	void slotRefresh();
	void slotToolSelected(int);
	void slotToggleFilter(bool);
	void slotHelp();

protected slots:
	void slotPrinterSelected(const QString&);
	void slotRightButtonClicked(const QString&, const QPoint&);
	void slotToggleToolBar(bool);
	void slotToggleMenuBar(bool);
	void slotChangeView(int);
	void slotChangeDirection(int);
	void slotUpdatePossible( bool );
	void slotInit();

protected:
	void initActions();
	void showErrorMsg(const QString& msg, bool usemgr = true);
	void restoreSettings();
	void saveSettings();
	void loadParameters();
	void reload();
	void configChanged();
	//void aboutToReload();
	void loadPluginActions();
	void removePluginActions();
	void createMessageWindow( const QString&, int delay = 500 );
	void destroyMessageWindow();
	void reset( const QString& msg = QString::null, bool useDelay = true, bool holdTimer = true );

private:
	KMPrinterView	*m_printerview;
	KMPages		*m_printerpages;
	QMenu	*m_pop;
	KActionCollection	*m_actions;
	KMPrinter	*m_current;
	KToolBar	*m_toolbar;
	PluginComboBox	*m_plugin;
	int		m_pactionsindex;
	QStringList	m_toollist;
	bool		m_first;
	QBoxLayout	*m_boxlayout;
	class KMainWindowPrivate;
	KMainWindowPrivate *d;
	KToolBar *m_menubar;
};

KDEPRINT_EXPORT int kdeprint_management_add_printer_wizard( QWidget* parent );

#endif
