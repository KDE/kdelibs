#ifndef KHTMLFONT_H
#define KHTMLFONT_H

#include <qfont.h>
#include <qfontmetrics.h>
#include <qpainter.h>

class QPaintDeviceMetrics;

namespace khtml
{
  
    class FontDef 
    {
    public:
	FontDef() 
	    : size( 0 ), italic( false ), smallCaps( false ), weight( 50 ) {}
	bool operator == ( const FontDef &other ) const {
	    return ( family == other.family &&
		     size == other.size &&
		     italic == other.italic &&
		     smallCaps == other.smallCaps &&
		     weight == other.weight );
	}
	
	QString family;
	float size;
	bool italic 		: 1;
	bool smallCaps 		: 1;
	unsigned int weight 		: 8;
    };
    

    class Font
    {
    public:
	Font() : fontDef(), f(), fm( f ), scFont( 0 ), letterSpacing( 0 ), wordSpacing( 0 ) {}
	Font( const FontDef &fd )
	    :  fontDef( fd ), f(), fm( f ), scFont( 0 ), letterSpacing( 0 ), wordSpacing( 0 )
	    {
	    }
	
	bool operator == ( const Font &other ) const {
	    return (fontDef == other.fontDef &&
		    letterSpacing == other.letterSpacing &&
		    wordSpacing == other.wordSpacing ); 
	}

	void update( QPaintDeviceMetrics *devMetrics ) const;
	
	void drawText( QPainter *p, int x, int y, QChar *str, int len, int width, 
                QPainter::TextDirection d, int from=-1, int to=-1, QColor bg=QColor() ) const;
	
	int width( QChar *str, int len ) const;
	int width( QChar ch ) const;
	
	FontDef fontDef;
	mutable QFont f;
	mutable QFontMetrics fm;
	QFont *scFont;
	short letterSpacing;
	short wordSpacing;
    };
    
};


#endif
