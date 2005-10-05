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

#include <kdebug.h>
#include "flowsystem.h"
#include "convert.h"
#include "kdatarequest_impl.moc"

using namespace Arts;

KDataRequest_impl::KDataRequest_impl()
{
	m_lastBlockSize = 0;
	m_outBlock = 0;
}

KDataRequest_impl::~KDataRequest_impl()
{
	delete []m_outBlock;
}

void KDataRequest_impl::streamInit()
{
}

void KDataRequest_impl::streamEnd()
{
}

void KDataRequest_impl::calculateBlock(unsigned long samples)
{
	long blockSize = samples * 4;

	if(m_lastBlockSize != blockSize)
	{
		delete []m_outBlock;
		emit newBlockSize(blockSize);
	
		m_outBlock = new unsigned char[blockSize]; // 2 channels, 16 bit
	
		emit newBlockPointer((long) m_outBlock);

		m_lastBlockSize = blockSize;
	}

	convert_stereo_2float_i16le(samples, left, right, m_outBlock);
	emit newData();
}

/*
 * this is the most tricky part here - since we will run in a context
 * where no audio hardware will play the "give me more data role",
 * we'll have to request things ourselves (requireFlow() tells the
 * flowsystem that more signal flow should happen, so that
 * calculateBlock will get called
 */
void KDataRequest_impl::goOn()
{
	_node()->requireFlow();
}

REGISTER_IMPLEMENTATION(KDataRequest_impl);
