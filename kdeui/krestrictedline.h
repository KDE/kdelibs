/**********************************************************************
**
** $Id$
**
** Definition of KRestrictedLine
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

#ifndef KRESTRICTEDLINE_H
#define KRESTRICTEDLINE_H

#include "qlined.h"

/** Restricted Editline: Only selected Characters are valid input. 
    All other characters will be discarded and the signal invalidChar() 
    will be emitted for each of them. Valid characters can be passed 
    through a pointer to the constructor or call of a public method.
    The default key binding of QLineEdit is still in effect.

    @author Michael Wiedmann <mw@miwie.in-berlin.de>
    @version 0.0.1
  */
class KRestrictedLine : public QLineEdit
{
  Q_OBJECT
  
public:
  /**@name methods */
  //@{
      
  /** Constructor: This contructor takes three - optional - arguments.
	  The first two parameters are simply passed on to QLineEdit.
	  @param parent   pointer to the parent widget
	  @param name     pointer to the name of this widget
	  @param valid    pointer to set of valid characters 
  */
  KRestrictedLine( QWidget *parent=0, const char *name=0, const char *valid=0 );

  /// Destructor
  ~KRestrictedLine();

  /** All characters in *valid are treated as valid characters
	*/
  void	setValidChars(const char *valid);
  //@}
  
  signals:
  /** This signal is emitted, if an invalid character was typed
	*/
  void	invalidChar(int);

protected:
  /// Handles the special keys
  void	keyPressEvent( QKeyEvent *e );

private:
  /// QString of valid characters for this line
  QString	qsValidChars;
};

#endif // KRESTRICTEDLINE_H



