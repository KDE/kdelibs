#ifndef KMDBCREATOR_H
#define KMDBCREATOR_H

#include <qobject.h>
#include <qstring.h>
#include <qdatetime.h>
#include <kprocess.h>

class QWidget;
class QProgressDialog;

class KMDBCreator : public QObject
{
	Q_OBJECT;
public:
	KMDBCreator(QObject *parent = 0, const char *name = 0);
	~KMDBCreator();

	bool checkDriverDB(const QString& dirname, const QDateTime& d);
	bool createDriverDB(const QString& dirname, const QString& filename, QWidget *parent = 0);
	bool status() const	{ return m_status; }

protected slots:
	void slotReceivedStdout(KProcess *p, char *bufm, int len);
	void slotReceivedStderr(KProcess *p, char *bufm, int len);
	void slotProcessExited(KProcess *p);
	void slotCancelled();

signals:
	void dbCreated();

private:
	KProcess	m_proc;
	QProgressDialog	*m_dlg;
	bool		m_status;
	bool		m_firstflag;
};

#endif
