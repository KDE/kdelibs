/* 
   Copyright (c) 2002 Malte Starostik <malte@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
 
   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

// $Id$

#ifndef __pixmaploader_h__
#define __pixmaploader_h__

#include <qintcache.h>
#include <qdict.h>
#include <qimage.h>

class QPixmap;
class QImage;

#include "keramikrc.h"

namespace Keramik
{
	class PixmapLoader
	{
	public:
		PixmapLoader();

		QPixmap pixmap( int name, const QColor& color, bool disabled = false );
		QPixmap scale( int name, int width, int height, const QColor& color, bool disabled = false );
		QSize size( int id );

		void clear();

		static PixmapLoader& the()
		{
			 if (!s_instance) 
			 	s_instance = new PixmapLoader;
			return *s_instance;
		}
		
		static void release()
		{
			delete s_instance;
			s_instance = 0;
		}

	private:
		struct KeramikCacheEntry
		{
			int m_id;
			int m_width;
			int m_height;
			QRgb m_colorCode;
			bool    m_disabled;
			
			QPixmap* m_pixmap;
			
			KeramikCacheEntry(int id, const QColor& color, bool disabled, int width = -1 , int height = -1, QPixmap* pixmap = 0 ):
				m_id(id), m_width(width), m_height(height), m_colorCode(color.rgb()),m_disabled(disabled),  m_pixmap(pixmap)
			{}
			
			int key()
			{
				return m_disabled ^ (m_id<<1) ^ (m_width<<11) ^ (m_height<<22) ^ m_colorCode;
			}
			
			bool operator == (const KeramikCacheEntry& other)
			{
				return (m_id        == other.m_id) &&
							(m_width   == other.m_width) &&
							(m_height == other.m_height) &&
							(m_colorCode == other.m_colorCode) && 
							(m_disabled == other.m_disabled);
			}
						
			~KeramikCacheEntry()
			{
				/*if (m_pixmap)
				{
					cerr<<"Deleting pixmap!\n";
				}*/
				delete m_pixmap;
			}
		};
		
		struct KeramikImageCacheEntry
		{
			int m_id;
			QRgb m_colorCode;
			bool    m_disabled;
			
			QImage* m_image;
			
			KeramikImageCacheEntry(int id, const QColor& color, bool disabled, QImage* image = 0 ):
				m_id(id), m_colorCode(color.rgb()), m_disabled(disabled), m_image(image)
			{}
			
			int key()
			{
				return m_disabled ^ (m_id<<1) ^ m_colorCode;
			}
			
			bool operator == (const KeramikImageCacheEntry& other)
			{
				return (m_id        == other.m_id) &&
							(m_colorCode == other.m_colorCode) && 
							(m_disabled == other.m_disabled);
			}
						
			~KeramikImageCacheEntry()
			{
				//if (m_image)
				//	cerr<<"Deleting image!\n";
				delete m_image;
			}
		};

		
		QImage* getColored(int id, const QColor& color);
		QImage* getDisabled(int id, const QColor& color);
		QIntCache <KeramikImageCacheEntry> m_imageCache;
		QIntCache <KeramikCacheEntry>  m_pixmapCache;
		
		
		unsigned char clamp[288];//256+32

		static PixmapLoader* s_instance;
	};

	class TilePainter
	{
	public:
		TilePainter( int name ) : m_columns(1),m_rows(1),m_name( name ) {};
		virtual ~TilePainter() {};
		
		enum PaintMode
		{
			PaintNormal,
			PaintMask
		};

		void draw( QPainter *p, int x, int y, int width, int height, const QColor& color,
		                 bool disabled = false, PaintMode mode = PaintNormal );
		void draw( QPainter *p, const QRect& rect, const QColor& color, bool disabled = false, PaintMode mode = PaintNormal )
		{
			draw( p, rect.x(), rect.y(), rect.width(), rect.height(), color, disabled, mode );
		}

	protected:
		enum TileMode { Fixed, Scaled, Tiled };

		unsigned int columns() const { return m_columns; }
		unsigned int rows() const { return m_rows; }
		
		virtual int tileName( unsigned int, unsigned int ) const { return 0; }
		
		TileMode columnMode( unsigned int col) const
		{
			return colMde[col];
		}
		
		TileMode rowMode( unsigned int row) const
		{
			return rowMde[row];
		}

	protected:
		TileMode colMde[5], rowMde[5];
		unsigned int m_columns;
		unsigned int m_rows;
	private:
	
		int absTileName( unsigned int column, unsigned int row ) const
		{
			int name = tileName( column, row );
			//cout<<"@"<<row<<","<<column<<":"<<m_name<<":"<<name<<"\n";
			return m_name + name;
		}
		
		
		QPixmap tile( unsigned int column, unsigned int row, const QColor& color, bool disabled )
			{ return PixmapLoader::the().pixmap( absTileName( column, row ), color, disabled ); }
		QPixmap scale( unsigned int column, unsigned int row, int width, int height, const QColor& color, bool disabled )
			{ return PixmapLoader::the().scale( absTileName( column, row ), width, height, color, disabled ); }

		int m_name;
		
	};

	class ScaledPainter : public TilePainter
	{
	public:
		enum Direction { Horizontal = 1, Vertical = 2, Both = Horizontal | Vertical };
		ScaledPainter( int name, Direction direction = Both )
			: TilePainter( name ), m_direction( direction )
		{
			colMde[0] = colMde[1] = colMde[2] = colMde[3] = ( m_direction & Horizontal ) ? Scaled : Fixed;
			rowMde[0] = rowMde[1] = rowMde[2] = rowMde[3] = ( m_direction & Vertical ) ? Scaled : Fixed;
		}
		
		virtual ~ScaledPainter() {};

	private:
		Direction m_direction;
	};

	class CenteredPainter : public TilePainter
	{
	public:
		CenteredPainter( int name ) : TilePainter( name ) {
			colMde[0] = colMde[1] = colMde[2] = colMde[3] =  Fixed;
			rowMde[0] = rowMde[1] = rowMde[2] = rowMde[3] = Fixed;
		};
		virtual ~CenteredPainter() {};

	protected:
	};

	class RectTilePainter : public TilePainter
	{
	public:
		RectTilePainter( int name,
		                 bool scaleH = true, bool scaleV = true,
		                 unsigned int columns = 3, unsigned int rows = 3 );
		
		virtual ~RectTilePainter() {};

	protected:
		virtual int tileName( unsigned int column, unsigned int row ) const;
	private:
		bool m_scaleH;
		bool m_scaleV;
	};

	class ActiveTabPainter : public RectTilePainter
	{
	public:
		ActiveTabPainter( bool bottom );
		virtual ~ActiveTabPainter() {};
	
	protected:
		virtual int tileName( unsigned int column, unsigned int row ) const;

	private:
		bool m_bottom;
	};

	class InactiveTabPainter : public RectTilePainter
	{
	public:
		enum Mode { First, Middle, Last };
		InactiveTabPainter( Mode mode, bool bottom );
		virtual ~InactiveTabPainter() {};

	protected:
		virtual int tileName( unsigned int column, unsigned int row ) const;

	private:
		Mode m_mode;
		bool m_bottom;
	};

	class ScrollBarPainter : public TilePainter
	{
	public:
		ScrollBarPainter( int type, int count, bool horizontal );
		virtual ~ScrollBarPainter() {};

		static int name( bool horizontal );

	protected:
		virtual int tileName( unsigned int column, unsigned int row ) const;
	private:
		int m_type;
		int m_count;
		bool m_horizontal;
	};

	class SpinBoxPainter : public TilePainter
	{
	public:
		SpinBoxPainter() : TilePainter( keramik_spinbox ) {
			colMde[0] = colMde[2] = Fixed;
			colMde[1] = Scaled;
			rowMde[0]=rowMde[1]=rowMde[2] = Scaled;
			m_columns = 3;
		};
		virtual ~SpinBoxPainter() {};

	protected:
		virtual int tileName( unsigned int column, unsigned int row ) const;
	};
};

#endif

// vim: ts=4 sw=4 noet
