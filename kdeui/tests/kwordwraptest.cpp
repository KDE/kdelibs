#include <kapplication.h>
#include <kdebug.h>
#include <qwidget.h>
#include "kwordwrap.h"

int main(int argc, char *argv[])
{
	KApplication app(argc, argv, "KProgressTest");
	
	QFont font( "helvetica", 12 ); // let's hope we all have the same...
	QFontMetrics fm( font );
	QRect r( 0, 0, 100, -1 );
	QString str = "test wadabada [/foo/bar/waba]";
	KWordWrap* ww = KWordWrap::formatText( fm, r, 0, str );
	kdDebug() << str << " => " << ww->truncatedString() << endl;
}
