#include <qtabbar.h>
#include "kmdidocumentviewtabwidget.h"

KMdiDocumentViewTabWidget::KMdiDocumentViewTabWidget(QWidget* parent, const char* name):KTabWidget(parent,name) {
//	QTabBar *bar=tabBar();
	tabBar()->hide();
	setHoverCloseButton(true);
}
	
KMdiDocumentViewTabWidget::~KMdiDocumentViewTabWidget() {
}

void KMdiDocumentViewTabWidget::addTab ( QWidget * child, const QString & label ) {
	KTabWidget::addTab(child,label);
	if (count()>1) tabBar()->show();
}

void KMdiDocumentViewTabWidget::addTab ( QWidget * child, const QIconSet & iconset, const QString & label ) {
	KTabWidget::addTab(child,iconset,label);
	if (count()>1) tabBar()->show();
}

void KMdiDocumentViewTabWidget::addTab ( QWidget * child, QTab * tab ) {
	KTabWidget::addTab(child,tab);
	if (count()>1) tabBar()->show();
}

void KMdiDocumentViewTabWidget::insertTab ( QWidget * child, const QString & label, int index) {
	KTabWidget::insertTab(child,label,index);
	if (count()>1) tabBar()->show();
	
}

void KMdiDocumentViewTabWidget::insertTab ( QWidget * child, const QIconSet & iconset, const QString & label, int index ) {
	KTabWidget::insertTab(child,iconset,label,index);
	if (count()>1) tabBar()->show();
}

void KMdiDocumentViewTabWidget::insertTab ( QWidget * child, QTab * tab, int index) {
	KTabWidget::insertTab(child,tab,index);
	if (count()>1) tabBar()->show();
}

void KMdiDocumentViewTabWidget::removePage ( QWidget * w ) {
	KTabWidget::removePage(w);
	if (count()<2) tabBar()->hide();
}

#ifndef NO_INCLUDE_MOCFILES
#include "kmdidocumentviewtabwidget.moc"
#endif
