#ifndef KDXS_TRANSLATION_H
#define KDXS_TRANSLATION_H

#include <kdialogbase.h>

class QLineEdit;
class QTextEdit;
class KURLRequester;

class KDXSTranslation : public KDialogBase
{
Q_OBJECT
public:
	KDXSTranslation(QWidget *parent);
private:
	QLineEdit *m_name;
	QTextEdit *m_description;
	KURLRequester *m_payload;
};

#endif

