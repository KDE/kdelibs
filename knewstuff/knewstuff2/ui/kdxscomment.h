#ifndef KNEWSTUFF2_UI_KDXSCOMMENT_H
#define KNEWSTUFF2_UI_KDXSCOMMENT_H

#include <kdialog.h>

class QTextEdit;

class KDXSComment : public KDialog
{
Q_OBJECT
public:
	KDXSComment(QWidget *parent);
	QString comment();
private:
	QTextEdit *m_textbox;
};

#endif
