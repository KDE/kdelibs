#include "kcupsgeneralpage.h"

#include <qcombobox.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <klocale.h>
#include <kiconloader.h>

void initCombo(QComboBox *cb, global_ppd_file_t *ppd, const QString& keyword, const QPixmap& pix);
void setComboItem(QComboBox *cb, const QString& s);
int findOption(const char *strs[], int n, const QString& txt);
ppd_choice_t* ppdFindOptionChoice(ppd_file_t *ppd, const QString& option, const QString& choice);

//*************************************************************************************************

// default values in case of NULL ppd file
#define	DEFAULT_SIZE	16
static const char*	default_size[] = {
	"Letter", I18N_NOOP("US Letter"),
	"Legal", I18N_NOOP("US Legal"),
	"A4", I18N_NOOP("ISO A4"),
	"Ledger", I18N_NOOP("Ledger"),
	"Tabloid", I18N_NOOP("Tabloid"),
	"Folio", I18N_NOOP("Folio"),
	"Comm10", I18N_NOOP("US #10 Envelope"),
	"DL", I18N_NOOP("ISO DL Envelope")
};
#define	DEFAULT_SOURCE	8
static const char*	default_source[] = {
	"Upper", I18N_NOOP("Upper tray"),
	"Lower", I18N_NOOP("Lower tray"),
	"MultiPurpose", I18N_NOOP("Multi-purpose tray"),
	"LargeCapacity", I18N_NOOP("Large capacity tray")
};
#define	DEFAULT_TYPE	4
static const char*	default_type[] = {
	"Normal", I18N_NOOP("Normal"),
	"Transparency", I18N_NOOP("Transparency")
};

//*************************************************************************************************

KCupsGeneralPage::KCupsGeneralPage(global_ppd_file_t *ppd, IppRequest *req, QWidget *parent, const char *name)
: KCupsGeneralPageBase(parent,name)
{
	ppd_ = ppd;
	request_ = req;

	initialize();

	connect(orientbox_,SIGNAL(clicked(int)),SLOT(orientationChanged(int)));
	connect(nupbox_,SIGNAL(clicked(int)),SLOT(nupChanged(int)));
	connect(duplexbox_,SIGNAL(clicked(int)),SLOT(duplexChanged(int)));
}

KCupsGeneralPage::~KCupsGeneralPage()
{
}

QString KCupsGeneralPage::pageTitle()
{
	return i18n("General");
}

void KCupsGeneralPage::orientationChanged(int ID)
{
	switch (ID)
	{
		case 0: orientpix_->setPixmap(UserIcon("kdeprint_portrait")); break;
		case 1: orientpix_->setPixmap(UserIcon("kdeprint_landscape")); break;
		case 2: orientpix_->setPixmap(UserIcon("kdeprint_revland")); break;
		case 3: orientpix_->setPixmap(UserIcon("kdeprint_revport")); break;
	}
}

void KCupsGeneralPage::duplexChanged(int ID)
{
	switch (ID)
	{
		case 0: duplexpix_->setPixmap(UserIcon("kdeprint_dupl_none")); break;
		case 1: duplexpix_->setPixmap(UserIcon("kdeprint_dupl_long")); break;
		case 2: duplexpix_->setPixmap(UserIcon("kdeprint_dupl_short")); break;
	}
}

void KCupsGeneralPage::nupChanged(int ID)
{
	switch (ID)
	{
		case 0: nuppix_->setPixmap(UserIcon("kdeprint_nup1")); break;
		case 1: nuppix_->setPixmap(UserIcon("kdeprint_nup2")); break;
		case 2: nuppix_->setPixmap(UserIcon("kdeprint_nup4")); break;
	}
}

void KCupsGeneralPage::setOptions(const OptionSet& options)
{
	QString	value;

	if (ppd_ && ppd_->ppd)
	{
		value = options["PageSize"];
		if (!value.isEmpty())
		{
			ppd_choice_t	*ch = ppdFindOptionChoice(ppd_->ppd, QString::fromLatin1("PageSize"), value);
			if (ch) setComboItem(size_, QString::fromLocal8Bit(ch->text));
		}
		value = options["MediaType"];
		if (!value.isEmpty())
		{
			ppd_choice_t	*ch = ppdFindOptionChoice(ppd_->ppd, QString::fromLatin1("MediaType"), value);
			if (ch) setComboItem(type_, QString::fromLocal8Bit(ch->text));
		}
		value = options["InputSlot"];
		if (!value.isEmpty())
		{
			ppd_choice_t	*ch = ppdFindOptionChoice(ppd_->ppd, QString::fromLatin1("InputSlot"), value);
			if (ch) setComboItem(source_, QString::fromLocal8Bit(ch->text));
		}

		value = options["Duplex"];
		int	ID(0);
		if (value == "DuplexNoTumble") ID = 1;
		else if (value == "DuplexTumble") ID = 2;
		duplexbox_->setButton(ID);
		duplexChanged(ID);
	}
	else
	{
		// Try to find "media" option
		value = options["media"];
		if (!value.isEmpty())
		{
			int	index(-1);
			QStringList	l = QStringList::split(',',value,false);
			if (l.count() > 0 && (index=findOption(default_size,DEFAULT_SIZE,l[0])) >= 0)
				size_->setCurrentItem(index);
			if (l.count() > 1 && (index=findOption(default_type,DEFAULT_TYPE,l[1])) >= 0)
				type_->setCurrentItem(index);
			if (l.count() > 2 && (index=findOption(default_source,DEFAULT_SOURCE,l[2])) >= 0)
				source_->setCurrentItem(index);
		}

		// Try to find "sides" option
		value = options["sides"];
		int	ID(0);
		if (value == "two-sided-long-edge") ID = 1;
		else if (value == "two-sided-short-edge") ID = 2;
		duplexbox_->setButton(ID);
		duplexChanged(ID);
	}

	// Banners
	value = options["job-sheets"];
	if (!value.isEmpty())
	{
		QStringList	l = QStringList::split(',',value,false);
		if (l.count() > 0) setComboItem(startbanner_,l[0]);
		if (l.count() > 1) setComboItem(stopbanner_,l[1]);
	}

	// Orientation
	value = options["orientation-requested"];
	if (!value.isEmpty())
	{
		bool	ok;
		int	ID = value.toInt(&ok)-3;
		if (ok)
		{
			orientbox_->setButton(ID);
			orientationChanged(ID);
		}
	}

	// N-up
	value = options["number-up"];
	if (!value.isEmpty())
	{
		bool	ok;
		int	ID = QMIN(value.toInt(&ok)-1,2);
		if (ok)
		{
			nupbox_->setButton(ID);
			nupChanged(ID);
		}
	}
}

void KCupsGeneralPage::getOptions(OptionSet& options, bool incldef)
{
	QString value;

	if (ppd_ && ppd_->ppd)
	{
		ppd_option_t	*opt;
		ppd_choice_t	*ch;
		if ((opt=ppdFindOption(ppd_->ppd,"PageSize")) != NULL && size_->currentItem() >= 0)
		{
			ch = opt->choices + size_->currentItem();
			if (incldef || strcmp(ch->choice,opt->defchoice) != 0) options["PageSize"] = QString::fromLatin1(ch->choice);
			ppdMarkOption(ppd_->ppd,"PageSize",ch->choice);
		}
		if ((opt=ppdFindOption(ppd_->ppd,"MediaType")) != NULL && type_->currentItem() >= 0)
		{
			ch = opt->choices + type_->currentItem();
			if (incldef || strcmp(ch->choice,opt->defchoice) != 0) options["MediaType"] = QString::fromLatin1(ch->choice);
			ppdMarkOption(ppd_->ppd,"MediaType",ch->choice);
		}
		if ((opt=ppdFindOption(ppd_->ppd,"InputSlot")) != NULL && source_->currentItem() >= 0)
		{
			ch = opt->choices + source_->currentItem();
			if (incldef || strcmp(ch->choice,opt->defchoice) != 0) options["InputSlot"] = QString::fromLatin1(ch->choice);
			ppdMarkOption(ppd_->ppd,"InputSlot",ch->choice);
		}

		if (duplexbox_->isEnabled())
		{
			switch (duplexbox_->id(duplexbox_->selected()))
			{
				case 0: value = "None"; break;
				case 1: value = "DuplexNoTumble"; break;
				case 2: value = "DuplexNoTumble"; break;
			}
			options["Duplex"] = value;
			if ((opt=ppdFindOption(ppd_->ppd,"Duplex")) != NULL)
				ppdMarkOption(ppd_->ppd,"Duplex",value.latin1());
		}
	}
	else
	{
		value = QString("%1,%2,%3").arg(default_size[size_->currentItem()*2]).arg(default_type[type_->currentItem()*2]).arg(default_source[source_->currentItem()*2]);
		options["media"] = value;

		if (duplexbox_->isEnabled())
		{
			switch (duplexbox_->id(duplexbox_->selected()))
			{
				case 0: value = "one-sided"; break;
				case 1: value = "two-sided-long-edge"; break;
				case 2: value = "two-sided-short-edge"; break;
			}
			options["sides"] = value;
		}
	}

	value = QString::number(orientbox_->id(orientbox_->selected())+3);
	if (value != "3" || incldef) options["orientation-requested"] = value;
	//options["orientation-requested"] = value;

	if (nupbox_->isEnabled())
	{
		switch (nupbox_->id(nupbox_->selected()))
		{
			case 0: value = "1"; break;
			case 1: value = "2"; break;
			case 2: value = "4"; break;
		}
		if (value != "1" || incldef) options["number-up"] = value;
	}

	if (bannerbox_->isEnabled())
	{
		QStringList	l;
		request_->name("job-sheets-default",l);
		if (incldef || (l.count() == 2 && (l[0] != startbanner_->currentText() || l[1] != stopbanner_->currentText()))
		    || (l.count() == 0 && (startbanner_->currentText() != "none" || stopbanner_->currentText() != "none")))
		{
			value = QString("%1,%2").arg(startbanner_->currentText()).arg(stopbanner_->currentText());
			options["job-sheets"] = value;
		}
	}
}

void KCupsGeneralPage::initialize()
{
	QPixmap	pix;

	if (ppd_ && ppd_->ppd)
	{
		// PageSize
		pix = UserIcon("kdeprint_pagesize");
		initCombo(size_,ppd_,"PageSize",pix);
		// MediaType
		pix = UserIcon("kdeprint_pagesize");
		initCombo(type_,ppd_,"MediaType",pix);
		// PageSize
		pix = UserIcon("kdeprint_printer");
		initCombo(source_,ppd_,"InputSlot",pix);

		// Duplex
		ppd_option_t	*opt;
		if ((opt=ppdFindOption(ppd_->ppd,"Duplex")) == NULL)
			duplexbox_->setEnabled(false);
		else
		{
			QString	ch;
			int	ID(0);
			for (int i=0;i<opt->num_choices;i++)
				if (opt->choices[i].marked)
				{
					ch = QString::fromLatin1(opt->choices[i].choice);
					break;
				}
			if (ch == "DuplexNoTumble") ID = 1;
			else if (ch == "DuplexTumble") ID = 2;
			duplexbox_->setButton(ID);
			duplexChanged(ID);
		}
	}
	else
	{
		// PageSize
		pix = UserIcon("kdeprint_pagesize");
		for (int i=0;i<DEFAULT_SIZE;i+=2)
			size_->insertItem(pix,i18n(default_size[i]));
		// MediaType
		pix = UserIcon("kdeprint_pagesize");
		for (int i=0;i<DEFAULT_TYPE;i+=2)
			type_->insertItem(pix,i18n(default_type[i]));
		// PageSize
		pix = UserIcon("kdeprint_printer");
		for (int i=0;i<DEFAULT_SOURCE;i+=2)
			source_->insertItem(pix,i18n(default_source[i]));
	}

	// Banners
	QStringList	values;
	if (request_->name("job-sheets-supported",values))
	{
		for (QStringList::ConstIterator it = values.begin(); it != values.end(); ++it)
		{
			startbanner_->insertItem(*it);
			stopbanner_->insertItem(*it);
		}
		if (request_->name("job-sheets-default",values))
		{
			if (values.count() > 0) setComboItem(startbanner_,values[0]);
			if (values.count() > 1) setComboItem(stopbanner_,values[1]);
		}
	}
	else
		bannerbox_->setEnabled(false);

	// some default values
	portrait_->setChecked(true);
	orientationChanged(0);
	nup1_->setChecked(true);
	nupChanged(0);
}

//*************************************************************************************************

void initCombo(QComboBox *cb, global_ppd_file_t *ppd, const QString& keyword, const QPixmap& pix)
{
	ppd_option_t*	option = ppdFindOption(ppd->ppd,keyword.latin1());
	cb->clear();
	if (option)
	{
		int	selected_choice(-1);
		for (int i=0;i<option->num_choices;i++)
		{
			ppd_choice_t*	choice = option->choices+i;
			cb->insertItem(pix,QString::fromLocal8Bit(choice->text));
			if (choice->marked) selected_choice = i;
		}
		cb->setCurrentItem((selected_choice != -1 ? selected_choice : 0));
	}
	else
		cb->setEnabled(false);
}

void setComboItem(QComboBox *cb, const QString& txt)
{
	for (int i=0;i<cb->count();i++)
		if (cb->text(i) == txt)
		{
			cb->setCurrentItem(i);
			break;
		}
}

int findOption(const char *strs[], int n, const QString& txt)
{
	for (int i=0;i<n;i+=2)
		if (txt == strs[i]) return (i/2);
	return (-1);
}

ppd_choice_t* ppdFindOptionChoice(ppd_file_t *ppd, const QString& option, const QString& choice)
{
	ppd_option_t	*opt = ppdFindOption(ppd, option.latin1());
	if (opt)
		return ppdFindChoice(opt,choice.latin1());
	return NULL;
}
#include "kcupsgeneralpage.moc"
