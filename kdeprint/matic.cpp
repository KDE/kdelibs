#include "matic.h"

MHash	*main_hash = 0;

void MHash::addHash(MHash *hash)
{
	type = MHash::Hash;
	hashes.setAutoDelete(true);
	if (hash)
	{
		hashes.append(hash);
		if (hash->type == MHash::Hash && !hash->name)
		{
			QListIterator<MHash>	it(hash->hashes);
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
		QListIterator<MHash>	it(h->hashes);
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

void cleanHash()
{
	if (main_hash)
	{
		delete main_hash;
		main_hash = 0;
	}
}

MaticBlock* loadMaticData(const char *s)
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
