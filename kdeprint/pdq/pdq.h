#ifndef PDQ_H
#define	PDQ_H

#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>

class BlockT
{
public:
	enum BlockType { String, StringList, PairList, Block, Null };

	BlockT(int type);
	~BlockT();
	void addValue(QString*);
	void addBlock(BlockT*);
	void dump();

	int		type_;
	QString		*name_;
	QList<QString>	*values_;
	QList<BlockT>	*blocks_;

	static QString	prefix_;
};

BlockT* pdqParse(const QString& filename, bool expandincl = true);
void initPdqFlex(const QString& filename, bool expandincl = true);

#endif
