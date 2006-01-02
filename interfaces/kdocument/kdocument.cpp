#include "kdocument/view.h"
#include "kdocument/document.h"
#include "kdocument/factory.h"

namespace KDocument {
	/// Contains the current highest document number.
	static int globalDocumentNumber = 0;
	class Document::Private {
	};
}

KDocument::Document::Document(QObject *parent):
	KParts::ReadWritePart(parent),
	d(0),
	m_documentNumber (++KDocument::globalDocumentNumber) {
}

int KDocument::Document::documentNumber () const
{
  return m_documentNumber;
}

KDocument::Factory *KDocument::factory(KParts::Factory *_factory) {
	return dynamic_cast<KDocument::Factory*>(_factory);
}

#include "document.moc"
#include "view.moc"