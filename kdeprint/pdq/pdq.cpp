#include "pdq.h"

#include <stdio.h>

BlockT::BlockT(int t)
: type_(t)
{
	name_ = 0;
	values_ = 0;
	blocks_ = 0;
}

BlockT::~BlockT()
{
	if (name_) delete name_;
	if (values_) delete values_;
	if (blocks_) delete blocks_;
}

void BlockT::addValue(QString *str)
{
	if (!values_)
	{
		values_ = new QList<QString>;
		values_->setAutoDelete(true);
	}
	values_->append(str);
}

void BlockT::addBlock(BlockT *blk)
{
	if (!blocks_)
	{
		blocks_ = new QList<BlockT>;
		blocks_->setAutoDelete(true);
	}
	blocks_->append(blk);
}

void BlockT::dump()
{
	switch (type_)
	{
		case String:
			printf("%s%s = %s\n",prefix_.latin1(),name_->latin1(),values_->first()->latin1());
			break;
		case StringList:
			printf("%s%s = ",prefix_.latin1(),name_->latin1());
			if (values_)
				for (values_->first();values_->current();values_->next())
				{
					printf("%s",values_->current()->latin1());
					if (values_->current() != values_->getLast()) printf(", ");
				}
			printf("\n");
			break;
		case PairList:
			printf("%s%s = ",prefix_.latin1(),name_->latin1());
			if (values_)
				for (values_->first();values_->current();values_->next())
				{
					printf("%s <- ",values_->current()->latin1());
					printf("%s",values_->next()->latin1());
					if (values_->current() != values_->getLast()) printf(", ");
				}
			printf("\n");
			break;
		case Block:
			if (values_) printf("%s%s \"%s\"\n",prefix_.latin1(),name_->latin1(),values_->first()->latin1());
			else printf("%s%s\n",prefix_.latin1(),name_->latin1());
			if (blocks_)
			{
				prefix_.append("   ");
				for (blocks_->first(); blocks_->current(); blocks_->next())
					blocks_->current()->dump();
				prefix_.truncate(prefix_.length()-3);
			}
			break;
		default:
			break;
	}
}
