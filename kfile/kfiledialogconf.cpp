/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
    
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

#include <qlayout.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include "kfiledialogconf.h"
#include "config-kfile.h"
#include <kapp.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>

enum Fields { B_STATUSLINE=10, B_FILTER, B_SINGLECLICK, B_HIDDEN, 
	       B_LISTLABELS, B_SHORTVIEW, B_DETAILVIEW, B_MIX, B_KEEPDIR };


KFileDialogConfigureDlg::KFileDialogConfigureDlg(QWidget *parent, 
						 const char *name)
  : QTabDialog(parent, name, true)
{
  KFileDialogConfigure *kfdc= new KFileDialogConfigure(this);
  addTab(kfdc, i18n("Look and Feel"));

  QLabel *label= new QLabel(i18n("KDE File Selector by:\n"
				 "\n"
				 "Richard Moore <rich@kde.org>\n"
				 "Stephan Kulow <coolo@kde.org>\n"
				 "and Daniel Grana <grana@ie.iwi.unibe.ch>\n"
				 "\n"
				 "with contributions by\n"
				 "\n"
				 "Mario Weilguni <mweilguni@sime.com>\n"
				 "and Martin Jones <mjones@kde.org>"),
			    this);
  label->setAlignment(AlignCenter);
  addTab(label, i18n("About"));
  
  setCancelButton(i18n("Cancel"));
  // setApplyButton(i18n("Apply"));
  setOKButton(i18n("OK"));
  connect( this, SIGNAL(applyButtonPressed()), kfdc, 
	   SLOT(saveConfiguration()) );
};

KFileDialogConfigure::KFileDialogConfigure(QWidget *parent, 
					   const char *name)
  : QWidget(parent, name)
{
  QVBoxLayout *choices = new QVBoxLayout(this, 5);
  
  QButtonGroup *group= new QButtonGroup( i18n("View style"), this);
  group->setExclusive(true);
  QVBoxLayout *l1 = new QVBoxLayout(group, 5);  
  l1->addSpacing(10);
  myShortView= new QRadioButton( i18n("Show Short View"), group);
  group->insert( myShortView, B_SHORTVIEW );
  myShortView->setMinimumSize( myShortView->sizeHint() );
  l1->addWidget( myShortView, 0 );
  l1->addSpacing(10);
  myDetailView= new QRadioButton( i18n("Show Detail View"), group);
  group->insert( myDetailView, B_DETAILVIEW );
  myDetailView->setMinimumSize( myDetailView->sizeHint() );
  l1->addWidget( myDetailView, 0 );
  choices->addWidget(group, 2);
  
  //
  //
  QButtonGroup *group2= new QButtonGroup(i18n("Misc"), this);
  choices->addSpacing(15);
  choices->addWidget( group2, 5 );
  
  QVBoxLayout *l2 = new QVBoxLayout(group2, 5);  
  l2->addSpacing(10);
  myShowFilter= new QCheckBox(i18n("Show Filter"), group2);
  group2->insert( myShowFilter, B_FILTER );
  myShowFilter->setMinimumSize( myShowFilter->sizeHint() );
  l2->addWidget( myShowFilter, 0 );
  l2->addSpacing(10);
  myShowListLabels= new QCheckBox(i18n("Show List Labels"), group2);
  group2->insert( myShowListLabels, B_LISTLABELS );
  myShowListLabels->setMinimumSize( myShowListLabels->sizeHint() );
  l2->addWidget( myShowListLabels, 0 );
  l2->addSpacing(10);
  myShowHidden= new QCheckBox( i18n("Show Hidden"), group2);
  group2->insert( myShowHidden, B_HIDDEN );
  myShowHidden->setMinimumSize( myShowHidden->sizeHint() );
  l2->addWidget( myShowHidden, 0 );
  l2->addSpacing(10);
  myShowStatusLine= new QCheckBox( i18n("Show Status Line"), group2);
  group2->insert( myShowStatusLine, B_STATUSLINE );
  myShowStatusLine->setMinimumSize( myShowStatusLine->sizeHint() );
  l2->addWidget( myShowStatusLine, 0 );
  l2->addSpacing(10);
  useSingleClick= new QCheckBox( i18n("Use single Click"), group2);
  group2->insert( useSingleClick, B_SINGLECLICK );
  useSingleClick->setMinimumSize( useSingleClick->sizeHint() );
  l2->addWidget( useSingleClick, 0 );
  l2->addSpacing(10);
  myMixDirsAndFiles = new QCheckBox( i18n("Mix dirs and files"), group2);
  group2->insert( myMixDirsAndFiles, B_MIX );
  myMixDirsAndFiles->setMinimumSize( myMixDirsAndFiles->sizeHint() );
  l2->addWidget( myMixDirsAndFiles, 0 );
  l2->addSpacing(10);
  myKeepDirsFirst = new QCheckBox( i18n("Keep dirs first"), group2);
  group2->insert( myKeepDirsFirst, B_KEEPDIR );
  myKeepDirsFirst->setMinimumSize( myKeepDirsFirst->sizeHint() );
  l2->addWidget( myKeepDirsFirst, 0 );

  group2->setMinimumSize( group2->childrenRect().size() );
  l2->activate();
  group->setMinimumSize( group->childrenRect().size() );
  l1->activate();
  choices->activate();
  
/*
  horizontal->addSpacing( 10 );
  horizontal->addWidget( group2, 5 );

   QBoxLayout *horizontalx= new QBoxLayout(QBoxLayout::LeftToRight);
   QLineEdit *findPathEdit;
   findPathEdit= new QLineEdit(this, "findpathedit");
   QLabel *tmpLabel;
   tmpLabel= new QLabel(findPathEdit, i18n("&Find command:"), this);
   tmpLabel->adjustSize();
   tmpLabel->setAlignment(AlignCenter);
   tmpLabel->setMinimumSize(tmpLabel->size());
   tmpLabel->setMaximumHeight(tmpLabel->height());
   findPathEdit->setMinimumHeight(tmpLabel->size().height()+8);
   findPathEdit->setMaximumHeight(tmpLabel->size().height()+8);

   vertical->addLayout(horizontalx);
   horizontal->addWidget(tmpLabel, 1);
   horizontal->addWidget(findPathEdit, 10);
   vertical->activate();
*/

  // Now read the current settings
  KConfig *c;
  QString oldgroup;
  QString dirview;

  c= KGlobal::config();
  oldgroup= c->group();
  c->setGroup("KFileDialog Settings");

  QString currentViewStyle = c->readEntry("ViewStyle", DefaultViewStyle);
  if ( currentViewStyle == "DetailView" )
    myDetailView->setChecked(true);
  else
    myShortView->setChecked(true);
  myShowHidden->setChecked(c->readBoolEntry("ShowHidden", DefaultShowHidden));
  myShowFilter->setChecked(c->readBoolEntry("ShowFilter", DefaultShowFilter));
  myShowListLabels->setChecked(c->readBoolEntry("ShowListLabels", 
						DefaultShowListLabels));
  useSingleClick->setChecked(c->readBoolEntry("SingleClick", 
					      DefaultSingleClick));
  myShowStatusLine->setChecked(c->readBoolEntry("ShowStatusLine", 
						DefaultShowStatusLine));
  myMixDirsAndFiles->setChecked(c->readBoolEntry("MixDirsAndFiles", 
						 DefaultMixDirsAndFiles));
  myKeepDirsFirst->setChecked(c->readBoolEntry("KeepDirsFirst", 
					       DefaultKeepDirsFirst));

  // Restore the old config group
  c->setGroup(oldgroup);

}

void 
KFileDialogConfigure::saveConfiguration()
{
  // Now read the current settings
  KConfig *c;
  QString oldgroup;
  QString dirview;

  c= KGlobal::config();
  oldgroup= c->group();
  c->setGroup("KFileDialog Settings");

  c->writeEntry("ViewStyle", 
		myDetailView->isChecked() ? "DetailView" : "ShortView", 
		true, true);
  
  c->writeEntry("ShowHidden", myShowHidden->isChecked(), true, true);
  c->writeEntry("ShowFilter", myShowFilter->isChecked(), true, true);
  c->writeEntry("ShowListLabels",myShowListLabels->isChecked(), true, true );
  c->writeEntry("SingleClick", useSingleClick->isChecked(), true, true);
  c->writeEntry("ShowStatusLine", myShowStatusLine->isChecked(), true, true);
  c->writeEntry("MixDirsAndFiles",myMixDirsAndFiles->isChecked(), true, true);
  c->writeEntry("KeepDirsFirst", myKeepDirsFirst->isChecked(), true, true);
  c->sync();
  // Restore the old config group
  c->setGroup(oldgroup);
}

#include "kfiledialogconf.moc"
