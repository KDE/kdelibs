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
#include <qbitmap.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpixmapcache.h>

#include "pixmaploader.h"


#include "pixmaps.keramik"

using namespace Keramik;

PixmapLoader* PixmapLoader::s_instance = 0;

PixmapLoader::PixmapLoader():   m_imageCache(131072,  1013),
											 	 m_pixmapCache(2*131072, 1013)
													
{ 
//	QPixmapCache::setCacheLimit( 128 );
	m_pixmapCache.setAutoDelete(true);
	m_imageCache.setAutoDelete(true);
	
	for (int c=0; c<256; c++)
		clamp[c]=static_cast<unsigned char>(c);
		
	for (int c=256; c<288; c++)
		clamp[c] = 255;
		
}

void PixmapLoader::clear()
{
	//m_cache.clear();
	QPixmapCache::clear();
}

QImage* PixmapLoader::getColored(int name, const QColor& color)
{
	KeramikEmbedImage* edata = KeramikGetDbImage(name);
	if (!edata)
		return 0;

	//Create a real image...
	QImage* img = new QImage(edata->width, edata->height, 32);
	img->setAlphaBuffer(true);


	//OK, now, fill it in, using the color..
	int hue = -1, sat = 0, val = 228;
	if ( color.isValid() ) color.hsv( &hue, &sat, &val );

	
	Q_UINT32* write = reinterpret_cast< Q_UINT32* >(img->bits() );
	int size = img->width()*img->height() * 4;

	for (int pos = 0; pos < size; pos+=4)
	{
		Q_UINT32 red      = edata->data[pos];
		Q_UINT32 green  = edata->data[pos+1];
		Q_UINT32 blue    = edata->data[pos+2];
		Q_UINT32 alpha = edata->data[pos+3];
		
		QColor c( red, green, blue );
		int h, s, v;
		c.hsv( &h, &s, &v );
		if ( hue >= 0 && h >= 0 ) h = ( h + 144 + hue ) % 360;
		if ( s ) s += sat / 2;
		c.setHsv( h, QMIN( s, 255 ), QMIN( v * val / 228, 255 ) );
		*write = qRgba( c.red(), c.green(), c.blue(), alpha );
		write++;
	}
	
	//Discard alpha buffer if it's not needed..
	img->setAlphaBuffer(edata->data[size]);
	//cout<<"Set to:"<<(int)edata->data[size]<<"Has:"<<img->hasAlphaBuffer()<<"\n";
	
	return img;
}

QImage* PixmapLoader::getDisabled(int name, const QColor& color)
{
	KeramikEmbedImage* edata = KeramikGetDbImage(name);
	if (!edata)
		return 0;
		
	//Like getColored, but desaturate a bit, and lower gamma..

	//Create a real image...
	QImage* img = new QImage(edata->width, edata->height, 32);
	img->setAlphaBuffer(true);

	//OK, now, fill it in, using the color..
	Q_UINT32* write = reinterpret_cast< Q_UINT32* >(img->bits() );
	int size = img->width()*img->height() * 4;
	
	int hue = -1, sat = 0, val = 228;
	if ( color.isValid() ) color.hsv( &hue, &sat, &val );

	for (int pos = 0; pos < size; pos+=4)
	{
		Q_UINT32 red      = edata->data[pos];
		Q_UINT32 green  = edata->data[pos+1];
		Q_UINT32 blue    = edata->data[pos+2];
		Q_UINT32 alpha = edata->data[pos+3];
		
		QColor c( red, green, blue );
		int h, s, v;
		c.hsv( &h, &s, &v );
		if ( hue >= 0 && h >= 0 ) h = ( h + 144 + hue ) % 360;
		if ( s ) s += sat / 5;
		c.setHsv( h, QMIN( s, 255 ), QMIN( v * val / 255, 255 ) );

		*write =qRgba(c.red(), c.green(), c.blue(), alpha);

		write++;
	}
	
	//Discard alpha buffer if it's not needed..
	img->setAlphaBuffer(edata->data[size]);
	
	return img;
}

QPixmap PixmapLoader::pixmap( int name, const QColor& color, bool disabled )
{
	KeramikCacheEntry entry(name, color, disabled);
	KeramikCacheEntry* cacheEntry;
	
	int key =entry.key();
	
	if ((cacheEntry = m_pixmapCache.find(key, false)))
	{
		if (entry == *cacheEntry) //True match!
		{
			m_pixmapCache.find(key, true); 
			return *cacheEntry->m_pixmap;
		}
		//else
			//cerr<<"Mismatch!\n";
	}
	

	QImage* img = 0;	
	QPixmap* result = 0;
	KeramikImageCacheEntry imageEntry(name, color, disabled);
	KeramikImageCacheEntry* imageCacheEntry;
	int imgKey = imageEntry.key();
	if ((imageCacheEntry = m_imageCache.find(imgKey, false)))
	{
		if (imageEntry == *imageCacheEntry) //True match!
		{
			m_imageCache.find(imgKey, true); 
			img = imageCacheEntry->m_image;
		}
		//else
			//cerr<<"Mismatch!\n";
	}
	
	if ( !img )
	{
		if (disabled)
			img = getDisabled(name, color);
		else
			img = getColored(name, color);
		if ( !img )
		{
			//cout<<"Can't find:"<<name<<"\n";
			KeramikCacheEntry* toAdd = new KeramikCacheEntry(entry);
			toAdd->m_pixmap = new QPixmap();
			m_pixmapCache.insert(key, toAdd, 16);
			return QPixmap();
		}

		KeramikImageCacheEntry* imgToAdd = new KeramikImageCacheEntry(imageEntry);
		imgToAdd->m_image = img;
		m_imageCache.insert(imgKey, imgToAdd, img->width()*img->height()*img->depth()/8);
	}
	
	
	//SadEagle: Disabled for now to conserve cache space.
	//if (img->width() >= 32 || img->hasAlphaBuffer())
		result = new QPixmap( *img );
	/*else
	{
		int repeat = int(32.0/float(img->width())+0.9);
		result = new QPixmap( img->width()*repeat, img->height()*repeat);
		QPainter p(result);
		p.drawTiledPixmap(0, 0, result->width(), result->height(), QPixmap(*img) );
		p.end();
	}*/
	KeramikCacheEntry* toAdd = new KeramikCacheEntry(entry);
	toAdd->m_pixmap = result;
	
	m_pixmapCache.insert(key, toAdd, result->width()*result->height()*result->depth()/8);
	
	//delete img;
	return *result;
}

QPixmap PixmapLoader::scale( int name, int width, int height, const QColor& color, bool disabled )
{
	KeramikCacheEntry entry(name, color, disabled, width, height);
	KeramikCacheEntry* cacheEntry;
	
	int key =entry.key();
	
	if ((cacheEntry = m_pixmapCache.find(key, false)))
	{
		if (entry == *cacheEntry) //True match!
		{
			m_pixmapCache.find(key, true); 
			return *cacheEntry->m_pixmap;
		}
		//else
			//cerr<<"Mismatch!\n";
	}
	
	
	QImage* img = 0;	
	QPixmap* result = 0;
	KeramikImageCacheEntry imageEntry(name, color, disabled);
	KeramikImageCacheEntry* imageCacheEntry;
	int imgKey = imageEntry.key();
	if ((imageCacheEntry = m_imageCache.find(imgKey, false)))
	{
		if (imageEntry == *imageCacheEntry) //True match!
		{
			m_imageCache.find(imgKey, true); 
			img = imageCacheEntry->m_image;
		}
		//else
			//cerr<<"Mismatch!\n";
	}
	
	if ( !img )
	{
		//cout<<"Miss on name:"<<name<<"\n";
		if (disabled)
			img = getDisabled(name, color);
		else
			img = getColored(name, color);

		if ( !img )
		{
			KeramikCacheEntry* toAdd = new KeramikCacheEntry(entry);
			toAdd->m_pixmap = new QPixmap();
			m_pixmapCache.insert(key, toAdd, 16);
			return QPixmap();
		}
		
		KeramikImageCacheEntry* imgToAdd = new KeramikImageCacheEntry(imageEntry);
		imgToAdd->m_image = img;
		m_imageCache.insert(imgKey, imgToAdd, img->width()*img->height()*img->depth()/8);
	}
	//else
		//cout<<"Hit on name:"<<name<<"\n";

	result = new QPixmap ( img->scale( width ? width : img->width(), height ? height : img->height() ) );

	KeramikCacheEntry* toAdd = new KeramikCacheEntry(entry);
	toAdd->m_pixmap = result;
	
	m_pixmapCache.insert(key, toAdd, result->width()*result->height()*result->depth()/8);

//	delete img;	
	return *result;
}

QSize PixmapLoader::size( int id )
{
	KeramikEmbedImage* edata = KeramikGetDbImage(id);
	if (!edata)
		return QSize(0,0);
	return QSize(edata->width, edata->height);
}

void TilePainter::draw( QPainter *p, int x, int y, int width, int height, const QColor& color, bool disabled, PaintMode mode )
{
	unsigned int scaledColumns = 0, scaledRows = 0, lastScaledColumn = 0, lastScaledRow = 0;
	int scaleWidth = width, scaleHeight = height;
		
	for ( unsigned int col = 0; col < columns(); ++col )
		if ( columnMode( col ) != Fixed )
		{
			scaledColumns++;
			lastScaledColumn = col;
		}
		else scaleWidth -= PixmapLoader::the().size (absTileName( col, 0 ) ).width();
	for ( unsigned int row = 0; row < rows(); ++row )
		if ( rowMode( row ) != Fixed )
		{
			scaledRows++;
			lastScaledRow = row;
		}
		else scaleHeight -= PixmapLoader::the().size (absTileName( 0, row ) ).height();
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
		
		int realH = h ? h : PixmapLoader::the().size (absTileName( 0, row ) ).height();
		if ( rowMode( row ) == Tiled ) h = 0;

		for ( unsigned int col = 0; col < columns(); ++col )
		{
			int w = columnMode( col ) == Fixed ? 0 : scaleWidth / scaledColumns;
			
			 //else t = tile( col, row, color, disabled );
			 int tileW = PixmapLoader::the().size (absTileName( col, row ) ).width();
			
			if ( scaledColumns && col == lastScaledColumn ) w += scaleWidth - scaleWidth / scaledColumns * scaledColumns;
			int realW = w ? w : tileW;
			if ( columnMode( col ) == Tiled ) w = 0;
			
			if (  tileW ) //!t.isNull() )
				if ( w || h )
				{
					
					if (mode == PaintNormal)
						p->drawTiledPixmap( xpos, ypos, realW, realH, scale( col, row, w, h, color, disabled ) );
					else
					{
						//QPixmap draw(->convertToImage());
						const QBitmap* mask  = scale( col, row, w, h, color, disabled ).mask();
						if (mask)
						{
							p->setPen(Qt::color1);
							p->drawTiledPixmap( xpos, ypos, realW, realH, *mask);
						}
						else
							p->fillRect ( xpos, ypos, realW, realH, Qt::color1);
					}
				}
				else
				{
					if (mode == PaintNormal)
						p->drawTiledPixmap( xpos, ypos, realW, realH, tile( col, row, color, disabled ) );
					else
					{
						const QBitmap* mask = tile( col, row, color, disabled ).mask();
						if (mask)
						{
							p->setPen(Qt::color1);
							p->drawTiledPixmap( xpos, ypos, realW, realH, *mask);
						}
						else
							p->fillRect ( xpos, ypos, realW, realH, Qt::color1);
						
					}
				}
			xpos += realW;
		}
		ypos += realH;
	}
}

RectTilePainter::RectTilePainter( int name,
                                  bool scaleH, bool scaleV,
                                  unsigned int columns, unsigned int rows )
	: TilePainter( name ),
	  m_scaleH( scaleH ),
	  m_scaleV( scaleV )
{
	m_columns =  columns;
	m_rows       = rows;
	
	TileMode mh = m_scaleH ? Scaled : Tiled;
	TileMode mv =m_scaleV ? Scaled : Tiled;
	for (int c=0; c<4; c++)
	{
		if (c != 1)
			colMde[c] = Fixed;
		else
			colMde[c] = mh;
	}
	
	for (int c=0; c<4; c++)
	{
		if (c != 1)
			rowMde[c] = Fixed;
		else
			rowMde[c] = mv;
	}

}

int RectTilePainter::tileName( unsigned int column, unsigned int row ) const
{
	return row *3 +  column;
}

ActiveTabPainter::ActiveTabPainter( bool bottom )
	: RectTilePainter( bottom? keramik_tab_bottom_active: keramik_tab_top_active),
	  m_bottom( bottom )
{
	m_rows = 2;
	if (m_bottom)
	{
		rowMde[0] = rowMde[2] = rowMde[3] = Scaled;
		rowMde[1] = Fixed;
	}
	else
	{
		rowMde[0] = rowMde[2] = rowMde[3] = Fixed;
		rowMde[1] = Scaled;
	}
}

int ActiveTabPainter::tileName( unsigned int column, unsigned int row ) const
{
	if ( m_bottom )
		return RectTilePainter::tileName( column, row + 1 );
	return RectTilePainter::tileName( column, row );
}

InactiveTabPainter::InactiveTabPainter( Mode mode, bool bottom )
	: RectTilePainter( bottom? keramik_tab_bottom_inactive: keramik_tab_top_inactive),
	  m_mode( mode ), m_bottom( bottom )
{
	m_rows = 2;
	if (m_bottom)
	{
		rowMde[0] = rowMde[2] = rowMde[3] = Scaled;
		rowMde[1] = Fixed;
	}
	else
	{
		rowMde[0] = rowMde[2] = rowMde[3] = Fixed;
		rowMde[1] = Scaled;
	}
	
	
	Mode check = QApplication::reverseLayout() ? First : Last;
	m_columns = (m_mode == check ? 3 : 2);
}

int InactiveTabPainter::tileName( unsigned int column, unsigned int row ) const
{
	Mode check = QApplication::reverseLayout() ? Last : First;
	if ( column == 0 && m_mode != check ) return KeramikTileSeparator;
	if ( m_bottom )
		return RectTilePainter::tileName( column, row + 1 );
	return RectTilePainter::tileName( column, row );
}

ScrollBarPainter::ScrollBarPainter( int type, int count, bool horizontal )
	: TilePainter( name( horizontal ) ),
	  m_type( type ),
	  m_count( count ),
	  m_horizontal( horizontal )
{
	for (int c=0; c<5; c++)
	{
		if ( !m_horizontal || !( c % 2 ) ) colMde[c] = Fixed;
		else colMde[c] =  ( m_count == 2 ) ? Scaled : Tiled;
		
		if ( m_horizontal || !( c % 2 ) ) rowMde[c] = Fixed;
		else rowMde[c] =  ( m_count == 2 ) ? Scaled : Tiled;
	}
	
	m_columns = m_horizontal ? m_count : 1;
	m_rows       = m_horizontal ? 1 : m_count;
	
}

int ScrollBarPainter::name( bool horizontal )
{
	return horizontal? keramik_scrollbar_hbar: keramik_scrollbar_vbar;
}

int ScrollBarPainter::tileName( unsigned int column, unsigned int row ) const
{
	unsigned int num = ( column ? column : row ) + 1;
	if ( m_count == 5 )
		if ( num == 3 ) num = 4;
		else if ( num == 4 ) num = 2;
		else if ( num == 5 ) num = 3;

	return m_type + (num-1)*16; //CHECKME -- offset?
}

int SpinBoxPainter::tileName( unsigned int column, unsigned int ) const
{
	return column + 1;
}

// vim: ts=4 sw=4 noet
