#include "kcoloractions.h"
#include <qcstring.h>
#include "kcoloractions.moc"
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>

KColorAction::KColorAction( const QString& text, int accel,
			    QObject* parent, const char* name )
    : QAction( text, accel, parent, name )
{
    init();
}

KColorAction::KColorAction( const QString& text, int accel,
			    QObject* receiver, const char* slot, QObject* parent, 
			    const char* name )
    : QAction( text, accel, receiver, slot, parent, name )
{
    init();
}

KColorAction::KColorAction( const QString& text, Type type, int accel,
			    QObject* parent, const char* name )
    : QAction( text, accel, parent, name )
{
    init();
    setType( type );
}

KColorAction::KColorAction( const QString& text, Type type, int accel,
			    QObject* receiver, const char* slot, QObject* parent, 
			    const char* name )
    : QAction( text, accel, receiver, slot, parent, name )
{
    init();
    setType( type );
}

KColorAction::KColorAction( QObject* parent, const char* name )
    : QAction( parent, name )
{
    init();
}

void KColorAction::setColor( const QColor &c )
{
    if ( c == col )
	return;
    
    col = c;
    createPixmap();
}

QColor KColorAction::color() const
{
    return col;
}
    
void KColorAction::setType( Type t )
{
    if ( t == typ )
	return;
    
    typ = t;
    createPixmap();
}

KColorAction::Type KColorAction::type() const
{
    return typ;
}
    
void KColorAction::init()
{
    col = Qt::black;
    typ = TextColor;
    createPixmap();
}

void KColorAction::createPixmap()
{
    int r, g, b;
    QCString pix;
    QCString line;

    col.rgb( &r, &g, &b );

    pix = "/* XPM */\n";

    pix += "static char * text_xpm[] = {\n";

    switch ( typ ) {
      case TextColor: {
        pix += "\"20 20 11 1\",\n";
        pix += "\"h c #c0c000\",\n";
        pix += "\"g c #808000\",\n";
        pix += "\"f c #c0c0ff\",\n";
        pix += "\"a c #000000\",\n";
        pix += "\"d c #ff8000\",\n";
        pix += "\". c none\",\n";
        pix += "\"e c #0000c0\",\n";
        pix += "\"i c #ffff00\",\n";
        line.sprintf( "\"# c #%02X%02X%02X \",\n", r, g, b );
        pix += line.copy();
        pix += "\"b c #c00000\",\n";
        pix += "\"c c #ff0000\",\n";
        pix += "\"....................\",\n";
        pix += "\"....................\",\n";
        pix += "\"....................\",\n";
        pix += "\"........#...........\",\n";
        pix += "\"........#a..........\",\n";
        pix += "\".......###..........\",\n";
        pix += "\".......###a.........\",\n";
        pix += "\"......##aa#.........\",\n";
        pix += "\"......##a.#a........\",\n";
        pix += "\".....##a...#........\",\n";
        pix += "\".....#######a.......\",\n";
        pix += "\"....##aaaaaa#.......\",\n";
        pix += "\"....##a.....aaaaaaaa\",\n";
        pix += "\"...####....#abbccdda\",\n";
        pix += "\"....aaaa....abbccdda\",\n";
        pix += "\"............aee##ffa\",\n";
        pix += "\"............aee##ffa\",\n";
        pix += "\"............agghhiia\",\n";
        pix += "\"............agghhiia\",\n";
        pix += "\"............aaaaaaaa\"};\n";
      } break;
      case FrameColor: {
	pix += "\" 20 20 3 1 \",\n";

	pix += "\"  c none \",\n";
	pix += "\"+ c white \",\n";
	line.sprintf( "\". c #%02X%02X%02X \",\n", r, g, b );
	pix += line.copy();

	pix += "\"                     \",\n";
	pix += "\"                     \",\n";
	pix += "\"  ................  \",\n";
	pix += "\"  ................  \",\n";
	pix += "\"  ................  \",\n";
	pix += "\"  ...++++++++++...  \",\n";
	pix += "\"  ...++++++++++...  \",\n";
	pix += "\"  ...++++++++++...  \",\n";
	pix += "\"  ...++++++++++...  \",\n";
	pix += "\"  ...++++++++++...  \",\n";
	pix += "\"  ...++++++++++...  \",\n";
	pix += "\"  ...++++++++++...  \",\n";
	pix += "\"  ...++++++++++...  \",\n";
	pix += "\"  ...++++++++++...  \",\n";
	pix += "\"  ...++++++++++...  \",\n";
	pix += "\"  ................  \",\n";
	pix += "\"  ................  \",\n";
	pix += "\"  ................  \",\n";
	pix += "\"                     \",\n";
	pix += "\"                     \";\n";
      } break;
      case BackgroundColor: {
        pix += "\" 20 20 3 1 \",\n";

        pix += "\"  c none \",\n";
        pix += "\". c red \",\n";
        line.sprintf( "\"+ c #%02X%02X%02X \",\n", r, g, b );
        pix += line.copy();

        pix += "\"                     \",\n";
        pix += "\"                     \",\n";
        pix += "\"  ................  \",\n";
        pix += "\"  ................  \",\n";
        pix += "\"  ..++++++++++++..  \",\n";
        pix += "\"  ..++++++++++++..  \",\n";
        pix += "\"  ..++++++++++++..  \",\n";
        pix += "\"  ..++++++++++++..  \",\n";
        pix += "\"  ..++++++++++++..  \",\n";
        pix += "\"  ..++++++++++++..  \",\n";
        pix += "\"  ..++++++++++++..  \",\n";
        pix += "\"  ..++++++++++++..  \",\n";
        pix += "\"  ..++++++++++++..  \",\n";
        pix += "\"  ..++++++++++++..  \",\n";
        pix += "\"  ..++++++++++++..  \",\n";
        pix += "\"  ..++++++++++++..  \",\n";
        pix += "\"  ................  \",\n";
        pix += "\"  ................  \",\n";
        pix += "\"                     \",\n";
        pix += "\"                     \";\n";
      } break;
    }

    QPixmap pixmap( pix );
    setIconSet( QIconSet( pixmap ) );
}
