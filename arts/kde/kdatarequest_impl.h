	/*

	Copyright (C) 2002 Nikolas Zimmermann <wildfox@kde.org>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.
  
	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public License
	along with this library; see the file COPYING.LIB.  If not, write to
	the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	Boston, MA 02110-1301, USA.

	*/

#ifndef KDATAREQUEST_IMPL
#define KDATAREQUEST_IMPL

#include <qobject.h>
#include "artskde.h"
#include "stdsynthmodule.h" 

namespace Arts
{

class KDataRequest_impl : public QObject, virtual public KDataRequest_skel,
						  virtual public StdSynthModule
{
Q_OBJECT
public:
	KDataRequest_impl();
	~KDataRequest_impl();
	
	void streamInit();	
	void streamEnd();

	void calculateBlock(unsigned long samples);
	
	void goOn();
	
signals:
	void newData();
	void newBlockSize(long blockSize);
	void newBlockPointer(long blockPointer);

private:
	long m_lastBlockSize;
	unsigned char *m_outBlock;
};

}

#endif
