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

#ifndef __FX_DATA
#define __FX_DATA

#define SHADOW_CONFIG_ENTRY QString("ShadowCurrentScheme")
#define SHADOW_TEXT_COLOR QString("ShadowTextColor")
#define SHADOW_TEXT_BACKGROUND QString("ShadowTextBackground")

// fallback configuration string
#define DEFAULT_SHADOW_CONFIGURATION QString("-1,0,0,4.0,255.0,3,0,0,0,0,0")

/**
 * This class is the implementation of a structure for the
 * various parameters required by the shadow class.
 *
 * One may afford this implementation since the shadow class is
 * designed to be used as singleton for an application.
 * @see KShadowEngine
 * @author laur.ivan@corvil.com
 * @since 3.2
 */
class KShadowItem
{
	public:
		/**
		 * Specifies the order of the options.
		 * @see fromString
		 */
		enum ConfigurationOrder
		{
			ID = 0,
			OFFSET_X = ID + 1,
			OFFSET_Y = OFFSET_X + 1,
			MULTIPLICATION_FACTOR = OFFSET_Y + 1,
			MAX_OPACITY = MULTIPLICATION_FACTOR + 1,
			THICKNESS = MAX_OPACITY + 1,
			ALGORITHM = THICKNESS + 1,
			SELECTION_TYPE = ALGORITHM + 1
		};
		
	public:
		/**
		 * The algorithm used.
		 */
		enum Algorithm
		{
			DefaultDecay = 1,   ///< the default AXIS/DIAGONAL_FACTOR based alg
			DoubleLinearDecay,  ///< decay factor is 1/dx+dy
			RadialDecay,	    ///< decay factor is 1/sqrt(dx*dx + dy*dy)
			NoDecay		    ///< decay factor is 1 always
		};
		
		/**
		 * The selected method used.
		 */
		enum SelectionType
		{
			InverseVideoOnSelection = 0,	///< when selected, the halo is on I/Video
			SelectionColorsOnSelection	///< the halo is made w/ selected colors
		};

		/**
		 * The default constructor.
		 * Creates an object with default settings for all the variabless.
		 */
		KShadowItem();
		
		// load/save methods
		/**
		 * Loads the configuration from a string.
		 * @param the string to load from (comma-separated values)
		 * @see ConfigurationOrder
		 */
		virtual void fromString(const QString &s);
		/**
		 * Saves the configuration to a string.
		 * @return the configuration (comma-separated values)
		 * @see ConfigurationOrder
		 */ 
		virtual QString toString() const;
		
		// get methods
		/**
		 * Returns the id of the item.
		 * @return the id of the item 
		 */
		int id() const;
		/**
		 * Returns the x offset of the item.
		 * @return the x offset
		 */
		int offsetX() const;
		/**
		 * Returns the y offset of the item.
		 * @return the y offset
		 */
		int offsetY() const;
		/**
		 * Returns the multiplication factor.
		 * @return the multiplication factor
		 */
		double multiplicationFactor() const;
		/**
		 * Returns the maximum opacity of the item.
		 * @return the maximum opacity
		 */
		double maxOpacity() const;
		/**
		 * Returns the thickness.
		 * @return the thickness
		 */ 
		int thickness() const;
		/**
		 * Returns the used algorithm.
		 * @return the algorithm used
		 */
		Algorithm algorithm() const;
		/** 
		 * Returns the selection type used.
		 * @return the selection type
		 */
		SelectionType selectionType() const;
		
		// set methods
		/**
		 * Sets default values.
		 */
		virtual void setDefaults();
		
		/**
		 * Sets the id of the item.
		 * @param id the id of the item 
		 */
		virtual void setId(int id);
		/**
		 * Sets the used algorithm.
		 * @param a the algorithm used
		 */
		virtual void setAlgorithm(Algorithm a);
		/**
		 * Sets the multiplication factor.
		 * @param mf the multiplication factor
		 */
		virtual void setMultiplicationFactor(double mf);
		/**
		 * Sets the maximum opacity of the item.
		 * @param mo the maximum opacity
		 */
		virtual void setMaxOpacity(double mo);
		/**
		 * Sets the x offset of the item.
		 * @param x the x offset
		 */
		virtual void setOffsetX(int x);
		/**
		 * Sets the y offset of the item.
		 * @param y the y offset
		 */
		virtual void setOffsetY(int y);
		/**
		 * Sets the thickness.
		 * @param t the thickness
		 */ 
		virtual void setThickness(int t);
		/** 
		 * Sets the selection type used.
		 * @param s the selection type
		 */
		virtual void setSelectionType(SelectionType s);

	private:
		
		/*
		 * The employed algorithm (see fxshadow.h)
		 */
		Algorithm _algorithm;

		/**
		 * This is the multiplication factor for the resulted shadow
		 */
		double _multiplicationFactor;

		/**
		 * The maximum permitted opacity for the shadow
		 */
		double _maxOpacity;

		/*
		 * offsetX and offsetY are the x/y offsets of the shadow with
		 * the mention that 0,0 is a centered shadow.
		 */
		int _offsetX;
		int _offsetY;

		/*
		 * The shadow thickness:
		 * shadow is this many pixels thicker than the text.
		 */
		int _thickness;
		
		/*
		 * If the value is InverseVideoOnSelection, then the fg/bg
		 * colours are swapped when the element is selected.
		 * Otherwise, the selected fg/bg colors are used for text
		 * as well
		 */
		SelectionType _selectionType;
		
		/*
		 * This is an identifier of the shadow
		 */
		int _id;
		
		void *d;
};


#endif
