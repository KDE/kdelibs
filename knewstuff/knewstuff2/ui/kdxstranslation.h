#ifndef KNEWSTUFF2_UI_KDXSTRANSLATION_H
#define KNEWSTUFF2_UI_KDXSTRANSLATION_H

#include <kdialog.h>

class QLineEdit;
class QTextEdit;
class KUrlRequester;

class KDXSTranslation : public KDialog
{
Q_OBJECT
public:
	KDXSTranslation(QWidget *parent);
private:
	QLineEdit *m_name;
	QTextEdit *m_description;
	KUrlRequester *m_payload;
};

#endif

