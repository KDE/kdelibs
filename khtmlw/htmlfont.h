
#include <qlist.h>
#include <qcolor.h>
#include <qfont.h>

#define MAXFONTSIZES 7

class HTMLFont
{
public:
	HTMLFont( const char *_family, int _size, int _weight=QFont::Normal, bool _italic=FALSE );
	HTMLFont( const HTMLFont &f );

	void setWeight( bool w )
		{	font.setWeight( w ); }
	void setItalic( bool u )
		{	font.setItalic( u ); }
	void setUnderline( bool u )
		{	font.setUnderline( u ); }
	void setStrikeOut( bool s )
		{	font.setStrikeOut( s ); }

	void setTextColor( const QColor &col )
		{	textCol = col; }
	void setLinkColor( const QColor &col )
		{	linkCol = col; }
	void setVLinkColor( const QColor &col )
		{	vLinkCol = col; }

	const char *family() const
		{	return font.family(); }
	const bool weight() const
		{	return font.weight(); }
	const bool italic() const
		{	return font.italic(); }
	const bool underline() const
		{	return font.underline(); }
	const bool strikeOut() const
		{	return font.strikeOut(); }
	const int  pointSize() const
		{	return font.pointSize(); }

	const QColor &textColor() const
		{	return textCol; }
	const QColor &linkColor() const
		{	return linkCol; }
	const QColor &vLinkColor() const
		{	return vLinkCol; }
	int size() const
		{	return fsize; }

	const HTMLFont &operator=( const HTMLFont &f );
	bool operator==( const HTMLFont &f );
	operator QFont() const
		{	return font; }

	static int pointSize( int _size );

private:
	QFont  font;
	QColor textCol;
	QColor linkCol;
	QColor vLinkCol;
	int    fsize;
};

inline HTMLFont::HTMLFont( const HTMLFont &f ) : font( f.font )
{
	textCol = f.textCol;
	linkCol = f.linkCol;
	vLinkCol = f.vLinkCol;
	fsize = f.fsize;
}

inline const HTMLFont &HTMLFont::operator=( const HTMLFont &f )
{
	font = f.font;
	textCol = f.textCol;
	linkCol = f.linkCol;
	vLinkCol = f.vLinkCol;
	fsize = f.fsize;

	return *this;
}

inline bool HTMLFont::operator==( const HTMLFont &f )
{
	return ( font == f.font && textCol == f.textCol && linkCol == f.linkCol &&
			vLinkCol == f.vLinkCol && fsize == f.fsize );
}

//-----------------------------------------------------------------------------

class HTMLFontManager
{
public:
	HTMLFontManager();

	const HTMLFont *getFont( const HTMLFont &f );

private:
	QList<HTMLFont> list;
};



