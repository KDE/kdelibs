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

		QPixmap pixmap( const QCString& name, const QColor& color, bool disabled = false );
		QPixmap scale( const QCString& name, int width, int height, const QColor& color, bool disabled = false );
		QSize size( const QCString& name );

		void clear();

		static PixmapLoader& the() { return s_instance; }

	private:
		void colorize( QImage &img, const QColor& color );
		void makeDisabled( QImage &img, const QColor& color );

		QDict< QImage > m_cache;

		static PixmapLoader s_instance;
	};

	class TilePainter
	{
	public:
		TilePainter( const QCString& name ) : m_name( name ) {};
		virtual ~TilePainter() {};

		void draw( QPainter *p, int x, int y, int width, int height, const QColor& color, bool disabled = false );
		void draw( QPainter *p, const QRect& rect, const QColor& color, bool disabled = false )
		{
			draw( p, rect.x(), rect.y(), rect.width(), rect.height(), color, disabled );
		}

	protected:
		enum TileMode { Fixed, Scaled, Tiled };

		virtual unsigned int columns() const = 0;
		virtual unsigned int rows() const  = 0;
		virtual QCString tileName( unsigned int, unsigned int ) const { return 0; }
		virtual TileMode columnMode( unsigned int ) const = 0;
		virtual TileMode rowMode( unsigned int ) const = 0;

	private:
		QCString absTileName( unsigned int column, unsigned int row ) const;
		QPixmap tile( unsigned int column, unsigned int row, const QColor& color, bool disabled )
			{ return PixmapLoader::the().pixmap( absTileName( column, row ), color, disabled ); }
		QPixmap scale( unsigned int column, unsigned int row, int width, int height, const QColor& color, bool disabled )
			{ return PixmapLoader::the().scale( absTileName( column, row ), width, height, color, disabled ); }

		QCString m_name;
	};

	class ScaledPainter : public TilePainter
	{
	public:
		enum Direction { Horizontal = 1, Vertical = 2, Both = Horizontal | Vertical };
		ScaledPainter( const QCString& name, Direction direction = Both )
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
		CenteredPainter( const QCString& name ) : TilePainter( name ) {};
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
		RectTilePainter( const QCString& name,
		                 bool scaleH = true, bool scaleV = true,
		                 unsigned int columns = 3, unsigned int rows = 3 );
		virtual ~RectTilePainter() {};

	protected:
		virtual unsigned int columns() const { return m_columns; }
		virtual unsigned int rows() const { return m_rows; }
		virtual QCString tileName( unsigned int column, unsigned int row ) const;
		virtual TileMode columnMode( unsigned int column ) const;
		virtual TileMode rowMode( unsigned int row ) const;

	private:
		bool m_scaleH;
		bool m_scaleV;
		unsigned int m_columns;
		unsigned int m_rows;
	};

	class ActiveTabPainter : public RectTilePainter
	{
	public:
		ActiveTabPainter( bool bottom );
		virtual ~ActiveTabPainter() {};
	
	protected:
		virtual unsigned int rows() const { return 2; }
		virtual TileMode rowMode( unsigned int row ) const;
		virtual QCString tileName( unsigned int column, unsigned int row ) const;

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
		virtual unsigned int columns() const;
		virtual unsigned int rows() const { return 2; }
		virtual TileMode rowMode( unsigned int row ) const;
		virtual QCString tileName( unsigned int column, unsigned int row ) const;

	private:
		Mode m_mode;
		bool m_bottom;
	};

	class ScrollBarPainter : public TilePainter
	{
	public:
		ScrollBarPainter( const QCString& type, int count, bool horizontal );
		virtual ~ScrollBarPainter() {};

		static QCString name( bool horizontal );

	protected:
		virtual unsigned int columns() const { return m_horizontal ? m_count : 1; }
		virtual unsigned int rows() const { return m_horizontal ? 1 : m_count; }
		virtual QCString tileName( unsigned int column, unsigned int row ) const;
		virtual TileMode columnMode( unsigned int column ) const;
		virtual TileMode rowMode( unsigned int row ) const;

	private:
		QCString m_type;
		int m_count;
		bool m_horizontal;
	};

	class SpinBoxPainter : public TilePainter
	{
	public:
		SpinBoxPainter() : TilePainter( "spinbox" ) {};
		virtual ~SpinBoxPainter() {};

	protected:
		virtual unsigned int columns() const { return 3; }
		virtual unsigned int rows() const { return 1; }
		virtual TileMode columnMode( unsigned int ) const;
		virtual TileMode rowMode( unsigned int ) const { return Scaled; }
		virtual QCString tileName( unsigned int column, unsigned int row ) const;
	};
};

#endif

// vim: ts=4 sw=4 noet
