#ifndef KDXS_CHANGES_H
#define KDXS_CHANGES_H

#include <kdialog.h>

class KHTMLPart;

class KDXSChanges : public KDialog
{
Q_OBJECT
public:
	KDXSChanges(QWidget *parent);
	void addChangelog(QString version, QString log);
	void finish();
private:
	void prepare();

	KHTMLPart *m_part;
};

#endif
