#include <font.h>
#include <qpainter.h>
#include <qfontdatabase.h>
#include <qpaintdevicemetrics.h>

using namespace khtml;

void Font::drawText( QPainter *p, int x, int y, QChar *str, int len, int width, QPainter::TextDirection d )
{
    // ### fixme for RTL
    if ( !letterSpacing && !wordSpacing || d == QPainter::RTL ) {
	// simply draw it
	p->drawText( x, y, QConstString(str, len).string(), len, d );
    } else {
	QConstString cstr( str, len );
	QString s( cstr.string() );
	for( int i = 0; i < len; i++ ) {
	    p->drawText( x, y, s, i, 1, d );
	    x += fm.charWidth( s, i );
	    if ( letterSpacing )
		x += letterSpacing;
	    if ( str[i].isSpace() && wordSpacing )
		s += wordSpacing;
	}
    }
}


int Font::width( QChar *chs, int len )
{
    int w = fm.width( QConstString( chs, len).string(), len );
    
    if ( letterSpacing )
	w += len*letterSpacing;
    
    if ( wordSpacing ) {
	// add amount
	for( int i = 0; i < len; i++ ) {
	    if( chs[i].isSpace() )
		w += wordSpacing;
	}
    }	
    return w;
}


void Font::update( QPaintDeviceMetrics *devMetrics ) const
{
    f.setFamily( fontDef.family );
    f.setItalic( fontDef.italic );
    f.setWeight( fontDef.weight );
    
    QFontDatabase db;

    float size = fontDef.size;

    float toPix = devMetrics->logicalDpiY()/72.;

    // ok, now some magic to get a nice unscaled font
    // all other font properties should be set before this one!!!!

    if( !db.isSmoothlyScalable(f.family(), db.styleString(f)) )
    {
        QValueList<int> pointSizes = db.smoothSizes(f.family(), db.styleString(f));
        // lets see if we find a nice looking font, which is not too far away
        // from the requested one.
        //kdDebug(6080) << "khtml::setFontSize family = " << f.family() << " size requested=" << size << endl;

        QValueList<int>::Iterator it;
        float diff = 1; // ### 100% deviation
        float bestSize = 0;
        for( it = pointSizes.begin(); it != pointSizes.end(); ++it )
        {
            float newDiff = ((*it)*toPix - size)/size;
            //kdDebug( 6080 ) << "smooth font size: " << *it << " diff=" << newDiff << endl;
            if(newDiff < 0) newDiff = -newDiff;
            if(newDiff < diff)
            {
                diff = newDiff;
                bestSize = *it;
            }
        }
        //kdDebug( 6080 ) << "best smooth font size: " << bestSize << " diff=" << diff << endl;
        if ( bestSize != 0 && diff < 0.2 ) // 20% deviation, otherwise we use a scaled font...
            size = bestSize*toPix;
//         else if ( size > 4 && size < 16 )
//             size = float( int( ( size + 1 ) / 2 )*2 );
    } else {
	size = size*toPix;
    }

    //qDebug(" -->>> using %f pixel font", size);
    
//     qDebug("setting font to %s, italic=%d, weight=%d, size=%f", fontDef.family.latin1(), fontDef.italic,
// 	   fontDef.weight, size );

    f.setPixelSizeFloat( size );

    fm = QFontMetrics( f );
}
