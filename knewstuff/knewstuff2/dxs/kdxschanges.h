#ifndef KDXS_CHANGES_H
#define KDXS_CHANGES_H

#include <kdialogbase.h>

class KHTMLPart;

class KDXSChanges : public KDialogBase
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
