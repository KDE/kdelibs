
#include "htmlfont.h"

HTMLFont::HTMLFont( const char *family, int size, int weight, bool italic )
	: font( family, size, weight, italic )
{
	textCol = black;
	linkCol = blue;
	vLinkCol = magenta;
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

