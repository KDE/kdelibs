#ifndef _KCOMBO_H_
#define _KCOMBO_H_

#include <qframe.h>
#include <qpopmenu.h>
#include <qlined.h>
#include <qlistbox.h>
#include <qstrlist.h>
#include <qcombo.h>	// some other parts of kdeui expect this to be included :(

#include <kdebug.h> // change to quotes? (#include "kdebug.h"?)
#include <klined.h> //   "    "   "

// Author:	Steve Dodd <dirk@loth.demon.co.uk>
// 111197:	full QComboBox interface, though some of it is not yet implemented.
//			completion / rotation support added, yees!
// 031297:	added completion() and rotation() signals as well
// 061297:	setCurrentItem() now doesn't emit signals
//			setInsertionPolicy() works, but we now have a bug which causes some
//			signals to be emitted twice - am working on it.
// 071297:	multiple signal emission problem solved; the lined also now displays
//			the first item in the listbox to start with.

class KCombo : public QFrame {
	Q_OBJECT

public:
	KCombo( QWidget* parent = NULL, const char* name = NULL, WFlags f = 0 );
	KCombo( bool readWrite, QWidget* parent = NULL, const char* name = NULL, WFlags f = 0 );
	virtual ~KCombo();

	// public functions special to KCombo

	void setLabelFlags( int textFlags ) { tf = textFlags; };
	int labelFlags() const { return tf; };
	void cursorAtEnd();

	// public functions like QCombo

	enum Policy { NoInsertion, AtTop, AtCurrent, AtBottom, AfterCurrent, BeforeCurrent };
	
	int count() const;
	void insertItem( const char* text, int index = -1 );
	void insertItem( const QPixmap& pixmap, int index = -1 );
	void insertStrList( const QStrList* list, int index = -1 );
	void insertStrList( const char** strings, int numStrings = -1, int index = -1 );
	void removeItem( int index );
	void clear();
	const char* text( int index ) const;
	const char* currentText() const;
	const QPixmap* pixmap( int index ) const;
	void changeItem( const char* text, int index );
	void changeItem( const QPixmap& pixmap, int index );
	void setCurrentItem( int index ) { select( index ); };
	int currentItem() const { return selected; };
	bool autoResize() const;
	void setAutoResize( bool enable );
	int sizeLimit() const { return sizeLimitLines; };
	void setSizeLimit( int lines );
	void setMaxCount( int max );
	int maxCount() const;
	void setInsertionPolicy( Policy policy );
	Policy insertionPolicy() const { return policy; };
	void setValidator( QValidator* v ) { if( lined ) lined->setValidator( v ); };
	QValidator* validator() const { if( lined ) return lined->validator(); return NULL; };

signals:
	void activated( int index );
	void activated( const char* text );
	void highlighted( int index );
	void highlighted( const char* text );
	void completion();
	void rotation();
	
public slots:
	void clearValidator() { setValidator( NULL ); };

protected slots:
	virtual void complete();
	virtual void rotate();
	void select( int item );
	void selectHide( int item );
	void selectQuiet( int item );
	void selectTyped();

protected:
	virtual void drawContents( QPainter* paint );
	virtual void resizeEvent( QResizeEvent* );
	virtual void mousePressEvent( QMouseEvent* ) { drop(); };
	virtual void keyPressEvent( QKeyEvent* e );
	virtual bool eventFilter( QObject* o, QEvent* e );
	void drop();

	QRect boxRect;		// rectangle of Motif 1.x selector box thingy
	QRect textRect;		// current item's text rectangle
	int	tf;				// text flags for current item
	QPopupMenu* popup;	// popup->menu for list of options
	int selected;		// currently selected item
	KLined* lined;		// editor for R/W Motif 2.x combos
	QListBox* listBox;	// list box for all Motif 2.x combos
	int sizeLimitLines;	// size of list box in lines
	Policy policy;		// insertion policy

};

#endif // _KCOMBO_H_
