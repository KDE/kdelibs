#include "kshortcutdialog.h"

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qsizepolicy.h>
#include <qvbox.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>

#ifdef Q_WS_X11
#define XK_XKB_KEYS
#define XK_MISCELLANY
#include <X11/Xlib.h>	// For x11Event()
#include <X11/keysymdef.h> // For XK_...

#ifdef KeyPress
const int XFocusOut = FocusOut;
const int XFocusIn = FocusIn;
const int XKeyPress = KeyPress;
const int XKeyRelease = KeyRelease;
#undef KeyRelease
#undef KeyPress
#undef FocusOut
#undef FocusIn
#endif
#endif

KShortcutBox::KShortcutBox( const KKeySequence& seq, QWidget* parent, const char* name )
:	QLabel( parent, name )
{
	setSeq( seq );
}

void KShortcutBox::setSeq( const KKeySequence& seq )
{
	m_seq = seq;
	if( !m_seq.isNull() )
		setText( seq.toString() );
	else
		setText( i18n("None") );
}

KShortcutDialog::KShortcutDialog( const KShortcut& cut, QWidget* parent, const char* name )
:	KDialog( parent, name ),
	m_cut( cut )
{
	m_iKey = 0;
	initGUI();

#ifdef Q_WS_X11
	kapp->installX11EventFilter( this );	// Allow button to capture X Key Events.
#endif
}

void KShortcutDialog::initGUI()
{
	setCaption( i18n("Define Shortcut") );

	QHBoxLayout* pHLayout = new QHBoxLayout( this, KDialog::marginHint() );
	QButtonGroup* pGroup = new QButtonGroup( this );
	pHLayout->addWidget( pGroup );

	m_prbPrimary = new QRadioButton( i18n("Primary"), pGroup );
	m_prbPrimary->setChecked( true );
	m_peditPrimary = new KShortcutBox( m_cut.seq(0), pGroup );
	m_peditPrimary->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
	m_pcbPrimary = new QCheckBox( i18n("Multi-Key"), pGroup );

	m_prbAlternate = new QRadioButton( i18n("Alternate"), pGroup );
	m_peditAlternate = new KShortcutBox( m_cut.seq(1), pGroup );
	m_peditAlternate->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
	m_pcbAlternate = new QCheckBox( i18n("Multi-Key"), pGroup );

	QGridLayout* pLayout = new QGridLayout( pGroup, 2, 3, KDialog::marginHint(), KDialog::spacingHint() );
	pLayout->setColStretch( 1, 1 );
	pLayout->addWidget( m_prbPrimary, 0, 0 );
	pLayout->addWidget( m_peditPrimary, 0, 1 );
	pLayout->addWidget( m_pcbPrimary, 0, 2 );
	pLayout->addWidget( m_prbAlternate, 1, 0 );
	pLayout->addWidget( m_peditAlternate, 1, 1 );
	pLayout->addWidget( m_pcbAlternate, 1, 2 );

	QVBox* pVBox = new QVBox( this );
	m_pcmdOK = new QPushButton( i18n("OK"), pVBox );
	m_pcmdCancel = new QPushButton( i18n("Cancel"), pVBox );
	m_pcbAutoClose = new QCheckBox( i18n("Auto-Close"), pVBox );
	m_pcbAutoClose->setChecked( true );

	connect( m_pcmdOK, SIGNAL(clicked()), this, SLOT(accept()) );
	connect( m_pcmdCancel, SIGNAL(clicked()), this, SLOT(reject()) );

	pHLayout->addWidget( pVBox );
}

#ifdef Q_WS_X11
bool KShortcutDialog::x11Event( XEvent *pEvent )
{
	//if( m_bEditing ) {
		//kdDebug(125) << "x11Event: type: " << pEvent->type << " window: " << pEvent->xany.window << endl;
		switch( pEvent->type ) {
			case XKeyPress:
			case XKeyRelease:
				x11EventKeyPress( pEvent );
				return true;
			case ButtonPress:
				m_iKey = 0;
				//captureShortcut( false );
				//setShortcut( m_cut );
				//return true;
		}
	//}
	return QWidget::x11Event( pEvent );
}

void KShortcutDialog::x11EventKeyPress( XEvent *pEvent )
{
	KKeyNative keyNative( pEvent );
	uint keyModX = keyNative.mod(), keySymX = keyNative.sym();

	//kdDebug(125) << QString( "keycode: 0x%1 state: 0x%2\n" )
	//			.arg( pEvent->xkey.keycode, 0, 16 ).arg( pEvent->xkey.state, 0, 16 );

	uint iSeq = m_prbPrimary->isChecked() ? 0 : 1;
	KShortcutBox* pBox = m_prbPrimary->isChecked() ? m_peditPrimary : m_peditAlternate;

	switch( keySymX ) {
		// Don't allow setting a modifier key as an accelerator.
		// Also, don't release the focus yet.  We'll wait until
		//  we get a 'normal' key.
		case XK_Shift_L:   case XK_Shift_R:   keyModX = KKeyNative::modX(KKey::SHIFT); break;
		case XK_Control_L: case XK_Control_R: keyModX = KKeyNative::modX(KKey::CTRL); break;
		case XK_Alt_L:     case XK_Alt_R:     keyModX = KKeyNative::modX(KKey::ALT); break;
		case XK_Meta_L:    case XK_Meta_R:    keyModX = KKeyNative::modX(KKey::WIN); break;
		case XK_Super_L:   case XK_Super_R:
		case XK_Hyper_L:   case XK_Hyper_R:
		case XK_Mode_switch:
			break;
		default:
			if( pEvent->type == XKeyPress && keyNative.sym() ) {
				kdDebug() << "m_iKey = " << m_iKey << endl;
				KKey key = keyNative;
				if( iSeq < m_cut.count() ) {
					if( m_iKey == 0 )
						m_cut.setSeq( iSeq, key );
					else
						m_cut.seq(iSeq).setKey( m_iKey, key );
				} else if( iSeq == m_cut.count() )
					m_cut.insert( key );
				else
					return;

				if( (iSeq == 0 && m_pcbPrimary->isChecked())
				    || (iSeq == 1 && m_pcbAlternate->isChecked()) )
					m_iKey++;

				pBox->setSeq( m_cut.seq(iSeq) );
				//captureShortcut( false );
				// The parent must decide whether this is a valid
				//  key, and if so, call setShortcut(uint) with the new value.
				//emit capturedShortcut( KShortcut(KKey(keyNative)) );
				kdDebug() << "m_cut = " << m_cut.toString() << endl;
				if( m_pcbAutoClose->isChecked() )
					accept();
			}
			return;
	}

	if( pEvent->type == XKeyPress )
		keyModX |= pEvent->xkey.state;
	else
		keyModX = pEvent->xkey.state & ~keyModX;

	QString keyModStr;
	if( keyModX & KKeyNative::modX(KKey::WIN) )	keyModStr += i18n("Win") + "+";
	if( keyModX & KKeyNative::modX(KKey::ALT) )	keyModStr += i18n("Alt") + "+";
	if( keyModX & KKeyNative::modX(KKey::CTRL) )	keyModStr += i18n("Ctrl") + "+";
	if( keyModX & KKeyNative::modX(KKey::SHIFT) )	keyModStr += i18n("Shift") + "+";

	// Display currently selected modifiers, or redisplay old key.
	if( !keyModStr.isEmpty() )
		pBox->setText( keyModStr );
	else
		pBox->setSeq( m_cut.seq(iSeq) );
}
#endif // QT_WS_X11

#include "kshortcutdialog.moc"
