#ifndef _KCOMBO_H_
#define _KCOMBO_H_

#include <qcombo.h>

class KCombo : public QComboBox {
	Q_OBJECT

public:
	KCombo( QWidget* parent = 0, const char* name = 0, WFlags f = 0 );
	KCombo( bool readWrite, QWidget* parent = 0, 
		const char* name = 0, WFlags f = 0 );
	virtual ~KCombo(){};


	/* the following functions are there for compatibility with
	 * the former KCombo. Most of them are dummies now.
	 * Their functionality will soon be available in QComboBox
	 */

	void setLabelFlags( int) {};
	int labelFlags() const {return 0;};
	void cursorAtEnd(){};
	void setText( const char *text );
        void setCompletion( bool){};     
	
 private:
	bool set_text_called;
	
};

#endif // _KCOMBO_H_
