/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef __filter_h__
#define __filter_h__

class KIOFilter
{
public:
  /**
   * If the arguments parameter is specified, it must be a null
   * terminated list. ie. the last element of the array should 
   * be null. 
   */
  KIOFilter( const char *_cmd, const char **arguments=0 );
  virtual ~KIOFilter();
 
  int pid() { return m_pid; }
 
  bool send( void *_p, int _len );
  bool finish();
  
protected:
  virtual void emitData( void *_p, int _len );
  
private:
  int buildPipe( int *_recv, int *_send );
  
  int m_pid;

  int recv_in;
  int recv_out;
  int send_in;
  int send_out;
};

#endif
