#ifndef KDXS_COMMENTS_H
#define KDXS_COMMENTS_H

#include <kdialogbase.h>

class KHTMLPart;

class KDXSComments : public KDialogBase
{
Q_OBJECT
public:
	KDXSComments(QWidget *parent);
	void addComment(QString username, QString comment);
	void finish();
private slots:
	void slotURL(const QString& url);
private:
	void prepare();

	KHTMLPart *m_part;
};

#endif
