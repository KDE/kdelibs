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

#include <qdict.h>

class QPixmap;
class QImage;

namespace Keramik
{
	class PixmapLoader
	{
	public:
		PixmapLoader();

		QPixmap pixmap( const QString& name );
		QPixmap scale( const QString& name, int width, int height );

		void setColor( const QColor& color );

		static PixmapLoader& the() { return s_instance; }

	private:
		void colorize( QImage &img );

		QDict< QImage > m_cache;
		QColor m_color;

		static PixmapLoader s_instance;
	};

	class TilePainter
	{
	public:
		TilePainter( const QString& name ) : m_name( name ) {};
		virtual ~TilePainter() {};

		void draw( QPainter *p, int x, int y, int width, int height );
		void draw( QPainter *p, const QRect& rect )
		{
			draw( p, rect.x(), rect.y(), rect.width(), rect.height() );
		}

	protected:
		enum TileMode { Fixed, Scaled };

		virtual unsigned int columns() const = 0;
		virtual unsigned int rows() const  = 0;
		virtual QString tileName( unsigned int, unsigned int ) const { return QString::null; }
		virtual TileMode columnMode( unsigned int ) const = 0;
		virtual TileMode rowMode( unsigned int ) const = 0;

	private:
		QString absTileName( unsigned int column, unsigned int row ) const;
		QPixmap tile( unsigned int column, unsigned int row )
			{ return PixmapLoader::the().pixmap( absTileName( column, row ) ); }
		QPixmap scale( unsigned int column, unsigned int row, int width, int height )
			{ return PixmapLoader::the().scale( absTileName( column, row ), width, height ); }

		QString m_name;
	};

	class ScaledPainter : public TilePainter
	{
	public:
		enum Direction { Horizontal = 1, Vertical = 2, Both = Horizontal | Vertical };
		ScaledPainter( const QString& name, Direction direction = Both )
			: TilePainter( name ), m_direction( direction ) {};
		virtual ~ScaledPainter() {};

	protected:
		virtual unsigned int columns() const { return 1; }
		virtual unsigned int rows() const { return 1; }
		virtual TileMode columnMode( unsigned int ) const
		{
			return ( m_direction & Horizontal ) ? Scaled : Fixed;
		}
		virtual TileMode rowMode( unsigned int ) const
		{
			return ( m_direction & Vertical ) ? Scaled : Fixed;
		}

	private:
		Direction m_direction;
	};

	class CenteredPainter : public TilePainter
	{
	public:
		CenteredPainter( const QString& name ) : TilePainter( name ) {};
		virtual ~CenteredPainter() {};

	protected:
		virtual unsigned int columns() const { return 1; }
		virtual unsigned int rows() const { return 1; }
		virtual TileMode columnMode( unsigned int ) const { return Fixed; }
		virtual TileMode rowMode( unsigned int ) const { return Fixed; }
	};

	class RectTilePainter : public TilePainter
	{
	public:
		RectTilePainter( const QString& name, unsigned int columns = 3, unsigned int rows = 3 );
		virtual ~RectTilePainter() {};

	protected:
		virtual unsigned int columns() const { return m_columns; }
		virtual unsigned int rows() const { return m_rows; }
		virtual QString tileName( unsigned int column, unsigned int row ) const;
		virtual TileMode columnMode( unsigned int column ) const
		{
			return column == 1 ? Scaled : Fixed;
		}
		virtual TileMode rowMode( unsigned int row ) const
		{
			return row == 1 ? Scaled : Fixed;
		}

	private:
		unsigned int m_columns;
		unsigned int m_rows;
	};

	class TabPainter : public RectTilePainter
	{
	public:
		enum Mode { First, Middle, Last };
		TabPainter( const QString& name, Mode mode, bool bottom )
			: RectTilePainter( name ), m_mode( mode ), m_bottom( bottom ) {};
		virtual ~TabPainter() {};

	protected:
		virtual unsigned int columns() const;
		virtual unsigned int rows() const { return 2; }
		virtual QString tileName( unsigned int column, unsigned int row ) const;

	private:
		Mode m_mode;
		bool m_bottom;
	};

	class ScrollBarPainter : public TilePainter
	{
	public:
		ScrollBarPainter( const QString& type, int count, bool horizontal );
		virtual ~ScrollBarPainter() {};

		static QString name( bool horizontal );

	protected:
		virtual unsigned int columns() const { return m_horizontal ? m_count : 1; }
		virtual unsigned int rows() const { return m_horizontal ? 1 : m_count; }
		virtual QString tileName( unsigned int column, unsigned int row ) const
		{
			return m_type + QString::number( column ? column + 1 : row + 1 );
		}
		virtual TileMode columnMode( unsigned int column ) const
		{
			return m_horizontal ? ( column % 2 ) ? Scaled : Fixed : Fixed;
		}
		virtual TileMode rowMode( unsigned int row ) const
		{
			return m_horizontal ? Fixed : ( row % 2 ) ? Scaled : Fixed;
		}

	private:
		QString m_type;
		int m_count;
		bool m_horizontal;
	};
};

#endif

// vim: ts=4 sw=4 noet
