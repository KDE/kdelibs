/*
 * variables.c: Implementation of the variable storage and lookup
 *
 * Reference:
 *   http://www.w3.org/TR/1999/REC-xslt-19991116
 *
 * See Copyright for the status of this software.
 *
 * daniel@veillard.com
 */

#include "libxslt.h"

#include <string.h>

#include <libxml/xmlmemory.h>
#include <libxml/tree.h>
#include <libxml/valid.h>
#include <libxml/hash.h>
#include <libxml/xmlerror.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/parserInternals.h>
#include "xslt.h"
#include "xsltInternals.h"
#include "xsltutils.h"
#include "variables.h"
#include "transform.h"
#include "imports.h"
#include "preproc.h"

#ifdef WITH_XSLT_DEBUG
#define WITH_XSLT_DEBUG_VARIABLE
#endif

/************************************************************************
 *									*
 *			Module interfaces				*
 *									*
 ************************************************************************/

/**
 * xsltNewStackElem:
 *
 * Create a new XSLT ParserContext
 *
 * Returns the newly allocated xsltParserStackElem or NULL in case of error
 */
static xsltStackElemPtr
xsltNewStackElem(void) {
    xsltStackElemPtr cur;

    cur = (xsltStackElemPtr) xmlMalloc(sizeof(xsltStackElem));
    if (cur == NULL) {
	xsltPrintErrorContext(NULL, NULL, NULL);
        xsltGenericError(xsltGenericErrorContext,
		"xsltNewStackElem : malloc failed\n");
	return(NULL);
    }
    cur->computed = 0;
    cur->name = NULL;
    cur->nameURI = NULL;
    cur->select = NULL;
    cur->tree = NULL;
    cur->value = NULL;
    cur->comp = NULL;
    return(cur);
}

/**
 * xsltCopyStackElem:
 * @elem:  an XSLT stack element
 *
 * Makes a copy of the stack element
 *
 * Returns the copy of NULL
 */
static xsltStackElemPtr
xsltCopyStackElem(xsltStackElemPtr elem) {
    xsltStackElemPtr cur;

    cur = (xsltStackElemPtr) xmlMalloc(sizeof(xsltStackElem));
    if (cur == NULL) {
	xsltPrintErrorContext(NULL, NULL, NULL);
        xsltGenericError(xsltGenericErrorContext,
		"xsltCopyStackElem : malloc failed\n");
	return(NULL);
    }
    cur->name = xmlStrdup(elem->name);
    cur->nameURI = xmlStrdup(elem->nameURI);
    cur->select = xmlStrdup(elem->select);
    cur->tree = elem->tree;
    cur->comp = elem->comp;
    cur->computed = 0;
    cur->value = NULL;
    return(cur);
}

/**
 * xsltFreeStackElem:
 * @elem:  an XSLT stack element
 *
 * Free up the memory allocated by @elem
 */
static void
xsltFreeStackElem(xsltStackElemPtr elem) {
    if (elem == NULL)
	return;
    if (elem->name != NULL)
	xmlFree(elem->name);
    if (elem->nameURI != NULL)
	xmlFree(elem->nameURI);
    if (elem->select != NULL)
	xmlFree(elem->select);
    if (elem->value != NULL)
	xmlXPathFreeObject(elem->value);

    xmlFree(elem);
}

/**
 * xsltFreeStackElemList:
 * @elem:  an XSLT stack element
 *
 * Free up the memory allocated by @elem
 */
void
xsltFreeStackElemList(xsltStackElemPtr elem) {
    xsltStackElemPtr next;

    while(elem != NULL) {
	next = elem->next;
	xsltFreeStackElem(elem);
	elem = next;
    }
}

/**
 * xsltCheckStackElem:
 * @ctxt:  xn XSLT transformation context
 * @name:  the variable name
 * @nameURI:  the variable namespace URI
 *
 * check wether the variable or param is already defined
 *
 * Returns 1 if present, 0 if not, -1 in case of failure.
 */
static int
xsltCheckStackElem(xsltTransformContextPtr ctxt, const xmlChar *name,
	           const xmlChar *nameURI) {
    xsltStackElemPtr cur;

    if ((ctxt == NULL) || (name == NULL))
	return(-1);

    cur = ctxt->vars;
    while (cur != NULL) {
	if (xmlStrEqual(name, cur->name)) {
	    if (((nameURI == NULL) && (cur->nameURI == NULL)) ||
		((nameURI != NULL) && (cur->nameURI != NULL) &&
		 (xmlStrEqual(nameURI, cur->nameURI)))) {
		return(1);
	    }
	}
	cur = cur->next;
    }
    return(0);
}

/**
 * xsltAddStackElem:
 * @ctxt:  xn XSLT transformation context
 * @elem:  a stack element
 *
 * add a new element at this level of the stack.
 *
 * Returns 0 in case of success, -1 in case of failure.
 */
static int
xsltAddStackElem(xsltTransformContextPtr ctxt, xsltStackElemPtr elem) {
    if ((ctxt == NULL) || (elem == NULL))
	return(-1);

    elem->next = ctxt->varsTab[ctxt->varsNr - 1];
    ctxt->varsTab[ctxt->varsNr - 1] = elem;
    ctxt->vars = elem;
    return(0);
}

/**
 * xsltAddStackElemList:
 * @ctxt:  xn XSLT transformation context
 * @elems:  a stack element list
 *
 * add the new element list at this level of the stack.
 *
 * Returns 0 in case of success, -1 in case of failure.
 */
int
xsltAddStackElemList(xsltTransformContextPtr ctxt, xsltStackElemPtr elems) {
    xsltStackElemPtr cur;

    if ((ctxt == NULL) || (elems == NULL))
	return(-1);

    /* TODO: check doublons */
    if (ctxt->varsTab[ctxt->varsNr - 1] != NULL) {
	cur = ctxt->varsTab[ctxt->varsNr - 1];
	while (cur->next != NULL)
	    cur = cur->next;
	cur->next = elems;
    } else {
	elems->next = ctxt->varsTab[ctxt->varsNr - 1];
	ctxt->varsTab[ctxt->varsNr - 1] = elems;
	ctxt->vars = elems;
    }
    return(0);
}

/**
 * xsltStackLookup:
 * @ctxt:  an XSLT transformation context
 * @name:  the local part of the name
 * @nameURI:  the URI part of the name
 *
 * Locate an element in the stack based on its name.
 */
static xsltStackElemPtr
xsltStackLookup(xsltTransformContextPtr ctxt, const xmlChar *name,
	        const xmlChar *nameURI) {
    xsltStackElemPtr ret = NULL;
    int i;
    xsltStackElemPtr cur;

    if ((ctxt == NULL) || (name == NULL) || (ctxt->varsNr == 0))
	return(NULL);

    /*
     * Do the lookup from the top of the stack, but
     * don't use params being computed in a call-param
     */
    ;

    for (i = ctxt->varsNr; i > ctxt->varsBase; i--) {
	cur = ctxt->varsTab[i-1];
	while (cur != NULL) {
	    if (xmlStrEqual(cur->name, name)) {
		if (nameURI == NULL) {
		    if (cur->nameURI == NULL) {
			return(cur);
		    }
		} else {
		    if ((cur->nameURI != NULL) &&
			(xmlStrEqual(cur->nameURI, nameURI))) {
			return(cur);
		    }
		}

	    }
	    cur = cur->next;
	}
    }
    return(ret);
}

/************************************************************************
 *									*
 *			Module interfaces				*
 *									*
 ************************************************************************/

/**
 * xsltEvalVariable:
 * @ctxt:  the XSLT transformation context
 * @elem:  the variable or parameter.
 * @precomp: pointer to precompiled data
 *
 * Evaluate a variable value.
 *
 * Returns the XPath Object value or NULL in case of error
 */
static xmlXPathObjectPtr
xsltEvalVariable(xsltTransformContextPtr ctxt, xsltStackElemPtr elem,
	         xsltStylePreCompPtr precomp) {
    xmlXPathObjectPtr result = NULL;
    int oldProximityPosition, oldContextSize;
    xmlNodePtr oldInst, oldNode;
    xsltDocumentPtr oldDoc;
    int oldNsNr;
    xmlNsPtr *oldNamespaces;

    if ((ctxt == NULL) || (elem == NULL))
	return(NULL);

#ifdef WITH_XSLT_DEBUG_VARIABLE
    xsltGenericDebug(xsltGenericDebugContext,
	"Evaluating variable %s\n", elem->name);
#endif
    if (elem->select != NULL) {
	xmlXPathCompExprPtr comp = NULL;

	if ((precomp != NULL) && (precomp->comp != NULL)) {
	    comp = precomp->comp;
	} else {
	    comp = xmlXPathCompile(elem->select);
	}
	if (comp == NULL)
	    return(NULL);
	oldProximityPosition = ctxt->xpathCtxt->proximityPosition;
	oldContextSize = ctxt->xpathCtxt->contextSize;
	ctxt->xpathCtxt->node = (xmlNodePtr) ctxt->node;
	oldDoc = ctxt->document;
	oldNode = ctxt->node;
	oldInst = ctxt->inst;
	oldNsNr = ctxt->xpathCtxt->nsNr;
	oldNamespaces = ctxt->xpathCtxt->namespaces;
	if (precomp != NULL) {
	    ctxt->inst = precomp->inst;
	    ctxt->xpathCtxt->namespaces = precomp->nsList;
	    ctxt->xpathCtxt->nsNr = precomp->nsNr;
	} else {
	    ctxt->inst = NULL;
	    ctxt->xpathCtxt->namespaces = NULL;
	    ctxt->xpathCtxt->nsNr = 0;
	}
	result = xmlXPathCompiledEval(comp, ctxt->xpathCtxt);
	ctxt->xpathCtxt->contextSize = oldContextSize;
	ctxt->xpathCtxt->proximityPosition = oldProximityPosition;
	ctxt->xpathCtxt->nsNr = oldNsNr;
	ctxt->xpathCtxt->namespaces = oldNamespaces;
	ctxt->inst = oldInst;
	ctxt->node = oldNode;
	ctxt->document = oldDoc;
	if ((precomp == NULL) || (precomp->comp == NULL))
	    xmlXPathFreeCompExpr(comp);
	if (result == NULL) {
	    xsltPrintErrorContext(ctxt, NULL, precomp->inst);
	    xsltGenericError(xsltGenericErrorContext,
		"Evaluating variable %s failed\n", elem->name);
	    ctxt->state = XSLT_STATE_STOPPED;
#ifdef WITH_XSLT_DEBUG_VARIABLE
#ifdef LIBXML_DEBUG_ENABLED
	} else {
	    if ((xsltGenericDebugContext == stdout) ||
		(xsltGenericDebugContext == stderr))
		xmlXPathDebugDumpObject((FILE *)xsltGenericDebugContext,
					result, 0);
#endif
#endif
	}
    } else {
	if (elem->tree == NULL) {
	    result = xmlXPathNewCString("");
	} else {
	    /*
	     * This is a result tree fragment.
	     */
	    xmlNodePtr container;
	    xmlNodePtr oldInsert;

	    container = xmlNewDocNode(ctxt->document->doc, NULL,
				      (const xmlChar *) "fake", NULL);
	    if (container == NULL)
		return(NULL);

	    oldInsert = ctxt->insert;
	    ctxt->insert = container;
	    xsltApplyOneTemplate(ctxt, ctxt->node, elem->tree, NULL, NULL);
	    ctxt->insert = oldInsert;

	    result = xmlXPathNewValueTree(container);
	    if (result == NULL) {
		result = xmlXPathNewCString("");
	    } else {
		/*
		 * Tag the subtree for removal once consumed
		 */
		result->boolval = 1;
	    }
#ifdef WITH_XSLT_DEBUG_VARIABLE
#ifdef LIBXML_DEBUG_ENABLED
	    if ((xsltGenericDebugContext == stdout) ||
		(xsltGenericDebugContext == stderr))
		xmlXPathDebugDumpObject((FILE *)xsltGenericDebugContext,
					result, 0);
#endif
#endif
	}
    }
    return(result);
}

/**
 * xsltEvalGlobalVariable:
 * @elem:  the variable or parameter.
 * @ctxt:  the XSLT transformation context
 *
 * Evaluate a global variable value.
 *
 * Returns the XPath Object value or NULL in case of error
 */
static xmlXPathObjectPtr
xsltEvalGlobalVariable(xsltStackElemPtr elem, xsltTransformContextPtr ctxt) {
    xmlXPathObjectPtr result = NULL;
    xsltStylePreCompPtr precomp;
    int oldProximityPosition, oldContextSize;
    xmlNodePtr oldInst;
    int oldNsNr;
    xmlNsPtr *oldNamespaces;

    if ((ctxt == NULL) || (elem == NULL))
	return(NULL);
    if (elem->computed)
	return(elem->value);


#ifdef WITH_XSLT_DEBUG_VARIABLE
    xsltGenericDebug(xsltGenericDebugContext,
	"Evaluating global variable %s\n", elem->name);
#endif

    precomp = elem->comp;
    if (elem->select != NULL) {
	xmlXPathCompExprPtr comp = NULL;

	if ((precomp != NULL) && (precomp->comp != NULL)) {
	    comp = precomp->comp;
	} else {
	    comp = xmlXPathCompile(elem->select);
	}
	if (comp == NULL)
	    return(NULL);
	oldProximityPosition = ctxt->xpathCtxt->proximityPosition;
	oldContextSize = ctxt->xpathCtxt->contextSize;
	oldInst = ctxt->inst;
	oldNsNr = ctxt->xpathCtxt->nsNr;
	oldNamespaces = ctxt->xpathCtxt->namespaces;
	if (precomp != NULL) {
	    ctxt->inst = precomp->inst;
	    ctxt->xpathCtxt->namespaces = precomp->nsList;
	    ctxt->xpathCtxt->nsNr = precomp->nsNr;
	} else {
	    ctxt->inst = NULL;
	    ctxt->xpathCtxt->namespaces = NULL;
	    ctxt->xpathCtxt->nsNr = 0;
	}
	ctxt->xpathCtxt->node = (xmlNodePtr) ctxt->node;
	result = xmlXPathCompiledEval(comp, ctxt->xpathCtxt);
	ctxt->xpathCtxt->contextSize = oldContextSize;
	ctxt->xpathCtxt->proximityPosition = oldProximityPosition;
	ctxt->inst = oldInst;
	ctxt->xpathCtxt->nsNr = oldNsNr;
	ctxt->xpathCtxt->namespaces = oldNamespaces;
	if ((precomp == NULL) || (precomp->comp == NULL))
	    xmlXPathFreeCompExpr(comp);
	if (result == NULL) {
	    xsltPrintErrorContext(ctxt, NULL, precomp->inst);
	    xsltGenericError(xsltGenericErrorContext,
		"Evaluating global variable %s failed\n", elem->name);
	    ctxt->state = XSLT_STATE_STOPPED;
#ifdef WITH_XSLT_DEBUG_VARIABLE
#ifdef LIBXML_DEBUG_ENABLED
	} else {
	    if ((xsltGenericDebugContext == stdout) ||
		(xsltGenericDebugContext == stderr))
		xmlXPathDebugDumpObject((FILE *)xsltGenericDebugContext,
					result, 0);
#endif
#endif
	}
    } else {
	if (elem->tree == NULL) {
	    result = xmlXPathNewCString("");
	} else {
	    /*
	     * This is a result tree fragment.
	     */
	    xmlNodePtr container;
	    xmlNodePtr oldInsert;

	    container = xmlNewDocNode(ctxt->document->doc, NULL,
				      (const xmlChar *) "fake", NULL);
	    if (container == NULL)
		return(NULL);

	    oldInsert = ctxt->insert;
	    ctxt->insert = container;
	    xsltApplyOneTemplate(ctxt, ctxt->node, elem->tree, NULL, NULL);
	    ctxt->insert = oldInsert;

	    result = xmlXPathNewValueTree(container);
	    if (result == NULL) {
		result = xmlXPathNewCString("");
	    } else {
		/*
		 * Tag the subtree for removal once consumed
		 */
		result->boolval = 1;
	    }
#ifdef WITH_XSLT_DEBUG_VARIABLE
#ifdef LIBXML_DEBUG_ENABLED
	    if ((xsltGenericDebugContext == stdout) ||
		(xsltGenericDebugContext == stderr))
		xmlXPathDebugDumpObject((FILE *)xsltGenericDebugContext,
					result, 0);
#endif
#endif
	}
    }
    if (result != NULL) {
	elem->value = result;
	elem->computed = 1;
    }
    return(result);
}

/**
 * xsltEvalGlobalVariables:
 * @ctxt:  the XSLT transformation context
 *
 * Evaluate the global variables of a stylesheet. This need to be
 * done on parsed stylesheets before starting to apply transformations
 *
 * Returns 0 in case of success, -1 in case of error
 */
int
xsltEvalGlobalVariables(xsltTransformContextPtr ctxt) {
    xsltStackElemPtr elem;
    xsltStylesheetPtr style;

    if (ctxt == NULL)
	return(-1);
 
#ifdef WITH_XSLT_DEBUG_VARIABLE
    xsltGenericDebug(xsltGenericDebugContext,
	"Registering global variables\n");
#endif
    ctxt->node = (xmlNodePtr) ctxt->document->doc;
    ctxt->xpathCtxt->contextSize = 1;
    ctxt->xpathCtxt->proximityPosition = 1;

    /*
     * Walk the list from the stylesheets and populate the hash table
     */
    style = ctxt->style;
    while (style != NULL) {
	elem = style->variables;
	
#ifdef WITH_XSLT_DEBUG_VARIABLE
	if ((style->doc != NULL) && (style->doc->URL != NULL)) {
	    xsltGenericDebug(xsltGenericDebugContext,
			     "Registering global variables from %s\n",
		             style->doc->URL);
	}
#endif

	while (elem != NULL) {
	    xsltStackElemPtr def;

	    /*
	     * Global variables are stored in the variables pool.
	     */
	    def = (xsltStackElemPtr) 
		    xmlHashLookup2(ctxt->globalVars,
		                 elem->name, elem->nameURI);
	    if (def == NULL) {
		int res;

		def = xsltCopyStackElem(elem);
		res = xmlHashAddEntry2(ctxt->globalVars,
				 elem->name, elem->nameURI, def);
	    } else if ((elem->comp != NULL) &&
		       (elem->comp->type == XSLT_FUNC_VARIABLE)) {
		/*
		 * Redefinition of variables from a different stylesheet
		 * should not generate a message.
		 */
		if ((elem->comp->inst != NULL) &&
		    (def->comp != NULL) && (def->comp->inst != NULL) &&
		    (elem->comp->inst->doc == def->comp->inst->doc)) {
		    xsltPrintErrorContext(ctxt, style, elem->comp->inst);
		    xsltGenericError(xsltGenericErrorContext,
			"Global variable %s already defined\n", elem->name);
		}
	    }
	    elem = elem->next;
	}

	style = xsltNextImport(style);
    }

    /*
     * This part does the actual evaluation
     */
    ctxt->node = (xmlNodePtr) ctxt->document->doc;
    ctxt->xpathCtxt->contextSize = 1;
    ctxt->xpathCtxt->proximityPosition = 1;
    xmlHashScan(ctxt->globalVars,
	        (xmlHashScanner) xsltEvalGlobalVariable, ctxt);

    return(0);
}

/**
 * xsltRegisterGlobalVariable:
 * @style:  the XSLT transformation context
 * @name:  the variable name
 * @ns_uri:  the variable namespace URI
 * @select:  the expression which need to be evaluated to generate a value
 * @tree:  the subtree if select is NULL
 * @comp:  the precompiled value
 * @value:  the string value if available
 *
 * Register a new variable value. If @value is NULL it unregisters
 * the variable
 *
 * Returns 0 in case of success, -1 in case of error
 */
static int
xsltRegisterGlobalVariable(xsltStylesheetPtr style, const xmlChar *name,
		     const xmlChar *ns_uri, const xmlChar *select,
		     xmlNodePtr tree, xsltStylePreCompPtr comp,
		     const xmlChar *value) {
    xsltStackElemPtr elem, tmp;
    if (style == NULL)
	return(-1);
    if (name == NULL)
	return(-1);
    if (comp == NULL)
	return(-1);

#ifdef WITH_XSLT_DEBUG_VARIABLE
    if (comp->type == XSLT_FUNC_PARAM)
	xsltGenericDebug(xsltGenericDebugContext,
			 "Defining global param %s\n", name);
    else
	xsltGenericDebug(xsltGenericDebugContext,
			 "Defining global variable %s\n", name);
#endif

    elem = xsltNewStackElem();
    if (elem == NULL)
	return(-1);
    elem->comp = comp;
    elem->name = xmlStrdup(name);
    elem->select = xmlStrdup(select);
    if (ns_uri)
	elem->nameURI = xmlStrdup(ns_uri);
    elem->tree = tree;
    tmp = style->variables;
    if (tmp == NULL) {
	elem->next = NULL;
	style->variables = elem;
    } else {
	while (tmp->next != NULL)
	    tmp = tmp->next;
	elem->next = NULL;
	tmp->next = elem;
    }
    if (value != NULL) {
	elem->computed = 1;
	elem->value = xmlXPathNewString(value);
    }
    return(0);
}

/**
 * xsltProcessUserParamInternal
 *
 * @ctxt:  the XSLT transformation context
 * @name:  a null terminated parameter name
 * @value: a null terminated value (may be an XPath expression)
 * @eval:  0 to treat the value literally, else evaluate as XPath expression
 *
 * If @eval is 0 then @value is treated literally and is stored in the global
 * parameter/variable table without any change.
 *
 * Uf @eval is 1 then @value is treated as an XPath expression and is
 * evaluated.  In this case, if you want to pass a string which will be
 * interpreted literally then it must be enclosed in single or double quotes.
 * If the string contains single quotes (double quotes) then it cannot be
 * enclosed single quotes (double quotes).  If the string which you want to
 * be treated literally contains both single and double quotes (e.g. Meet
 * at Joe's for "Twelfth Night" at 7 o'clock) then there is no suitable
 * quoting character.  You cannot use &apos; or &quot; inside the string
 * because the replacement of character entities with their equivalents is
 * done at a different stage of processing.  The solution is to call
 * xsltQuoteUserParams or xsltQuoteOneUserParam.
 *
 * This needs to be done on parsed stylesheets before starting to apply
 * transformations.  Normally this will be called (directly or indirectly)
 * only from xsltEvalUserParams, xsltEvalOneUserParam, xsltQuoteUserParams,
 * or xsltQuoteOneUserParam.
 *
 * Returns 0 in case of success, -1 in case of error
 */

static
int
xsltProcessUserParamInternal(xsltTransformContextPtr ctxt,
		             const xmlChar * name,
			     const xmlChar * value,
			     int eval) {

    xsltStylesheetPtr style;
    xmlChar *ncname;
    xmlChar *prefix;
    const xmlChar *href;
    xmlXPathCompExprPtr comp;
    xmlXPathObjectPtr result;
    int oldProximityPosition;
    int oldContextSize;
    int oldNsNr;
    xmlNsPtr *oldNamespaces;
    xsltStackElemPtr elem;
    int res;

    if (ctxt == NULL)
	return(-1);
    if (name == NULL)
	return(0);
    if (value == NULL)
	return(0);

    style = ctxt->style;

#ifdef WITH_XSLT_DEBUG_VARIABLE
    xsltGenericDebug(xsltGenericDebugContext,
	    "Evaluating user parameter %s=%s\n", name, value);
#endif

    /*
     * Name lookup
     */

    ncname = xmlSplitQName2(name, &prefix);
    href = NULL;
    if (ncname != NULL) {
	if (prefix != NULL) {
	    xmlNsPtr ns;

	    ns = xmlSearchNs(style->doc, xmlDocGetRootElement(style->doc),
			     prefix);
	    if (ns == NULL) {
		xsltPrintErrorContext(ctxt, style, NULL);
		xsltGenericError(xsltGenericErrorContext,
		"user param : no namespace bound to prefix %s\n", prefix);
		href = NULL;
	    } else {
		href = ns->href;
	    }
	    xmlFree(prefix);
	} else {
	    href = NULL;
	}
	xmlFree(ncname);
    } else {
	href = NULL;
	ncname = xmlStrdup(name);
    }

    /*
     * Do the evaluation if @eval is non-zero.
     */

    result = NULL;
    if (eval != 0) {
        comp = xmlXPathCompile(value);
	if (comp != NULL) {
	    oldProximityPosition = ctxt->xpathCtxt->proximityPosition;
	    oldContextSize = ctxt->xpathCtxt->contextSize;
	    ctxt->xpathCtxt->node = (xmlNodePtr) ctxt->node;

	    /* 
	     * There is really no in scope namespace for parameters on the
	     * command line.
	     */

	    oldNsNr = ctxt->xpathCtxt->nsNr;
	    oldNamespaces = ctxt->xpathCtxt->namespaces;
	    ctxt->xpathCtxt->namespaces = NULL;
	    ctxt->xpathCtxt->nsNr = 0;
	    result = xmlXPathCompiledEval(comp, ctxt->xpathCtxt);
	    ctxt->xpathCtxt->contextSize = oldContextSize;
	    ctxt->xpathCtxt->proximityPosition = oldProximityPosition;
	    ctxt->xpathCtxt->nsNr = oldNsNr;
	    ctxt->xpathCtxt->namespaces = oldNamespaces;
	    xmlXPathFreeCompExpr(comp);
	}
	if (result == NULL) {
	    xsltPrintErrorContext(ctxt, style, NULL);
	    xsltGenericError(xsltGenericErrorContext,
		"Evaluating user parameter %s failed\n", name);
	    ctxt->state = XSLT_STATE_STOPPED;
	    xmlFree(ncname);
	    return(-1);
	}
    }

    /* 
     * If @eval is 0 then @value is to be taken literally and result is NULL
     * 
     * If @eval is not 0, then @value is an XPath expression and has been
     * successfully evaluated and result contains the resulting value and
     * is not NULL.
     *
     * Now create an xsltStackElemPtr for insertion into the context's
     * global variable/parameter hash table.
     */

#ifdef WITH_XSLT_DEBUG_VARIABLE
#ifdef LIBXML_DEBUG_ENABLED
    if ((xsltGenericDebugContext == stdout) ||
        (xsltGenericDebugContext == stderr))
	    xmlXPathDebugDumpObject((FILE *)xsltGenericDebugContext,
				    result, 0);
#endif
#endif

    elem = xsltNewStackElem();
    if (elem != NULL) {
	elem->name = xmlStrdup(ncname);
	if (value != NULL)
	    elem->select = xmlStrdup(value);
	else
	    elem->select = NULL;
	if (href)
	    elem->nameURI = xmlStrdup(href);
	elem->tree = NULL;
	elem->computed = 1;
	if (eval == 0) {
	    elem->value = xmlXPathNewString(value);
	} 
	else {
	    elem->value = result;
	}
    }

    /*
     * Global parameters are stored in the XPath context variables pool.
     */

    res = xmlHashAddEntry2(ctxt->globalVars, ncname, href, elem);
    if (res != 0) {
	xsltFreeStackElem(elem);
	xsltPrintErrorContext(ctxt, style, NULL);
	xsltGenericError(xsltGenericErrorContext,
	    "Global parameter %s already defined\n", ncname);
    }
    xmlFree(ncname);
    return(0);
}

/**
 * xsltEvalUserParams:
 *
 * @ctxt:  the XSLT transformation context
 * @params:  a NULL terminated array of parameters name/value tuples
 *
 * Evaluate the global variables of a stylesheet. This needs to be
 * done on parsed stylesheets before starting to apply transformations.
 * Each of the parameters is evaluated as an XPath expression and stored
 * in the global variables/parameter hash table.  If you want your
 * parameter used literally, use xsltQuoteUserParams.
 *
 * Returns 0 in case of success, -1 in case of error
 */
 
int
xsltEvalUserParams(xsltTransformContextPtr ctxt, const char **params) {
    int indx = 0;
    const xmlChar *name;
    const xmlChar *value;

    if (params == NULL)
	return(0);
    while (params[indx] != NULL) {
	name = (const xmlChar *) params[indx++];
	value = (const xmlChar *) params[indx++];
    	if (xsltEvalOneUserParam(ctxt, name, value) != 0) 
	    return(-1);
    }
    return 0;
}

/**
 * xsltQuoteUserParams:
 *
 * @ctxt:  the XSLT transformation context
 * @params:  a NULL terminated arry of parameters names/values tuples
 *
 * Similar to xsltEvalUserParams, but the values are treated literally and
 * are * *not* evaluated as XPath expressions. This should be done on parsed
 * stylesheets before starting to apply transformations.
 *
 * Returns 0 in case of success, -1 in case of error.
 */
 
int
xsltQuoteUserParams(xsltTransformContextPtr ctxt, const char **params) {
    int indx = 0;
    const xmlChar *name;
    const xmlChar *value;

    if (params == NULL)
	return(0);
    while (params[indx] != NULL) {
	name = (const xmlChar *) params[indx++];
	value = (const xmlChar *) params[indx++];
    	if (xsltQuoteOneUserParam(ctxt, name, value) != 0) 
	    return(-1);
    }
    return 0;
}

/**
 * xsltEvalOneUserParam:
 *
 * @ctxt:  the XSLT transformation context
 * @name:  a null terminated string giving the name of the parameter
 * @value  a null terminated string giving the XPath expression to be evaluated
 *
 * This is normally called from xsltEvalUserParams to process a single
 * parameter from a list of parameters.  The @value is evaluated as an
 * XPath expression and the result is stored in the context's global
 * variable/parameter hash table.
 *
 * To have a parameter treated literally (not as an XPath expression)
 * use xsltQuoteUserParams (or xsltQuoteOneUserParam).  For more
 * details see description of xsltProcessOneUserParamInternal.
 *
 * Returns 0 in case of success, -1 in case of error.
 */

int
xsltEvalOneUserParam(xsltTransformContextPtr ctxt,
    		     const xmlChar * name,
		     const xmlChar * value) {
    return xsltProcessUserParamInternal(ctxt, name, value,
		                        1 /* xpath eval ? */);
}

/**
 * xsltQuoteOneUserParam:
 *
 * @ctxt:  the XSLT transformation context
 * @name:  a null terminated string giving the name of the parameter
 * @value  a null terminated string giving the parameter value
 *
 * This is normally called from xsltQuoteUserParams to process a single
 * parameter from a list of parameters.  The @value is stored in the
 * context's global variable/parameter hash table.
 *
 * Returns 0 in case of success, -1 in case of error.
 */

int
xsltQuoteOneUserParam(xsltTransformContextPtr ctxt,
			 const xmlChar * name,
			 const xmlChar * value) {
    return xsltProcessUserParamInternal(ctxt, name, value,
					0 /* xpath eval ? */);
}

/**
 * xsltBuildVariable:
 * @ctxt:  the XSLT transformation context
 * @comp:  the precompiled form
 * @tree:  the tree if select is NULL
 *
 * Computes a new variable value.
 *
 * Returns the xsltStackElemPtr or NULL in case of error
 */
static xsltStackElemPtr
xsltBuildVariable(xsltTransformContextPtr ctxt, xsltStylePreCompPtr comp,
		  xmlNodePtr tree) {
    xsltStackElemPtr elem;

#ifdef WITH_XSLT_DEBUG_VARIABLE
    xsltGenericDebug(xsltGenericDebugContext,
		     "Building variable %s", comp->name);
    if (comp->select != NULL)
	xsltGenericDebug(xsltGenericDebugContext,
			 " select %s", comp->select);
    xsltGenericDebug(xsltGenericDebugContext, "\n");
#endif

    elem = xsltNewStackElem();
    if (elem == NULL)
	return(NULL);
    elem->comp = comp;
    elem->name = xmlStrdup(comp->name);
    if (comp->select != NULL)
	elem->select = xmlStrdup(comp->select);
    else
	elem->select = NULL;
    if (comp->ns)
	elem->nameURI = xmlStrdup(comp->ns);
    elem->tree = tree;
    if (elem->computed == 0) {
	elem->value = xsltEvalVariable(ctxt, elem, comp);
	if (elem->value != NULL)
	    elem->computed = 1;
    }
    return(elem);
}

/**
 * xsltRegisterVariable:
 * @ctxt:  the XSLT transformation context
 * @comp:  pointer to precompiled data
 * @tree:  the tree if select is NULL
 * @param:  this is a parameter actually
 *
 * Computes and register a new variable value. 
 *
 * Returns 0 in case of success, -1 in case of error
 */
static int
xsltRegisterVariable(xsltTransformContextPtr ctxt, xsltStylePreCompPtr comp,
		     xmlNodePtr tree, int param) {
    xsltStackElemPtr elem;

    if (xsltCheckStackElem(ctxt, comp->name, comp->ns) != 0) {
	if (!param) {
	    xsltPrintErrorContext(ctxt, NULL, comp->inst);
	    xsltGenericError(xsltGenericErrorContext,
	    "xsl:variable : redefining %s\n", comp->name);
	}
#ifdef WITH_XSLT_DEBUG_VARIABLE
	else
	    xsltGenericDebug(xsltGenericDebugContext,
		     "param %s defined by caller\n", comp->name);
#endif
	return(0);
    }
    elem = xsltBuildVariable(ctxt, comp, tree);
    xsltAddStackElem(ctxt, elem);
    return(0);
}

/**
 * xsltGlobalVariableLookup:
 * @ctxt:  the XSLT transformation context
 * @name:  the variable name
 * @ns_uri:  the variable namespace URI
 *
 * Search in the Variable array of the context for the given
 * variable value.
 *
 * Returns the value or NULL if not found
 */
static xmlXPathObjectPtr
xsltGlobalVariableLookup(xsltTransformContextPtr ctxt, const xmlChar *name,
		         const xmlChar *ns_uri) {
    xsltStackElemPtr elem;
    xmlXPathObjectPtr ret = NULL;

    /*
     * Lookup the global variables in XPath global variable hash table
     */
    if ((ctxt->xpathCtxt == NULL) || (ctxt->globalVars == NULL))
	return(NULL);
    elem = (xsltStackElemPtr)
	    xmlHashLookup2(ctxt->globalVars, name, ns_uri);
    if (elem == NULL) {
#ifdef WITH_XSLT_DEBUG_VARIABLE
	xsltGenericDebug(xsltGenericDebugContext,
			 "global variable not found %s\n", name);
#endif
	return(NULL);
    }
    if (elem->computed == 0)
	ret = xsltEvalGlobalVariable(elem, ctxt);
    else
	ret = elem->value;
    return(xmlXPathObjectCopy(ret));
}

/**
 * xsltVariableLookup:
 * @ctxt:  the XSLT transformation context
 * @name:  the variable name
 * @ns_uri:  the variable namespace URI
 *
 * Search in the Variable array of the context for the given
 * variable value.
 *
 * Returns the value or NULL if not found
 */
xmlXPathObjectPtr
xsltVariableLookup(xsltTransformContextPtr ctxt, const xmlChar *name,
		   const xmlChar *ns_uri) {
    xsltStackElemPtr elem;

    if (ctxt == NULL)
	return(NULL);

    elem = xsltStackLookup(ctxt, name, ns_uri);
    if (elem == NULL) {
	return(xsltGlobalVariableLookup(ctxt, name, ns_uri));
    }
    if (elem->computed == 0) {
#ifdef WITH_XSLT_DEBUG_VARIABLE
	xsltGenericDebug(xsltGenericDebugContext,
		         "uncomputed variable %s\n", name);
#endif
        elem->value = xsltEvalVariable(ctxt, elem, NULL);
	elem->computed = 1;
    }
    if (elem->value != NULL)
	return(xmlXPathObjectCopy(elem->value));
#ifdef WITH_XSLT_DEBUG_VARIABLE
    xsltGenericDebug(xsltGenericDebugContext,
		     "variable not found %s\n", name);
#endif
    return(NULL);
}

/**
 * xsltParseStylesheetCallerParam:
 * @ctxt:  the XSLT transformation context
 * @cur:  the "param" element
 *
 * parse an XSLT transformation param declaration, compute
 * its value but doesn't record it.
 *
 * It returns the new xsltStackElemPtr or NULL
 */

xsltStackElemPtr
xsltParseStylesheetCallerParam(xsltTransformContextPtr ctxt, xmlNodePtr cur) {
    xmlNodePtr tree = NULL;
    xsltStackElemPtr elem = NULL;
    xsltStylePreCompPtr comp;

    if ((cur == NULL) || (ctxt == NULL))
	return(NULL);
    comp = (xsltStylePreCompPtr) cur->_private;
    if (comp == NULL) {
	xsltPrintErrorContext(ctxt, NULL, cur);
	xsltGenericError(xsltGenericErrorContext,
	    "xsl:param : compilation error\n");
	return(NULL);
    }

    if (comp->name == NULL) {
	xsltPrintErrorContext(ctxt, NULL, cur);
	xsltGenericError(xsltGenericErrorContext,
	    "xsl:param : missing name attribute\n");
	return(NULL);
    }

#ifdef WITH_XSLT_DEBUG_VARIABLE
    xsltGenericDebug(xsltGenericDebugContext,
	    "Handling param %s\n", comp->name);
#endif

    if (comp->select == NULL) {
	tree = cur->children;
    } else {
#ifdef WITH_XSLT_DEBUG_VARIABLE
	xsltGenericDebug(xsltGenericDebugContext,
	    "        select %s\n", comp->select);
#endif
	tree = cur;
    }

    elem = xsltBuildVariable(ctxt, comp, tree);

    return(elem);
}

/**
 * xsltParseGlobalVariable:
 * @style:  the XSLT stylesheet
 * @cur:  the "variable" element
 *
 * parse an XSLT transformation variable declaration and record
 * its value.
 */

void
xsltParseGlobalVariable(xsltStylesheetPtr style, xmlNodePtr cur) {
    xsltStylePreCompPtr comp;

    if ((cur == NULL) || (style == NULL))
	return;

    xsltStylePreCompute(style, cur);
    comp = (xsltStylePreCompPtr) cur->_private;
    if (comp == NULL) {
	xsltPrintErrorContext(NULL, style, cur);
	xsltGenericError(xsltGenericErrorContext,
	     "xsl:variable : compilation failed\n");
	return;
    }

    if (comp->name == NULL) {
	xsltPrintErrorContext(NULL, style, cur);
	xsltGenericError(xsltGenericErrorContext,
	    "xsl:variable : missing name attribute\n");
	return;
    }

#ifdef WITH_XSLT_DEBUG_VARIABLE
    xsltGenericDebug(xsltGenericDebugContext,
	"Registering global variable %s\n", comp->name);
#endif

    xsltRegisterGlobalVariable(style, comp->name, comp->ns, comp->select,
	                       cur->children, comp, NULL);
}

/**
 * xsltParseGlobalParam:
 * @style:  the XSLT stylesheet
 * @cur:  the "param" element
 *
 * parse an XSLT transformation param declaration and record
 * its value.
 */

void
xsltParseGlobalParam(xsltStylesheetPtr style, xmlNodePtr cur) {
    xsltStylePreCompPtr comp;

    if ((cur == NULL) || (style == NULL))
	return;

    xsltStylePreCompute(style, cur);
    comp = (xsltStylePreCompPtr) cur->_private;
    if (comp == NULL) {
	xsltPrintErrorContext(NULL, style, cur);
	xsltGenericError(xsltGenericErrorContext,
	     "xsl:param : compilation failed\n");
	return;
    }

    if (comp->name == NULL) {
	xsltPrintErrorContext(NULL, style, cur);
	xsltGenericError(xsltGenericErrorContext,
	    "xsl:param : missing name attribute\n");
	return;
    }

#ifdef WITH_XSLT_DEBUG_VARIABLE
    xsltGenericDebug(xsltGenericDebugContext,
	"Registering global param %s\n", comp->name);
#endif

    xsltRegisterGlobalVariable(style, comp->name, comp->ns, comp->select,
	                       cur->children, comp, NULL);
}

/**
 * xsltParseStylesheetVariable:
 * @ctxt:  the XSLT transformation context
 * @cur:  the "variable" element
 *
 * parse an XSLT transformation variable declaration and record
 * its value.
 */

void
xsltParseStylesheetVariable(xsltTransformContextPtr ctxt, xmlNodePtr cur) {
    xsltStylePreCompPtr comp;

    if ((cur == NULL) || (ctxt == NULL))
	return;

    comp = (xsltStylePreCompPtr) cur->_private;
    if (comp == NULL) {
	xsltPrintErrorContext(ctxt, NULL, cur);
	xsltGenericError(xsltGenericErrorContext,
	     "xsl:variable : compilation failed\n");
	return;
    }

    if (comp->name == NULL) {
	xsltPrintErrorContext(ctxt, NULL, cur);
	xsltGenericError(xsltGenericErrorContext,
	    "xsl:variable : missing name attribute\n");
	return;
    }

#ifdef WITH_XSLT_DEBUG_VARIABLE
    xsltGenericDebug(xsltGenericDebugContext,
	"Registering variable %s\n", comp->name);
#endif

    xsltRegisterVariable(ctxt, comp, cur->children, 0);
}

/**
 * xsltParseStylesheetParam:
 * @ctxt:  the XSLT transformation context
 * @cur:  the "param" element
 *
 * parse an XSLT transformation param declaration and record
 * its value.
 */

void
xsltParseStylesheetParam(xsltTransformContextPtr ctxt, xmlNodePtr cur) {
    xsltStylePreCompPtr comp;

    if ((cur == NULL) || (ctxt == NULL))
	return;

    comp = (xsltStylePreCompPtr) cur->_private;
    if (comp == NULL) {
	xsltPrintErrorContext(ctxt, NULL, cur);
	xsltGenericError(xsltGenericErrorContext,
	     "xsl:param : compilation failed\n");
	return;
    }

    if (comp->name == NULL) {
	xsltPrintErrorContext(ctxt, NULL, cur);
	xsltGenericError(xsltGenericErrorContext,
	    "xsl:param : missing name attribute\n");
	return;
    }

#ifdef WITH_XSLT_DEBUG_VARIABLE
    xsltGenericDebug(xsltGenericDebugContext,
	"Registering param %s\n", comp->name);
#endif

    xsltRegisterVariable(ctxt, comp, cur->children, 1);
}

/**
 * xsltFreeGlobalVariables:
 * @ctxt:  the XSLT transformation context
 *
 * Free up the data associated to the global variables
 * its value.
 */

void
xsltFreeGlobalVariables(xsltTransformContextPtr ctxt) {
    xmlHashFree(ctxt->globalVars, (xmlHashDeallocator) xsltFreeStackElem);
}

/**
 * xsltXPathVariableLookup:
 * @ctxt:  a void * but the the XSLT transformation context actually
 * @name:  the variable name
 * @ns_uri:  the variable namespace URI
 *
 * This is the entry point when a varibale is needed by the XPath
 * interpretor.
 *
 * Returns the value or NULL if not found
 */
xmlXPathObjectPtr
xsltXPathVariableLookup(void *ctxt, const xmlChar *name,
	                const xmlChar *ns_uri) {
    xsltTransformContextPtr context;
    xmlXPathObjectPtr ret;

    if ((ctxt == NULL) || (name == NULL))
	return(NULL);

#ifdef WITH_XSLT_DEBUG_VARIABLE
    xsltGenericDebug(xsltGenericDebugContext,
	    "Lookup variable %s\n", name);
#endif
    context = (xsltTransformContextPtr) ctxt;
    ret = xsltVariableLookup(context, name, ns_uri);
    if (ret == NULL) {
	xsltPrintErrorContext(ctxt, NULL, NULL);
	xsltGenericError(xsltGenericErrorContext,
	    "unregistered variable %s\n", name);
    }
#ifdef WITH_XSLT_DEBUG_VARIABLE
    if (ret != NULL)
	xsltGenericDebug(xsltGenericDebugContext,
	    "found variable %s\n", name);
#endif
    return(ret);
}


