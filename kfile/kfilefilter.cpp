#include "kfilefilter.h"
#include <qstrlist.h>
#include <kapp.h>
#include <klocale.h>

KFileFilter::KFileFilter( QWidget *parent, const char *name)
    : QComboBox(true, parent, name)
{
    setInsertionPolicy(NoInsertion);
    connect(this, SIGNAL(activated(const QString &)), SLOT(changed(const QString &)));
}

KFileFilter::~KFileFilter()
{
}

void KFileFilter::changed( const QString & )
{
    emit filterChanged();
}

void KFileFilter::setFilter(const QString& filter)
{
    filters.clear();

    if (!filter.isEmpty()) {
	QString tmp = filter;
	int index = tmp.find('\n');
	while (index > 0) {
	    filters.append(tmp.left(index));
	    tmp = tmp.mid(index + 1);
	    index = tmp.find('\n');
	}
	filters.append(tmp);
    } else
	filters.append(i18n("*|All Files"));

    clear();
    QStringList::ConstIterator it;
    for (it = filters.begin(); it != filters.end(); it++) {
	int tab = (*it).find('|');
	insertItem((tab < 0) ? *it :
		   (*it).mid(tab + 1));
    }
}

QString KFileFilter::currentFilter() const
{
    QString filter = currentText();
    if (filter == text(currentItem()))
	filter = *filters.at(currentItem());
    
    int tab = filter.find('|');
    if (tab < 0)
	return filter;
    else
	return filter.left(tab);
}

#include "kfilefilter.moc"

