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
	KShortcutDialog( const KShortcut& cut, bool bQtShortcut, QWidget* parent = 0, const char* name = 0 );
	~KShortcutDialog();

	const KShortcut& cut() const { return m_cut; }

 protected:
	bool          m_bQtShortcut;      // true if qt shortcut, false if native shortcut
	bool          m_bGrabKeyboardOnFocusIn;
	bool          m_bKeyboardGrabbed;
	KShortcut     m_cut;
	QRadioButton* m_prbSeq[2];
	KShortcutBox* m_peditSeq[2];
	QCheckBox*    m_pcbMultiKey[2];
	QPushButton*  m_pcmdOK;
	QPushButton*  m_pcmdCancel;
	QCheckBox*    m_pcbAutoClose;
	uint          m_iSeq;             // index of sequence being edited.
	uint          m_iKey;             // index of key being edited.

	void selectSeq( uint );
	void clearSeq( uint );

 protected slots:
	void slotSeq0Selected();
	void slotSeq1Selected();
	void slotClearSeq0();
	void slotClearSeq1();

	virtual void accept();	// override parent's accept()

 private:
	void initGUI();
#ifdef Q_WS_X11
	virtual bool x11Event( XEvent *pEvent );
	void x11EventKeyPress( XEvent *pEvent );
#endif

 protected:
	virtual void virtual_hook( int id, void* data );
 private:
	class KShortcutDialogPrivate* d;
};

#endif // !_KSHORTCUTDIALOG_H
