#include "kdocument/view.h"
#include "kdocument/document.h"
#include "kdocument/factory.h"


KDocument::Factory *KDocument::factory(KParts::Factory *_factory) {
	return dynamic_cast<KDocument::Factory*>(_factory);
}
