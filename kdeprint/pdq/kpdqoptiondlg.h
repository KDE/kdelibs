#ifndef KPDQOPTIONDLG_H
#define KPDQOPTIONDLG_H

#include <qdialog.h>

class QLineEdit;

class KPdqOptionDlg : public QDialog
{
	Q_OBJECT
public:
	KPdqOptionDlg(QWidget *parent = 0, const char *name = 0);
	~KPdqOptionDlg();

	static bool configure(QWidget *parent = 0);

protected slots:
	void slotBrowse();

private:
	QLineEdit	*rcfile_;
};

#endif
