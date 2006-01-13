#ifndef KDXS_COMMENT_H
#define KDXS_COMMENT_H

#include <kdialogbase.h>

class QMultiLineEdit;

class KDXSComment : public KDialogBase
{
Q_OBJECT
public:
	KDXSComment(QWidget *parent);
	QString comment();
private:
	QMultiLineEdit *m_textbox;
};

#endif
