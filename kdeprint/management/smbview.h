#ifndef SMBVIEW_H
#define SMBVIEW_H

#include <klistview.h>

class KProcess;

class SmbView : public KListView
{
	Q_OBJECT
public:
	SmbView(QWidget *parent = 0, const char *name = 0);
	~SmbView();

	void setLoginInfos(const QString& login, const QString& password);
	void setOpen(QListViewItem*, bool);
	void init();
	void abort();

signals:
	void printerSelected(const QString& work, const QString& server, const QString& printer);
	void running(bool);

protected:
	void startProcess(int);
	void endProcess();
	void processGroups();
	void processServers();
	void processShares();

protected slots:
	void slotReceivedStdout(KProcess*, char*, int);
	void slotProcessExited(KProcess*);
	void slotSelectionChanged(QListViewItem*);

private:
	enum State { GroupListing, ServerListing, ShareListing, Idle };
	int 		m_state;
	QListViewItem	*m_current;
	KProcess	*m_proc;
	QString		m_buffer;
	QString		m_login, m_password;
};

#endif
