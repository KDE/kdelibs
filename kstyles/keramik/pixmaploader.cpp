/* 
   Copyright (c) 2002 Malte Starostik <malte@kde->org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version->
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE->  See the GNU
   General Public License for more details->
 
   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING->  If not, write to
   the Free Software Foundation, Inc->, 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA->
*/

// $Id$

#include <qapplication.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpixmapcache.h>

#include "pixmaploader.h"

#include "pixmaps.embed"

using namespace Keramik;

PixmapLoader PixmapLoader::s_instance;

PixmapLoader::PixmapLoader()
	:m_cache( 193 ) 
{ 
	QPixmapCache::setCacheLimit( 128 );
	m_cache.setAutoDelete( true );
}

void PixmapLoader::setColor( const QColor& color )
{
	if ( color == m_color ) return;
	m_color = color;
	m_cache.clear();
	QPixmapCache::clear();
}

void PixmapLoader::colorize( QImage &img )
{
	if ( img.isNull() || !m_color.isValid() ) return;
	int newh, news, newv;
	m_color.hsv( &newh, &news, &newv );

	for ( register int y = 0; y < img.height(); ++y )
	{
		Q_UINT32* data = reinterpret_cast< Q_UINT32* >( img.scanLine( y ) );
		for ( register int x = 0; x < img.width(); ++x )
		{
			QColor c( *data );
			int h, s, v;
			c.hsv( &h, &s, &v );
			c.setHsv( ( h - 216 + newh ) % 360, QMIN( s * news / 14, 255 ), QMIN( v * newv / 90, 255 ) );
			*data++ = ( c.rgb() & RGB_MASK ) | ( *data & ~RGB_MASK );
		}
	}
}

QPixmap PixmapLoader::pixmap( const QString& name )
{
	QPixmap result;
	if ( QPixmapCache::find( name, result ) )
		return result;

	QImage* img = m_cache[ name ];
	if ( !img ) {
		img = new QImage( qembed_findImage( name ).copy() );
		colorize( *img );
		m_cache.insert( name, img );
	}
	result.convertFromImage( *img );
	QPixmapCache::insert( name, result );
	return result;
}


QPixmap PixmapLoader::scale( const QString& name, int width, int height )
{
	QString key = name + '-' + QString::number( width ) + '-' + QString::number( height );
	QPixmap result;
	if ( QPixmapCache::find( key, result  ) )
		return result;

	QImage* img = m_cache[ name ];
	if ( !img ) {
		img = new QImage( qembed_findImage( name ) );
		m_cache.insert( name, img );
	}

	result.convertFromImage( img->scale( width ? width : img->width(), height ? height : img->height() ) );
	QPixmapCache::insert( key, result );
	return result;
}

void TilePainter::draw( QPainter *p, int x, int y, int width, int height )
{
	unsigned int scaledColumns = 0, scaledRows = 0, lastScaledColumn = 0, lastScaledRow = 0;
	int scaleWidth = width, scaleHeight = height;

	for ( unsigned int col = 0; col < columns(); ++col )
		if ( columnMode( col ) == Scaled )
		{
			scaledColumns++;
			lastScaledColumn = col;
		}
		else scaleWidth -= tile( col, 0 ).width();
	for ( unsigned int row = 0; row < rows(); ++row )
		if ( rowMode( row ) == Scaled )
		{
			scaledRows++;
			lastScaledRow = row;
		}
		else scaleHeight -= tile( 0, row ).height();
	if ( scaleWidth < 0 ) scaleWidth = 0;
	if ( scaleHeight < 0 ) scaleHeight = 0;

	int ypos = y;
	if ( scaleHeight && !scaledRows ) ypos += scaleHeight / 2;
	for ( unsigned int row = 0; row < rows(); ++row )
	{
		int xpos = x;
		if ( scaleWidth && !scaledColumns ) xpos += scaleWidth / 2;
		int h = rowMode( row ) == Fixed ? 0 : scaleHeight / scaledRows;
		if ( scaledRows && row == lastScaledRow ) h += scaleHeight - scaleHeight / scaledRows * scaledRows;

		for ( unsigned int col = 0; col < columns(); ++col )
		{
			int w = columnMode( col ) == Fixed ? 0 : scaleWidth / scaledColumns;
			if ( scaledColumns && col == lastScaledColumn ) w += scaleWidth - scaleWidth / scaledColumns * scaledColumns;

			if ( !tile( col, row ).isNull() )
				if ( w || h ) p->drawPixmap( xpos, ypos, scale( col, row, w, h ) );
				else p->drawPixmap( xpos, ypos, tile( col, row ) );
			xpos += w ? w : tile( col, row ).width();
		}
		ypos += h ? h : tile( 0, row ).height();
	}
}

QString TilePainter::absTileName( unsigned int column, unsigned int row ) const
{
	QString name = tileName( column, row );
	if ( name.isEmpty() ) return m_name;
	return m_name + "-" + name;
}

RectTilePainter::RectTilePainter( const QString& name, unsigned int columns, unsigned int rows )
	: TilePainter( name ),
	  m_columns( columns ),
	  m_rows( rows )
{
}

QString RectTilePainter::tileName( unsigned int column, unsigned int row ) const
{
	static QString c = "lcr", r = "tcb";
	return QString( r.mid( row, 1 ) + c.mid( column, 1 ) );
}

ActiveTabPainter::ActiveTabPainter( bool bottom )
	: RectTilePainter( QString( "tab-" ) + ( bottom ? "bottom-" : "top-" ) + "active" ),
	  m_bottom( bottom )
{
}

TilePainter::TileMode ActiveTabPainter::rowMode( unsigned int row ) const
{
	if ( m_bottom ) return ( row == 1 ) ? Fixed : Scaled;
	return ( row == 1 ) ? Scaled : Fixed;
}

QString ActiveTabPainter::tileName( unsigned int column, unsigned int row ) const
{
	if ( m_bottom )
		return RectTilePainter::tileName( column, row + 1 );
	return RectTilePainter::tileName( column, row );
}

InactiveTabPainter::InactiveTabPainter( Mode mode, bool bottom )
	: RectTilePainter( QString( "tab-" ) + ( bottom ? "bottom-" : "top-" ) + "inactive" ),
	  m_mode( mode ), m_bottom( bottom )
{
}

unsigned int InactiveTabPainter::columns() const
{
	Mode check = QApplication::reverseLayout() ? First : Last;
	return m_mode == check ? 3 : 2;
}

TilePainter::TileMode InactiveTabPainter::rowMode( unsigned int row ) const
{
	if ( m_bottom ) return ( row == 1 ) ? Fixed : Scaled;
	return ( row == 1 ) ? Scaled : Fixed;
}

QString InactiveTabPainter::tileName( unsigned int column, unsigned int row ) const
{
	Mode check = QApplication::reverseLayout() ? Last : First;
	if ( column == 0 && m_mode != check ) return "separator";
	if ( m_bottom )
		return RectTilePainter::tileName( column, row + 1 );
	return RectTilePainter::tileName( column, row );
}

ScrollBarPainter::ScrollBarPainter( const QString& type, int count, bool horizontal )
	: TilePainter( name( horizontal ) ),
	  m_type( type ),
	  m_count( count ),
	  m_horizontal( horizontal )
{
}

QString ScrollBarPainter::name( bool horizontal )
{
	return QString( "scrollbar-" ) + ( horizontal ? "hbar" : "vbar" );
}

QString ScrollBarPainter::tileName( unsigned int column, unsigned int row ) const
{
	unsigned int num = ( column ? column : row ) + 1;
	if ( m_count == 5 )
		if ( num == 3 ) num = 4;
		else if ( num == 4 ) num = 2;
		else if ( num == 5 ) num = 3;

	return m_type + QString::number( num );
}

TilePainter::TileMode SpinBoxPainter::columnMode( unsigned int column ) const
{
	return column == 1 ? Scaled : Fixed;
}

QString SpinBoxPainter::tileName( unsigned int column, unsigned int ) const
{
	return QString::number( column + 1 );
}

// vim: ts=4 sw=4 noet
