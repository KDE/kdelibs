/***************************************************************************
                          kplotobject.h - A list of points to be plotted
                             -------------------
    begin                : Sun 18 May 2003
    copyright            : (C) 2003 by Jason Harris
    email                : kstars@30doradus.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KPLOTOBJECT_H
#define KPLOTOBJECT_H

#include <QColor>
#include <QPointF>
#include <QRectF>
#include <QString>

#include <kdemacros.h>

/**
 * @class KPlotObject
 * @short Encapsulates an object to be plotted in a KPlotWidget.
 * @author Jason Harris
 * @version 1.0
 * Each KPlotObject consists of a list of QPoints, an object type, a color, a size,
 * and a QString name. An additional integer (param) specifies something further
 * about the object's appearance, depending on its type.  There is a draw function
 * for plotting the object on a KPlotWidget's QPainter.
 */
class KDE_EXPORT KPlotObject{
public:
	/**
	 * @enum PTYPE
	 * The Type classification of the KPlotObject
	 */
	enum PTYPE { POINTS=0, CURVE=1, LABEL=2, POLYGON=3, UNKNOWN_TYPE };

	/**
	 * @enum PPARAM
	 * Parameter specifying the kind of points
	 */
	enum PPARAM { DOT=0, CIRCLE=1, SQUARE=2, LETTER=3, UNKNOWN_POINT };

	/**
	 * @enum CPARAM
	 * Parameter specifying the kind of line. These are numerically equal to
	 * the Qt::PenStyle enum values.
	*/
	enum CPARAM { NO_LINE=0, SOLID=1, DASHED=2, DOTTED=3, DASHDOTTED=4, DASHDOTDOTTED=5, UNKNOWN_CURVE };

	/**
	 * Default constructor.  Create a POINTS-type object with an empty list of points.
	 */
	KPlotObject();

	/**
	 * Constructor. Create a KPlotObject according to the arguments.
	 */
	KPlotObject( const QString &name, const QColor &color, PTYPE otype, unsigned int size=2, unsigned int param=0 );

	/**
	 * Destructor (empty)
	 */
	~KPlotObject();

	/**
	 * @return the KPlotObject's Name
	 */
	QString name() const { return Name; }

	/**
	 * Set the KPlotObject's Name
	 * @param n the new name
	 */
	void setName( const QString &n ) { Name = n; }

	/**
	 * @return the KPlotObject's Color
	 */
	QColor color() const { return Color; }

	/**
	 * Set the KPlotObject's Color
	 * @param c the new color
	 */
	void setColor( const QColor &c ) { Color = c; }

	/**
	 * @return the KPlotObject's Type
	 */
	PTYPE type() const { return Type; }

	/**
	 * Set the KPlotObject's Type
	 * @param t the new type
	 */
	void setType( PTYPE t ) { Type = t; }

	/**
	 * @return the KPlotObject's Size
	*/
	unsigned int size() const { return Size; }

	/**
	 * Set the KPlotObject's Size
	 * @param s the new size
	 */
	void setSize( unsigned int s ) { Size = s; }

	/**
	 * @return the KPlotObject's type-specific Parameter
	 * Parameter is an unsigned int because it can either be a PPARAM or a CPARAM enum.
	 */
	unsigned int param() const { return Parameter; }

	/**
	 * Set the KPlotObject's type-specific Parameter
	 * @param p the new parameter
	 * Parameter is an unsigned int because it can either be a PPARAM or a CPARAM enum.
	 */
	void setParam( unsigned int p ) { Parameter = p; }

	/**
	 * @return a pointer to the QPointF at position i
	 * @param i the index of the desired point.
	 */
	QPointF* point( unsigned int i ) { return pList.at(i); }

	QList<QPointF*> *points() { return &pList; }

	/**
	 * Add a point to the object's list.
	 * @param p the QPointF to add.
	 */
	void addPoint( const QPointF &p ) { pList.append( new QPointF( p.x(), p.y() ) ); }

	/**
	 * Add a point to the object's list.
	 * @overload
	 * @param p pointer to the QPointF to add.
	 */
	void addPoint( QPointF *p ) { pList.append( p ); }

	/**
	 * Remove the QPoint at position index from the list of points
	 * @param index the index of the point to be removed.
	 */
	void removePoint( int index );

	/**
	 * @return the number of QPoints currently in the list
	 */
	unsigned int count() const { return pList.count(); }

	/**
	 * Clear the Object's points list
	 */
	void clearPoints() { pList.clear(); }

private:
	QList<QPointF*> pList;
	PTYPE Type;
	unsigned int Size, Parameter;
	QString Name;
	QColor Color;
};

#endif
