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

//
// 1999-20-10 Espen Sand <espen@kde.org>:  Some layout modifications.
// Using KDialogBase and KTextBrowser.
//


#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qradiobutton.h>

#include <kapp.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <ktextbrowser.h>

#include "kfiledialogconf.h"
#include "config-kfile.h"



enum Fields { B_STATUSLINE=10, B_FILTER, B_SINGLECLICK, B_HIDDEN, 
	      B_LISTLABELS, B_SHORTVIEW, B_DETAILVIEW, B_MIX, B_KEEPDIR };


KFileDialogConfigureDlg::KFileDialogConfigureDlg( QWidget *parent, 
						  const char *name )
  :KDialogBase( Tabbed, i18n("Configure file selector"), Ok|Cancel, Ok,
	        parent, name, true, false )
{
  setupConfigPage( i18n("Look and Feel") );
  setupAboutPage( i18n("About") );
}


void KFileDialogConfigureDlg::setupConfigPage( const QString &title )
{
  QFrame *page = addPage( title );
  QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );

  KFileDialogConfigure *kfdc = new KFileDialogConfigure( page );
  topLayout->addWidget( kfdc );  
  topLayout->activate();

  connect( this, SIGNAL(okClicked()), kfdc, SLOT(saveConfiguration()) );
  connect( this, SIGNAL(applyClicked()), kfdc, SLOT(saveConfiguration()) );

  page->setMinimumSize( kfdc->minimumSize() );
}


void KFileDialogConfigureDlg::setupAboutPage( const QString &title )
{
  QFrame *page = addPage( title );
  QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );
  
  const QString text = i18n(""
    "<h3>KDE File Selector</h3>"
    "<hr><br>"
    "Richard Moore <A HREF=\"mailto:rich@kde.org\">"
    "rich@kde.org</A><br>"
    "Stephan Kulow <A HREF=\"mailto:coolo@kde.org\">"
    "coolo@kde.org</A><br>"
    "Daniel Grana <A HREF=\"mailto:grana@ie.iwi.unibe.ch\">"
    "grana@ie.iwi.unibe.ch</A>"    
    "<br><br>with contributions by<br><br>"
    "Mario Weilguni <A HREF=\"mailto:mweilguni@sime.com\">"
    "mweilguni@sime.com</A><br>"
    "Martin Jones <A HREF=\"mailto:mjones@kde.org\">"
    "mjones@kde.org</A> and<br>"
    "Espen Sand <A HREF=\"mailto:espen@kde.org\">"
    "espen@kde.org</A>");
    
  KTextBrowser *browser = new KTextBrowser( page, "browser" );  
  browser->setHScrollBarMode( QScrollView::AlwaysOff );
  browser->setText( text );
  browser->setMinimumHeight( fontMetrics().lineSpacing()*10 );
  browser->setMinimumWidth( 300 );
  topLayout->addWidget( browser, 10 );
  
  topLayout->activate();
  page->setMinimumSize( page->sizeHint() );
}



KFileDialogConfigure::KFileDialogConfigure(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  QString text;
  QVBoxLayout *topLayout = new QVBoxLayout( this, 0, KDialog::spacingHint() );

  //
  // First frame
  //
  text = i18n("Display entries using");
  QButtonGroup *group = new QButtonGroup( text, this );
  topLayout->addWidget( group );

  QVBoxLayout *vbox = new QVBoxLayout( group, KDialog::spacingHint() );
  vbox->addSpacing( fontMetrics().lineSpacing() );

  text = i18n("Short view");
  myShortView = new QRadioButton( text, group );
  group->insert( myShortView, B_SHORTVIEW );
  vbox->addWidget( myShortView );

  text = i18n("Detailed view");
  myDetailView = new QRadioButton( text, group );
  group->insert( myDetailView, B_DETAILVIEW );
  vbox->addWidget( myDetailView );
  vbox->activate();

  int w1 = group->fontMetrics().width( group->title() ) +
    group->fontMetrics().maxWidth()*2;
  int w2 = group->sizeHint().width();
  group->setMinimumWidth( QMAX( w1, w2 ) );

  
  //
  // Second frame
  //
  text = i18n("Miscellaneous options");
  group = new QButtonGroup( text, this );
  topLayout->addWidget( group );

  vbox = new QVBoxLayout( group, KDialog::spacingHint() );
  vbox->addSpacing( fontMetrics().lineSpacing() );

  text = i18n("Show filter entry");
  myShowFilter= new QCheckBox( text, group );
  group->insert( myShowFilter, B_FILTER );
  vbox->addWidget( myShowFilter );

  text = i18n("Show List Labels");
  myShowListLabels= new QCheckBox( text, group );
  group->insert( myShowListLabels, B_LISTLABELS );
  vbox->addWidget( myShowListLabels);

  text = i18n("Show Hidden Files"); 
  myShowHidden= new QCheckBox( text, group );
  group->insert( myShowHidden, B_HIDDEN );
  vbox->addWidget( myShowHidden );

  text = i18n("Show Status Line");
  myShowStatusLine= new QCheckBox( text, group );
  group->insert( myShowStatusLine, B_STATUSLINE );
  vbox->addWidget( myShowStatusLine );

  text = i18n("Use single Click");
  useSingleClick= new QCheckBox( text, group );
  group->insert( useSingleClick, B_SINGLECLICK );
  vbox->addWidget( useSingleClick );

  text = i18n("Mix dirs and files");
  myMixDirsAndFiles = new QCheckBox( text, group );
  group->insert( myMixDirsAndFiles, B_MIX );
  vbox->addWidget( myMixDirsAndFiles );

  text = i18n("Keep dirs first");
  myKeepDirsFirst = new QCheckBox( text, group );
  group->insert( myKeepDirsFirst, B_KEEPDIR );
  vbox->addWidget( myKeepDirsFirst );

  vbox->addStretch();
  vbox->activate();

  w1 = group->fontMetrics().width( group->title() ) +
    group->fontMetrics().maxWidth()*2;
  w2 = group->sizeHint().width();
  group->setMinimumWidth( QMAX( w1, w2 ) );

  topLayout->activate();
  setMinimumSize( sizeHint() );

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

  c = KGlobal::config();
  oldgroup = c->group();
  c->setGroup("KFileDialog Settings");

  QString currentViewStyle = c->readEntry("ViewStyle", DefaultViewStyle);
  if( currentViewStyle == "DetailView" )
  {
    myDetailView->setChecked(true);
  }
  else
  {
    myShortView->setChecked(true);
  }
  myShowHidden->setChecked( 
    c->readBoolEntry("ShowHidden", DefaultShowHidden));
  myShowFilter->setChecked(
    c->readBoolEntry("ShowFilter", DefaultShowFilter));
  myShowListLabels->setChecked(
    c->readBoolEntry("ShowListLabels", DefaultShowListLabels));
  useSingleClick->setChecked(
    c->readBoolEntry("SingleClick", DefaultSingleClick));
  myShowStatusLine->setChecked(
    c->readBoolEntry("ShowStatusLine", DefaultShowStatusLine));
  myMixDirsAndFiles->setChecked(
    c->readBoolEntry("MixDirsAndFiles", DefaultMixDirsAndFiles));
  myKeepDirsFirst->setChecked(
    c->readBoolEntry("KeepDirsFirst", DefaultKeepDirsFirst));

  // Restore the old config group
  c->setGroup(oldgroup);

}

void KFileDialogConfigure::saveConfiguration( void )
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
