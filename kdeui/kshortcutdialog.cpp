#include "kshortcutdialog.h"

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qsizepolicy.h>
#include <qtooltip.h>
#include <qvbox.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kglobalaccel.h>
#include <kiconloader.h>
#include <kkeynative.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>

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
	setFrameStyle( QFrame::Panel | QFrame::Plain );
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
	m_bGrabKeyboardOnFocusIn = true;
	m_bKeyboardGrabbed = false;
	m_iSeq = 0;
	m_iKey = 0;
	initGUI();

#ifdef Q_WS_X11
	kapp->installX11EventFilter( this );	// Allow button to capture X Key Events.
#endif
}

KShortcutDialog::~KShortcutDialog()
{
	if( m_bKeyboardGrabbed ) {
		kdDebug(125) << "KShortcutDialog::~KShortcutDialog(): m_bKeyboardGrabbed still true." << endl;
		releaseKeyboard();
	}
}

void KShortcutDialog::initGUI()
{
	setCaption( i18n("Define Shortcut") );

	QHBoxLayout* pHLayout = new QHBoxLayout( this, KDialog::marginHint() );
	QButtonGroup* pGroup = new QButtonGroup( this );
	pHLayout->addWidget( pGroup );

	m_prbSeq[0] = new QRadioButton( i18n("Primary"), pGroup );
	m_prbSeq[0]->setChecked( true );
	connect( m_prbSeq[0], SIGNAL(clicked()), this, SLOT(slotSeq0Selected()) );
	QPushButton* pb0 = new QPushButton( pGroup );
	pb0->setFlat( true );
	pb0->setPixmap( SmallIcon( "locationbar_erase" ) );
	QToolTip::add( pb0, i18n("Clear shortcut") );
	connect( pb0, SIGNAL(clicked()), this, SLOT(slotClearSeq0()) );
	m_peditSeq[0] = new KShortcutBox( m_cut.seq(0), pGroup );
	m_pcbMultiKey[0] = new QCheckBox( i18n("Multi-key"), pGroup );
	m_pcbMultiKey[0]->setChecked( m_cut.seq(0).count() > 1 );
	connect( m_pcbMultiKey[0], SIGNAL(clicked()), this, SLOT(slotSeq0Selected()) );

	m_prbSeq[1] = new QRadioButton( i18n("Alternate"), pGroup );
	connect( m_prbSeq[1], SIGNAL(clicked()), this, SLOT(slotSeq1Selected()) );
	QPushButton* pb1 = new QPushButton( pGroup );
	pb1->setFlat( true );
	pb1->setPixmap( SmallIcon( "locationbar_erase" ) );
	QToolTip::add( pb1, i18n("Clear shortcut") );
	connect( pb1, SIGNAL(clicked()), this, SLOT(slotClearSeq1()) );
	m_peditSeq[1] = new KShortcutBox( m_cut.seq(1), pGroup );
	m_pcbMultiKey[1] = new QCheckBox( i18n("Multi-key"), pGroup );
	m_pcbMultiKey[1]->setChecked( m_cut.seq(1).count() > 1 );
	connect( m_pcbMultiKey[1], SIGNAL(clicked()), this, SLOT(slotSeq1Selected()) );

	QGridLayout* pLayout = new QGridLayout( pGroup, 2, 3, KDialog::marginHint(), KDialog::spacingHint() );
	pLayout->setColStretch( 2, 1 );
	pLayout->addWidget( m_prbSeq[0], 0, 0 );
	pLayout->addWidget( pb0, 0, 1 );
	pLayout->addWidget( m_peditSeq[0], 0, 2 );
	pLayout->addWidget( m_pcbMultiKey[0], 0, 3 );
	pLayout->addWidget( m_prbSeq[1], 1, 0 );
	pLayout->addWidget( pb1, 1, 1 );
	pLayout->addWidget( m_peditSeq[1], 1, 2 );
	pLayout->addWidget( m_pcbMultiKey[1], 1, 3 );

	QVBox* pVBox = new QVBox( this );
        
	// Don't use KStdGuiItem because shown accels would be assigned as shortcut when pressed
	KGuiItem ok = KStdGuiItem::ok();
	ok.setText( i18n( "OK" ) );
	KGuiItem cancel = KStdGuiItem::cancel();
	cancel.setText( i18n( "Cancel" ) );
	m_pcmdOK = new KPushButton( ok, pVBox );
	m_pcmdCancel = new KPushButton( cancel, pVBox );
	m_pcbAutoClose = new QCheckBox( i18n("Auto-close"), pVBox );
	m_pcbAutoClose->setChecked( true );
	// Disable auto-close if the sequence we're editing is a multi-key shortcut.
	m_pcbAutoClose->setEnabled( !m_pcbMultiKey[0]->isChecked() );

	connect( m_pcmdOK, SIGNAL(clicked()), this, SLOT(accept()) );
	connect( m_pcmdCancel, SIGNAL(clicked()), this, SLOT(reject()) );

	pHLayout->addWidget( pVBox );
	m_prbSeq[0]->clearFocus();
}

void KShortcutDialog::selectSeq( uint i )
{
	kdDebug(125) << "KShortcutDialog::selectSeq( " << i << " )" << endl;
	m_iSeq = i;
	m_prbSeq[m_iSeq]->setChecked( true );
	// Start editing at the first key in the sequence.
	m_iKey = 0;
	// Can't auto-close if editing a multi-key shortcut.
	m_pcbAutoClose->setEnabled( !m_pcbMultiKey[m_iSeq]->isChecked() );
	m_prbSeq[m_iSeq]->setFocus();
}

void KShortcutDialog::clearSeq( uint i )
{
	kdDebug(125) << "KShortcutDialog::deleteSeq( " << i << " )" << endl;
	m_peditSeq[i]->setSeq( KKeySequence::null() );
	m_cut.setSeq( i, KKeySequence::null() );
	selectSeq( i );

	// If we're clearing the alternate, then we need to set m_cut.count()
	//  back to 1 if there is a primary sequence.
	if( i == 1 && m_cut.count() > 0 )
		m_cut = m_cut.seq(0);
}

void KShortcutDialog::slotSeq0Selected() { selectSeq( 0 ); }
void KShortcutDialog::slotSeq1Selected() { selectSeq( 1 ); }
void KShortcutDialog::slotClearSeq0()    { clearSeq( 0 ); }
void KShortcutDialog::slotClearSeq1()    { clearSeq( 1 ); }

void KShortcutDialog::accept()
{
	kdDebug(125) << "KShortcutDialog::accept()" << endl;
	m_bGrabKeyboardOnFocusIn = false;
	m_bKeyboardGrabbed = false;
	releaseKeyboard();
	KDialog::accept();
}

#ifdef Q_WS_X11
bool KShortcutDialog::x11Event( XEvent *pEvent )
{
	switch( pEvent->type ) {
		case XKeyPress:
		case XKeyRelease:
			if( m_bKeyboardGrabbed ) {
				x11EventKeyPress( pEvent );
				return true;
			}
			break;
		case ButtonPress:
			m_iKey = 0;
			break;
		case XFocusIn:
			kdDebug(125) << "FocusIn" << endl;
			if( m_bGrabKeyboardOnFocusIn && !m_bKeyboardGrabbed ) {
				kdDebug(125) << "\tkeyboard grabbed." << endl;
				m_bKeyboardGrabbed = true;
				grabKeyboard();
			}
			break;
		case XFocusOut:
			kdDebug(125) << "FocusOut" << endl;
			if( m_bKeyboardGrabbed ) {
				kdDebug(125) << "\tkeyboard released." << endl;
				m_bKeyboardGrabbed = false;
				releaseKeyboard();
			}
			break;
		default:
			//kdDebug(125) << "x11Event->type = " << pEvent->type << endl;
			break;
	}
	return QWidget::x11Event( pEvent );
}

void KShortcutDialog::x11EventKeyPress( XEvent *pEvent )
{
	KKeyNative keyNative( pEvent );
	uint keyModX = keyNative.mod(), keySymX = keyNative.sym();

	//kdDebug(125) << QString( "keycode: 0x%1 state: 0x%2\n" )
	//			.arg( pEvent->xkey.keycode, 0, 16 ).arg( pEvent->xkey.state, 0, 16 );

	switch( keySymX ) {
		// Don't allow setting a modifier key as an accelerator.
		// Also, don't release the focus yet.  We'll wait until
		//  we get a 'normal' key.
		case XK_Shift_L:   case XK_Shift_R:   keyModX = KKeyNative::modX(KKey::SHIFT); break;
		case XK_Control_L: case XK_Control_R: keyModX = KKeyNative::modX(KKey::CTRL); break;
		case XK_Alt_L:     case XK_Alt_R:     keyModX = KKeyNative::modX(KKey::ALT); break;
		// FIXME: check whether the Meta or Super key are for the Win modifier
		case XK_Meta_L:    case XK_Meta_R:
		case XK_Super_L:   case XK_Super_R:   keyModX = KKeyNative::modX(KKey::WIN); break;
		case XK_Hyper_L:   case XK_Hyper_R:
		case XK_Mode_switch:
			break;
		default:
			if( pEvent->type == XKeyPress && keyNative.sym() ) {
				// If RETURN was pressed and we are recording a
				//  multi-key shortcut, then we are done.
				if( keyNative.sym() == XK_Return && m_iKey > 0 ) {
					accept();
					return;
				}

				KKey key = keyNative;
				key.simplify();
				KKeySequence seq;
				if( m_iKey == 0 )
					seq = key;
				else {
					seq = m_cut.seq( m_iSeq );
					seq.setKey( m_iKey, key );
				}
				m_cut.setSeq( m_iSeq, seq );

				if( m_pcbMultiKey[m_iSeq]->isChecked() )
					m_iKey++;

				m_peditSeq[m_iSeq]->setSeq( m_cut.seq(m_iSeq) );
				//captureShortcut( false );
				// The parent must decide whether this is a valid
				//  key, and if so, call setShortcut(uint) with the new value.
				//emit capturedShortcut( KShortcut(KKey(keyNative)) );
				kdDebug(125) << "m_cut = " << m_cut.toString() << endl;
				if( m_pcbAutoClose->isEnabled() && m_pcbAutoClose->isChecked() )
					accept();
			}
			return;
	}

	// If we are editing the first key in the sequence,
	//  display modifier keys which are held down
	if( m_iKey == 0 ) {
		if( pEvent->type == XKeyPress )
			keyModX |= pEvent->xkey.state;
		else
			keyModX = pEvent->xkey.state & ~keyModX;

		QString keyModStr;
		if( keyModX & KKeyNative::modX(KKey::WIN) )	keyModStr += KKey::modFlagLabel(KKey::WIN) + "+";
		if( keyModX & KKeyNative::modX(KKey::ALT) )	keyModStr += KKey::modFlagLabel(KKey::ALT) + "+";
		if( keyModX & KKeyNative::modX(KKey::CTRL) )	keyModStr += KKey::modFlagLabel(KKey::CTRL) + "+";
		if( keyModX & KKeyNative::modX(KKey::SHIFT) )	keyModStr += KKey::modFlagLabel(KKey::SHIFT) + "+";

		// Display currently selected modifiers, or redisplay old key.
		if( !keyModStr.isEmpty() )
			m_peditSeq[m_iSeq]->setText( keyModStr );
		else
			m_peditSeq[m_iSeq]->setSeq( m_cut.seq(m_iSeq) );
	}
}
#endif // QT_WS_X11

void KShortcutDialog::virtual_hook( int id, void* data )
{ KDialog::virtual_hook( id, data ); }


#include "kshortcutdialog.moc"
