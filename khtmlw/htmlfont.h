
#include <qlist.h>
#include <qcolor.h>
#include <qfont.h>

class HTMLFont
{
public:
	HTMLFont( const char *family, int size, int weight=QFont::Normal, bool italic=FALSE );
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

	const HTMLFont &operator=( const HTMLFont &f );
	bool operator==( const HTMLFont &f );
	operator QFont() const
		{	return font; }

private:
	QFont font;
	QColor textCol;
	QColor linkCol;
	QColor vLinkCol;
};

inline HTMLFont::HTMLFont( const HTMLFont &f ) : font( f.font )
{
	textCol = f.textCol;
	linkCol = f.linkCol;
	vLinkCol = f.vLinkCol;
}

inline const HTMLFont &HTMLFont::operator=( const HTMLFont &f )
{
	font = f.font;
	textCol = f.textCol;
	linkCol = f.linkCol;
	vLinkCol = f.vLinkCol;

	return *this;
}

inline bool HTMLFont::operator==( const HTMLFont &f )
{
	return ( font == f.font && textCol == f.textCol && linkCol == f.linkCol &&
			vLinkCol == f.vLinkCol );
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



