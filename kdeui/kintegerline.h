/**********************************************************************
**
** $Id$
**
** Definition of KIntegerLine
**
** Copyright (C) 1997 Michael Wiedmann, <mw@miwie.in-berlin.de>
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the Free
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
*****************************************************************************/

#ifndef KINTEGERLINE_H
#define KINTEGERLINE_H

#include "krestrictedline.h"

/** Enum for the possible types of Editlines
    */
enum KEditLineType{
  KEditTypeOct =  8,
  KEditTypeDec = 10,
  KEditTypeHex = 16
};

/** IntegerEditline: Editline for Integers. Only octal, decimal or
    hexadecimal characters are valid input characters for this sort 
    of edit lines. 
    A few special keys are supported by this class:
    <ul>
    <li>The up-arrow increments the contents by 1, 
    <li>the down-arrow decrements the contents by 1,
    <li>Page-Up increments by 8, 10 or 16 (depending on the EditLineType),
    <li>Page-Down decrements by 8, 10 or 16 (depending on the EditLinetype)
    </ul>

    @author Michael Wiedmann <mw@miwie.in-berlin.de>
    @version 0.0.1
  */
class KIntegerLine : public KRestrictedLine
{
  Q_OBJECT
  
public:
  /**@name methods */
  //@{
  /** Default - empty - constructor
	*/
  KIntegerLine();
    
  /** Contructor: This constructor takes three - optional - arguments.
	  The first two parameters are simply passed to KRestrictedLine.
	  @param parent   pointer to the parent widget
	  @param name     pointer to the name of this widget
	  @param t        type of this integer line (defaults to KEditTypeDec)
  */
  KIntegerLine( QWidget *parent=0, 
				const char *name=0, 
				KEditLineType t=KEditTypeDec);

  /// Destructor
  ~KIntegerLine();

  /// Get the type of this Line
  KEditLineType getType();
  //@}

protected:
  /** Key press event handler. 
	  Handles the following special keys:
	  <UL>
	  <LI>Key_Up:    Increments contents of line by 1
	  <LI>Key_Prior: Increments contents of line by 8, 10 or 16
	  <LI>Key_Down:  Decrements contents of line by 1
	  <LI>Key_Next:  Decrements contents of line by 8, 10 or 16
	  </UL>
  */
  void	keyPressEvent( QKeyEvent *e );

private:
  /// type of this line
  KEditLineType lineType;

  /// get value based on radix of this line
  int getValue(QString &s);
    
  /// put value based on radix of this line
  void putValue(QString &s, int val);  

  /// increment value based on radix of this line
  void incValue(QString &s, int val);

  /// decrement value based on radix of this line
  void decValue(QString &s, int val);
};

#endif // 

