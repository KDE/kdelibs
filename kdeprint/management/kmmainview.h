#ifndef KMMAINVIEW_H
#define KMMAINVIEW_H

#include <qwidget.h>

class KMManager;
class KMPrinterView;
class KMPrinter;
class KMPages;
class KActionCollection;
class QPopupMenu;
class QTimer;
class QBoxLayout;

class KMMainView : public QWidget
{
	Q_OBJECT
public:
	KMMainView(QWidget *parent = 0, const char *name = 0);
	~KMMainView();

	void startTimer();
	void stopTimer();
	void setOrientation(int);

public slots:
	void slotTimer();

protected slots:
	void slotPrinterSelected(KMPrinter*);
	void slotShowMenu();
	void slotHideMenu();
	void slotChangeView(int);
	void slotRightButtonClicked(KMPrinter*, const QPoint&);
	void slotEnable();
	void slotDisable();
	void slotRemove();
	void slotConfigure();
	void slotAdd();
	void slotHardDefault();
	void slotSoftDefault();
	void slotChangeDirection(int);
	void slotTest();

protected:
	void initActions();
	void showErrorMsg(const QString& msg, bool usemgr = true);

private:
	KMPrinterView	*m_printerview;
	KMPages		*m_printerpages;
	QTimer		*m_timer;
	KMManager	*m_manager;
	QPopupMenu	*m_pop;
	KActionCollection	*m_actions;
	KMPrinter	*m_current;
	QBoxLayout	*m_layout;
};

#endif
