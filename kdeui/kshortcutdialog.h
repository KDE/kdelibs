#ifndef _KSHORTCUTDIALOG_H
#define _KSHORTCUTDIALOG_H

#include <qlabel.h>
#include <kdialog.h>
#include <kshortcut.h>

class QCheckBox;
class QRadioButton;

class KShortcutBox : public QLabel
{
	Q_OBJECT
 public:
	KShortcutBox( const KKeySequence& seq, QWidget* parent = 0, const char* name = 0 );

	const KKeySequence& seq() const { return m_seq; }
	void setSeq( const KKeySequence& );

 protected:
	KKeySequence m_seq;
};

class KShortcutDialog : public KDialog
{
	Q_OBJECT
 public:
	KShortcutDialog( const KShortcut& cut, QWidget* parent = 0, const char* name = 0 );
	
	const KShortcut& cut() const { return m_cut; }

 protected:
	KShortcut m_cut;
	QRadioButton* m_prbPrimary;
	KShortcutBox* m_peditPrimary;
	QCheckBox*    m_pcbPrimary;
	QRadioButton* m_prbAlternate;
	KShortcutBox* m_peditAlternate;
	QCheckBox*    m_pcbAlternate;
	QPushButton*  m_pcmdOK;
	QPushButton*  m_pcmdCancel;
	QCheckBox*    m_pcbAutoClose;
	uint          m_iKey;

 private:
	void initGUI();
#ifdef Q_WS_X11
	virtual bool x11Event( XEvent *pEvent );
	void x11EventKeyPress( XEvent *pEvent );
#endif

	class KShortcutDialogPrivate* d;
};

#endif // !_KSHORTCUTDIALOG_H
