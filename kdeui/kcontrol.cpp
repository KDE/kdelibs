/*
  kcontrol - Base for KDE Control Applications

  written 1997 by Matthias Hoelzer
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
   
  */


#include <klocale.h>


#include "kcontrol.h"
#include "kcontrol.moc"


KControlDialog::KControlDialog()
  : QTabDialog(0, 0, FALSE)
{
  // Create help button
  helpBtn = new QPushButton(klocale->translate("Help"), this);
  helpBtn->resize(helpBtn->sizeHint());
  helpBtn->move(7, height() - helpBtn->height() - 6);

  // set the default buttons
  setOKButton(klocale->translate("OK"));
  setApplyButton(klocale->translate("Apply"));
  setCancelButton(klocale->translate("Cancel"));
}


void KControlDialog::done(int result)
{
  hide();
  setResult(result);
  kapp->quit();
}


void KControlDialog::resizeEvent(QResizeEvent *event)
{
  QTabDialog::resizeEvent(event);

  if (helpBtn)
    helpBtn->move(7, height() - helpBtn->height() - 6);
}


KControlApplication::KControlApplication(int &argc, char **argv, const char *name)
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
  // Stephan: this is not possible (or better: not practical)
  // dialog->setCaption(klocale->translate(title));

  // connect the buttons
  connect(dialog, SIGNAL(applyButtonPressed()), this, SLOT(apply()));
  connect(dialog->helpBtn, SIGNAL(pressed()), this, SLOT(help()));

  // set dialog as main widget
  setMainWidget(dialog);

  // parse the command line parameters, if any
  if (argc > 1)
    {
      pages = new QStrList();
      if (pages)
	{
	  for (int i=1; i<argc; i++)
	    pages->append(argv[i]);
	}
    }

  // set the default size
  dialog->resize(480, 480);
  
  // show the dialog
  dialog->show();
}


void KControlApplication::setTitle(const char *title)
{
    if (dialog)
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
      helpNames.append(help_name.data());
    }
}


void KControlApplication::help()
{
  QString name("index");
  
  if (dialog)
    name = helpNames.at(dialog->tabBar()->currentTab());

  kapp->invokeHTMLHelp(QString("kcontrol/")+kapp->appName()+"/"+name, "");
}
