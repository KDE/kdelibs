/*
  kcontrol - Base for KDE Control Applications

  written 1997 by Matthias Hoelzer

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/


#include <klocale.h>
#include "kcontrol.h"

KControlDialog::KControlDialog()
  : QTabDialog(0, 0, FALSE)
{
  // set the default buttons
  setOkButton(i18n("&OK"));
  setApplyButton(i18n("&Apply"));
  setCancelButton(i18n("&Cancel"));
  setDefaultButton(i18n("&Default"));
  setHelpButton(i18n("&Help"));
}

void KControlDialog::done(int result)
{
  hide();
  setResult(result);
  kapp->quit();
}

KControlApplication::KControlApplication(int &argc, char **argv, const QCString &name)
  : KApplication(argc, argv, name)
{
  dialog = 0;
  pages = 0;

  // called to initialize?
  if (argc == 2 && strcmp("-init", argv[1]) == 0)
    {
      init();
      justInit = TRUE;
      return;
    }

  // run the setup dialog
  justInit = FALSE;

  // create setup dialog
  dialog = new KControlDialog();
  if (!dialog)
    return;

  // connect the buttons
  connect(dialog, SIGNAL(applyButtonPressed()), this, SLOT(apply()));
  connect(dialog, SIGNAL(helpButtonPressed()), this, SLOT(help()));
  connect(dialog, SIGNAL(defaultButtonPressed()), this, SLOT(defaultValues()));
  
  // set dialog as main widget
  setMainWidget(dialog);
  
  // detect, if swallowing
  int start=1;
  if (argc >= 3 && strcmp(argv[1],"-swallow") == 0)
    {
      swallowCaption = argv[2];
      start = 3;
    }

  dialog->setCaption( swallowCaption );

  // parse the command line parameters, if any
  if (argc > start)
    {
      pages = new QStrList();
      if (pages)
	{
	  for (int i=start; i<argc; i++)
	    pages->append(argv[i]);
	}
    }

}

void KControlApplication::setTitle(const QString& title)
{
  if (dialog && swallowCaption.isEmpty())
      dialog->setCaption(title);
}

KControlApplication::~KControlApplication()
{
  if (dialog)
    delete dialog;
  if (pages)
    delete pages;
}


void KControlApplication::addPage(QWidget *page, const QString &name, const QString &help_name)
{
  if (dialog)
    {
      dialog->addTab(page, name);
      helpNames.append(help_name.ascii());
      // set the default size
      dialog->resize(dialog->sizeHint());
    }
}


void KControlApplication::help()
{
  QString name("index.html");

  if (dialog)
    name = helpNames.at(dialog->tabBar()->currentTab());

  kapp->invokeHTMLHelp(QString("kcontrol/")+kapp->name()+"/"+name, "");
}

#include "kcontrol.moc"
