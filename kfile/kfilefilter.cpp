
#include "kfilefilter.h"
#include <qstrlist.h>
#include <kapp.h>

KFileFilter::KFileFilter( QWidget *parent, const char *name)
    : QComboBox(true, parent, name), filters(0)
{
    setInsertionPolicy(NoInsertion);
    connect(this, SIGNAL(activated(const char*)), SLOT(changed(const char*)));
}

KFileFilter::~KFileFilter()
{
    delete filters;
}

void KFileFilter::changed( const char * )
{
    emit filterChanged();
}

void KFileFilter::setFilter(const char *filter)
{
    delete filters;
    filters = new QStrList( true );
    if (filter) {
	QString tmp = filter; // deep copy
	char *g = strtok(tmp.data(), "\n");
	while (g) {
	    filters->append(g);
	    g = strtok(0, "\n");
	}
    } else
	filters->append(i18n("*|All Files"));

    clear();
    QString name;
    for (const char *item = filters->first(); item; 
	 item = filters->next()) {
	name = item;
	int tab = name.find('|');
	insertItem((tab < 0) ? name :
		   name.mid(tab + 1, name.length() - tab));
    }
}

QString KFileFilter::currentFilter() 
{
    QString filter = currentText();
    if (filter == text(currentItem()))
	filter = filters->at(currentItem());
    
    int tab = filter.find('|');
    if (tab < 0)
	return filter;
    else
	return filter.left(tab);
}

#include "kfilefilter.moc"

