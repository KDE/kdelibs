#ifndef KMJOBVIEWER_H
#define KMJOBVIEWER_H

#include <kmainwindow.h>
#include "kmprinterpage.h"

class KMJobManager;
class KMJob;
class QListView;
class JobItem;
class QPopupMenu;
class QListViewItem;
class KMPrinter;

class KMJobViewer : public KMainWindow, public KMPrinterPage
{
	Q_OBJECT
public:
	KMJobViewer(QWidget *parent = 0, const char *name = 0);
	~KMJobViewer();

	void addPrinter(const QString& prname);
	void setPrinter(const QString& prname);
	void setPrinter(KMPrinter *p);
	void refresh();
	void selectAll();

signals:
	void jobsShown();

protected slots:
	void slotSelectionChanged();
	void slotHold();
	void slotResume();
	void slotRemove();
	void slotRightClicked(QListViewItem*,const QPoint&,int);
	void slotMove(int prID);
	void slotPrinterToggled(bool);
	void slotAllPrinters();
	void slotRefresh();
	void slotShowMenu();
	void slotHideMenu();
	void slotOnItem(QListViewItem*);
	void slotOnViewport();

protected:
	void init();
	void updateJobs();
	void initActions();
	void initPrinterActions();
	JobItem* findItem(int ID);
	void jobSelection(QList<KMJob>& l);
	void send(int cmd, const QString& name, const QString& arg = QString::null);
	void loadPrinters();

private:
	KMJobManager		*m_manager;
	QListView		*m_view;
	QList<KMJob>		m_jobs;
	QList<JobItem>		m_items;
	QPopupMenu		*m_pop;
	QList<KMPrinter>	m_printers;
};

#endif
