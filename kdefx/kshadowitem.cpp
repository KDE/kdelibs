/* This file is proposed to be part of the KDE libraries.
 * Copyright (C) 2003 Laur Ivan <laurivan@eircom.net>
 *
 * Many thanks to:
 *  - Tim Jansen <tim@tjansen.de> for the API updates and fixes.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <qstring.h>
#include <qstringlist.h>
#include "kshadowitem.h"

KShadowItem::KShadowItem()
{
	// init the components with some default values
	setDefaults();
}

// load/save methods
void KShadowItem::fromString(const QString &val)
{

/*	if (val == QString::null) 
	{
    fromString(DEFAULT_CONFIGURATION);
    return;
	}

  QStringList l= QStringList::split(',', val);

  if (l.count() < SELECTION_TYPE)
  {
    fromString(DEFAULT_CONFIGURATION);
    return;
  }
*/
  setId(val.section(',', ID, ID).toInt());
  setOffsetX(val.section(',', OFFSET_X, OFFSET_X).toInt());
  setOffsetY(val.section(',', OFFSET_Y, OFFSET_Y).toInt());
  setMultiplicationFactor(val.section(',', MULTIPLICATION_FACTOR, MULTIPLICATION_FACTOR).toDouble());
  setMaxOpacity(val.section(',', MAX_OPACITY, MAX_OPACITY).toDouble());
  setThickness(val.section(',', THICKNESS, THICKNESS).toInt());
  setAlgorithm((Algorithm) val.section(',', ALGORITHM, ALGORITHM).toInt());
  setSelectionType((SelectionType)val.section(',', SELECTION_TYPE, SELECTION_TYPE).toInt());
}

QString KShadowItem::toString() const
{
  QString result;
  result.sprintf("%d,%d,%d,%f,%f,%d,%d,%d",
		 id(),
		 offsetX(),
		 offsetY(),
		 multiplicationFactor(),
		 maxOpacity(),
		 thickness(),
		 (int)algorithm(),
		 (int)selectionType());
  return result;
}

//***********************************
//               get methods
//***********************************

/**
 * Returns the item's ID
 */
int KShadowItem::id() const
{
	return _id;
}

/**
 * Returns the decay algorithm to be used (see the alg. enumeration in the .h)
 */
KShadowItem::Algorithm KShadowItem::algorithm() const
{
	return _algorithm;
}

/**
 * Returns a multiplication facor used to average the resulted data
 */
double KShadowItem::multiplicationFactor() const
{
	return _multiplicationFactor;
}

/**
 * Returns the max opacity allowed (0 = transparent, 255 = opaque)
 */
double KShadowItem::maxOpacity() const
{
	return _maxOpacity;
}

/**
 * Returns the Y offset (0 is centered on text)
 */
int KShadowItem::offsetX() const
{
	return _offsetX;
}

/**
 * Returns the Y offset (0 is centered on text)
 */
int KShadowItem::offsetY() const
{
	return _offsetY;
}

/**
 * Returns the thickness. Used by the KShadow algorithm
 */
int KShadowItem::thickness() const
{
	return _thickness;
}

/**
 *
 */
KShadowItem::SelectionType KShadowItem::selectionType() const
{
	return _selectionType;
}

// set methods
/**
 *
 */
void KShadowItem::setDefaults()
{
	fromString(DEFAULT_SHADOW_CONFIGURATION);
}


/**
 *
 */
void KShadowItem::setId(int val)
{
	_id = val;
}

/**
 *
 */
void KShadowItem::setAlgorithm(Algorithm val)
{
	_algorithm = val;
}

/**
 *
 */
void KShadowItem::setMultiplicationFactor(double val)
{
	_multiplicationFactor = val;
}

/**
 *
 */
void KShadowItem::setMaxOpacity(double val)
{
	_maxOpacity = val;
}

/**
 *
 */
void KShadowItem::setOffsetX(int val)
{
	_offsetX = val;
}

/**
 *
 */
void KShadowItem::setOffsetY(int val)
{
	_offsetY = val;
}

/**
 *
 */
void KShadowItem::setThickness(int val)
{
	_thickness = val;
}

/**
 *
 */
void KShadowItem::setSelectionType(SelectionType val)
{
	_selectionType = val;
}
