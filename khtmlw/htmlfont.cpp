
#include "htmlfont.h"

// most of these sizes are standard X font sizes, so all of our fonts
// display nicely.
//
static int fontSizes[7] = { 8, 10, 12, 14, 18, 24, 32 };


HTMLFont::HTMLFont( const char *_family, int _size, int _weight, bool _italic )
	: font( _family, fontSizes[ _size ], _weight, _italic )
{
	textCol = black;
	linkCol = blue;
	vLinkCol = magenta;
	fsize = _size;
}

int HTMLFont::pointSize( int _size )
{
	return fontSizes[ _size ];
}

HTMLFontManager::HTMLFontManager()
{
	list.setAutoDelete( TRUE );
}

const HTMLFont *HTMLFontManager::getFont( const HTMLFont &f )
{
	HTMLFont *cf;

	for ( cf = list.first(); cf; cf = list.next() )
	{
		if ( *cf == f )
			return cf;
	}

	cf = new HTMLFont( f );

	list.append( cf );

	return cf;
}

