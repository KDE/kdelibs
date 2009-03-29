/***************************************************************************
 *   Copyright (C) 2009 by Rahman Duran <rahman.duran@gmail.com>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#ifndef KDE_TERMINAL_INTERFACE_V2_H
#define KDE_TERMINAL_INTERFACE_V2_H

/**
 * TerminalInterfaceV2 is an extension to TerminalInterface that implemented
 * by KonsolePart to allow developers access to the KonsolePart in ways that are not
 * possible through the normal KPart interface.
 *
 * @author Rahman Duran  <rahman.duran@gmail.com>
 */
#include <kde_terminal_interface.h>

class TerminalInterfaceV2: public TerminalInterface
{
public:
  virtual ~TerminalInterfaceV2(){}

  /**
   * Return terminal PID
   */
  virtual int terminalProcessId() = 0;

  /**
   * Return foregound PID, If there is no foreground process running, returns -1
   */
  virtual int foregroundProcessId() = 0;

  /**
   * Returns sub process name. If there is no sub process running, returns empty QString
   */
  virtual QString foregroundProcessName() = 0;


};

Q_DECLARE_INTERFACE(TerminalInterfaceV2, "org.kde.TerminalInterfaceV2")
#endif
