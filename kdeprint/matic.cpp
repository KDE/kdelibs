/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  $Id$
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "matic.h"

MHash	*main_hash;

void MHash::addHash(MHash *hash)
{
	type = MHash::Hash;
	hashes.setAutoDelete(true);
	if (hash)
	{
		hashes.append(hash);
		if (hash->type == MHash::Hash && !hash->name)
		{
			QPtrListIterator<MHash>	it(hash->hashes);
			for (;it.current();++it)
				if (it.current()->type == MHash::String && it.current()->name && *(it.current()->name) == "name")
				{
					if (it.current()->value)
						hash->name = new QString(*(it.current()->value));
					break;
				}
		}
	}
}

//***********************************************************************************************************

MaticBlock::MaticBlock()
{
	m_blocks.setAutoDelete(true);
}

void MaticBlock::readMHash(MHash *h)
{
	if (h->type == MHash::Hash)
	{
		if (h->name) m_name = *(h->name);
		QPtrListIterator<MHash>	it(h->hashes);
		for (;it.current();++it)
		{
			switch (it.current()->type)
			{
			   case MHash::String:
				if (it.current()->name)
					if (it.current()->value)
						m_args[*(it.current()->name)] = *(it.current()->value);
					else
						m_args[*(it.current()->name)] = QString::null;
				break;
			   case MHash::Hash:
			   {
				MaticBlock	*blk = new MaticBlock();
				blk->readMHash(it.current());
				if (!blk->m_name.isEmpty()) m_blocks.insert(blk->m_name,blk);
				break;
			   }
			}
		}
	}
}

static void cleanHash()
{
	if (main_hash)
	{
		delete main_hash;
		main_hash = 0;
	}
}

static MaticBlock* loadMaticData(const char *s)
{
	// just to be sure
	cleanHash();

	initMaticParser(s);
	maticparse();

	if (main_hash)
	{
		MaticBlock	*mainblk = new MaticBlock();
		mainblk->readMHash(main_hash);
		cleanHash();

		return mainblk;
	}
	return NULL;
}
