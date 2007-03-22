#ifndef KNEWSTUFF2_UI_KDXSCHANGES_H
#define KNEWSTUFF2_UI_KDXSCHANGES_H

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
