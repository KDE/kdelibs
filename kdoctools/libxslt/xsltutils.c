/*
 * xsltutils.c: Utilities for the XSL Transformation 1.0 engine
 *
 * Reference:
 *   http://www.w3.org/TR/1999/REC-xslt-19991116
 *
 * See Copyright for the status of this software.
 *
 * daniel@veillard.com
 */

#include "libxslt.h"

#include <stdio.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <stdarg.h>

#include <libxml/xmlmemory.h>
#include <libxml/tree.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlerror.h>
#include <libxml/xmlIO.h>
#include "xsltutils.h"
#include "templates.h"
#include "xsltInternals.h"
#include "imports.h"

/* gettimeofday on Windows ??? */
#ifdef WIN32
#ifdef _MSC_VER
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#define gettimeofday(p1,p2)
#define HAVE_GETTIMEOFDAY
#endif /* _MS_VER */
#endif /* WIN32 */

/************************************************************************
 * 									*
 * 			Convenience function				*
 * 									*
 ************************************************************************/

/**
 * xsltGetNsProp:
 * @node:  the node
 * @name:  the attribute name
 * @nameSpace:  the URI of the namespace
 *
 * Similar to xmlGetNsProp() but with a slightly different semantic
 *
 * Search and get the value of an attribute associated to a node
 * This attribute has to be anchored in the namespace specified,
 * or has no namespace and the element is in that namespace.
 *
 * This does the entity substitution.
 * This function looks in DTD attribute declaration for #FIXED or
 * default declaration values unless DTD use has been turned off.
 *
 * Returns the attribute value or NULL if not found.
 *     It's up to the caller to free the memory.
 */
xmlChar *
xsltGetNsProp(xmlNodePtr node, const xmlChar *name, const xmlChar *nameSpace) {
    xmlAttrPtr prop;
    xmlDocPtr doc;
    xmlNsPtr ns;

    if (node == NULL)
	return(NULL);

    prop = node->properties;
    if (nameSpace == NULL)
	return(xmlGetProp(node, name));
    while (prop != NULL) {
	/*
	 * One need to have
	 *   - same attribute names
	 *   - and the attribute carrying that namespace
	 */
        if ((xmlStrEqual(prop->name, name)) &&
	    (((prop->ns == NULL) && (node->ns != NULL) &&
	      (xmlStrEqual(node->ns->href, nameSpace))) ||
	     ((prop->ns != NULL) &&
	      (xmlStrEqual(prop->ns->href, nameSpace))))) {
	    xmlChar *ret;

	    ret = xmlNodeListGetString(node->doc, prop->children, 1);
	    if (ret == NULL) return(xmlStrdup((xmlChar *)""));
	    return(ret);
        }
	prop = prop->next;
    }

    /*
     * Check if there is a default declaration in the internal
     * or external subsets
     */
    doc =  node->doc;
    if (doc != NULL) {
        if (doc->intSubset != NULL) {
	    xmlAttributePtr attrDecl;

	    attrDecl = xmlGetDtdAttrDesc(doc->intSubset, node->name, name);
	    if ((attrDecl == NULL) && (doc->extSubset != NULL))
		attrDecl = xmlGetDtdAttrDesc(doc->extSubset, node->name, name);
		
	    if ((attrDecl != NULL) && (attrDecl->prefix != NULL)) {
	        /*
		 * The DTD declaration only allows a prefix search
		 */
		ns = xmlSearchNs(doc, node, attrDecl->prefix);
		if ((ns != NULL) && (xmlStrEqual(ns->href, nameSpace)))
		    return(xmlStrdup(attrDecl->defaultValue));
	    }
	}
    }
    return(NULL);
}


/************************************************************************
 * 									*
 * 		Handling of XSLT stylesheets messages			*
 * 									*
 ************************************************************************/

/**
 * xsltMessage:
 * @ctxt:  an XSLT processing context
 * @node:  The current node
 * @inst:  The node containing the message instruction
 *
 * Process and xsl:message construct
 */
void
xsltMessage(xsltTransformContextPtr ctxt, xmlNodePtr node, xmlNodePtr inst) {
    xmlChar *prop, *message;
    int terminate = 0;

    if ((ctxt == NULL) || (inst == NULL))
	return;

    prop = xsltGetNsProp(inst, (const xmlChar *)"terminate", XSLT_NAMESPACE);
    if (prop != NULL) {
	if (xmlStrEqual(prop, (const xmlChar *)"yes")) {
	    terminate = 1;
	} else if (xmlStrEqual(prop, (const xmlChar *)"no")) {
	    terminate = 0;
	} else {
	    xsltGenericError(xsltGenericErrorContext,
		"xsl:message : terminate expecting 'yes' or 'no'\n");
	}
	xmlFree(prop);
    }
    message = xsltEvalTemplateString(ctxt, node, inst);
    if (message != NULL) {
	int len = xmlStrlen(message);

	xsltGenericError(xsltGenericErrorContext, "%s",
		         (const char *)message);
	if ((len > 0) && (message[len - 1] != '\n'))
	    xsltGenericError(xsltGenericErrorContext, "\n");
	xmlFree(message);
    }
    if (terminate)
	ctxt->state = XSLT_STATE_STOPPED;
}

/************************************************************************
 * 									*
 * 		Handling of out of context errors			*
 * 									*
 ************************************************************************/

/**
 * xsltGenericErrorDefaultFunc:
 * @ctx:  an error context
 * @msg:  the message to display/transmit
 * @...:  extra parameters for the message display
 * 
 * Default handler for out of context error messages.
 */
static void
xsltGenericErrorDefaultFunc(void *ctx ATTRIBUTE_UNUSED, const char *msg, ...) {
    va_list args;

    if (xsltGenericErrorContext == NULL)
	xsltGenericErrorContext = (void *) stderr;

    va_start(args, msg);
    vfprintf((FILE *)xsltGenericErrorContext, msg, args);
    va_end(args);
}

xmlGenericErrorFunc xsltGenericError = xsltGenericErrorDefaultFunc;
void *xsltGenericErrorContext = NULL;


/**
 * xsltSetGenericErrorFunc:
 * @ctx:  the new error handling context
 * @handler:  the new handler function
 *
 * Function to reset the handler and the error context for out of
 * context error messages.
 * This simply means that @handler will be called for subsequent
 * error messages while not parsing nor validating. And @ctx will
 * be passed as first argument to @handler
 * One can simply force messages to be emitted to another FILE * than
 * stderr by setting @ctx to this file handle and @handler to NULL.
 */
void
xsltSetGenericErrorFunc(void *ctx, xmlGenericErrorFunc handler) {
    xsltGenericErrorContext = ctx;
    if (handler != NULL)
	xsltGenericError = handler;
    else
	xsltGenericError = xsltGenericErrorDefaultFunc;
}

/**
 * xsltGenericDebugDefaultFunc:
 * @ctx:  an error context
 * @msg:  the message to display/transmit
 * @...:  extra parameters for the message display
 * 
 * Default handler for out of context error messages.
 */
static void
xsltGenericDebugDefaultFunc(void *ctx ATTRIBUTE_UNUSED, const char *msg, ...) {
    va_list args;

    if (xsltGenericDebugContext == NULL)
	return;

    va_start(args, msg);
    vfprintf((FILE *)xsltGenericDebugContext, msg, args);
    va_end(args);
}

xmlGenericErrorFunc xsltGenericDebug = xsltGenericDebugDefaultFunc;
void *xsltGenericDebugContext = NULL;


/**
 * xsltSetGenericDebugFunc:
 * @ctx:  the new error handling context
 * @handler:  the new handler function
 *
 * Function to reset the handler and the error context for out of
 * context error messages.
 * This simply means that @handler will be called for subsequent
 * error messages while not parsing or validating. And @ctx will
 * be passed as first argument to @handler
 * One can simply force messages to be emitted to another FILE * than
 * stderr by setting @ctx to this file handle and @handler to NULL.
 */
void
xsltSetGenericDebugFunc(void *ctx, xmlGenericErrorFunc handler) {
    xsltGenericDebugContext = ctx;
    if (handler != NULL)
	xsltGenericDebug = handler;
    else
	xsltGenericDebug = xsltGenericDebugDefaultFunc;
}

/**
 * xsltPrintErrorContext:
 * @ctxt:  the transformation context
 * @style:  the stylesheet
 * @node:  the current node being processed
 *
 * Display the context of an error.
 */
void
xsltPrintErrorContext(xsltTransformContextPtr ctxt,
	              xsltStylesheetPtr style, xmlNodePtr node) {
    int line = 0;
    const xmlChar *file = NULL;
    const xmlChar *name = NULL;
    const char *type = "error";

    if ((node == NULL) && (ctxt != NULL))
	node = ctxt->inst;

    if (node != NULL)  {
	if ((node->type == XML_DOCUMENT_NODE) ||
	    (node->type == XML_HTML_DOCUMENT_NODE)) {
	    xmlDocPtr doc = (xmlDocPtr) node;

	    file = doc->URL;
	} else {
	    /*
	     * Try to find contextual informations to report
	     */
	    if (node->type == XML_ELEMENT_NODE) {
		line = (int) node->content;
	    } else if ((node->prev != NULL) &&
		       (node->prev->type == XML_ELEMENT_NODE)) {
		line = (int) node->prev->content;
	    } else if ((node->parent != NULL) &&
		       (node->parent->type == XML_ELEMENT_NODE)) {
		line = (int) node->parent->content;
	    }
	    if ((node->doc != NULL) && (node->doc->URL != NULL))
		file = node->doc->URL;
	    if (node->name != NULL)
		name = node->name;
	}
    } 
    
    if (ctxt != NULL)
	type = "runtime error";
    else if (style != NULL)
	type = "compilation error";

    if ((file != NULL) && (line != 0) && (name != NULL))
	xsltGenericError(xsltGenericErrorContext,
		"%s: file %s line %d element %s\n",
		type, file, line, name);
    else if ((file != NULL) && (name != NULL))
	xsltGenericError(xsltGenericErrorContext,
		"%s: file %s element %s\n",
		type, file, name);
    else if ((file != NULL) && (line != 0))
	xsltGenericError(xsltGenericErrorContext,
		"%s: file %s line %d\n",
		type, file, line);
    else if (file != NULL)
	xsltGenericError(xsltGenericErrorContext,
		"%s: file %s\n",
		type, file);
    else if (name != NULL)
	xsltGenericError(xsltGenericErrorContext,
		"%s: element %s\n",
		type, name);
    else
	xsltGenericError(xsltGenericErrorContext,
		"%s\n",
		type);
}

/************************************************************************
 * 									*
 * 				QNames					*
 * 									*
 ************************************************************************/

/**
 * xsltGetQNameURI:
 * @node:  the node holding the QName
 * @name:  pointer to the initial QName value
 *
 * This function analyzes @name, if the name contains a prefix,
 * the function seaches the associated namespace in scope for it.
 * It will also replace @name value with the NCName, the old value being
 * freed.
 * Errors in the prefix lookup are signalled by setting @name to NULL.
 *
 * NOTE: the namespace returned is a pointer to the place where it is
 *       defined and hence has the same lifespan as the document holding it.
 *
 * Returns the namespace URI if there is a prefix, or NULL if @name is
 *         not prefixed.
 */
const xmlChar *
xsltGetQNameURI(xmlNodePtr node, xmlChar ** name)
{
    int len = 0;
    xmlChar *qname;
    xmlNsPtr ns;

    if (name == NULL)
	return(NULL);
    qname = *name;
    if ((qname == NULL) || (*qname == 0))
	return(NULL);
    if (node == NULL) {
	xsltGenericError(xsltGenericErrorContext,
		         "QName: no element for namespace lookup %s\n",
			 qname);
	xmlFree(qname);
	*name = NULL;
	return(NULL);
    }

    /* nasty but valid */
    if (qname[0] == ':')
	return(NULL);

    /*
     * we are not trying to validate but just to cut, and yes it will
     * work even if this is a set of UTF-8 encoded chars
     */
    while ((qname[len] != 0) && (qname[len] != ':')) 
	len++;
    
    if (qname[len] == 0)
	return(NULL);

    /*
     * handle xml: separately, this one is magical
     */
    if ((qname[0] == 'x') && (qname[1] == 'm') &&
        (qname[2] == 'l') && (qname[3] == ':')) {
	if (qname[4] == 0)
	    return(NULL);
        *name = xmlStrdup(&qname[4]);
	xmlFree(qname);
	return(XML_XML_NAMESPACE);
    }

    qname[len] = 0;
    ns = xmlSearchNs(node->doc, node, qname);
    if (ns == NULL) {
	xsltGenericError(xsltGenericErrorContext,
		"%s:%s : no namespace bound to prefix %s\n",
		         qname, &qname[len + 1]);
	*name = NULL;
	xmlFree(qname);
	return(NULL);
    }
    *name = xmlStrdup(&qname[len + 1]);
    xmlFree(qname);
    return(ns->href);
}

/************************************************************************
 * 									*
 * 				Sorting					*
 * 									*
 ************************************************************************/

/**
 * xsltDocumentSortFunction:
 * @list:  the node set
 *
 * reorder the current node list @list accordingly to the document order
 */
void
xsltDocumentSortFunction(xmlNodeSetPtr list) {
    int i, j;
    int len, tst;
    xmlNodePtr node;

    if (list == NULL)
	return;
    len = list->nodeNr;
    if (len <= 1)
	return;
    /* TODO: sort is really not optimized, does it needs to ? */
    for (i = 0;i < len -1;i++) {
	for (j = i + 1; j < len; j++) {
	    tst = xmlXPathCmpNodes(list->nodeTab[i], list->nodeTab[j]);
	    if (tst == -1) {
		node = list->nodeTab[i];
		list->nodeTab[i] = list->nodeTab[j];
		list->nodeTab[j] = node;
	    }
	}
    }
}

/**
 * xsltComputeSortResult:
 * @ctxt:  a XSLT process context
 * @sorts:  array of sort nodes
 *
 * reorder the current node list accordingly to the set of sorting
 * requirement provided by the array of nodes.
 */
static xmlXPathObjectPtr *
xsltComputeSortResult(xsltTransformContextPtr ctxt, xmlNodePtr sort) {
    xmlXPathObjectPtr *results = NULL;
    xmlNodeSetPtr list = NULL;
    xmlXPathObjectPtr res;
    int len = 0;
    int i;
    xsltStylePreCompPtr comp;
    xmlNodePtr oldNode;
    xmlNodePtr oldInst;
    int	oldPos, oldSize ;
    int oldNsNr;
    xmlNsPtr *oldNamespaces;

    comp = sort->_private;
    if (comp == NULL) {
	xsltGenericError(xsltGenericErrorContext,
	     "xsl:sort : compilation failed\n");
	return(NULL);
    }

    if ((comp->select == NULL) || (comp->comp == NULL))
	return(NULL);

    list = ctxt->nodeList;
    if ((list == NULL) || (list->nodeNr <= 1))
	return(NULL);

    len = list->nodeNr;

    /* TODO: xsl:sort lang attribute */
    /* TODO: xsl:sort case-order attribute */


    results = xmlMalloc(len * sizeof(xmlXPathObjectPtr));
    if (results == NULL) {
	xsltGenericError(xsltGenericErrorContext,
	     "xsltComputeSortResult: memory allocation failure\n");
	return(NULL);
    }

    oldNode = ctxt->node;
    oldInst = ctxt->inst;
    oldPos = ctxt->xpathCtxt->proximityPosition;
    oldSize = ctxt->xpathCtxt->contextSize;
    oldNsNr = ctxt->xpathCtxt->nsNr;
    oldNamespaces = ctxt->xpathCtxt->namespaces;
    for (i = 0;i < len;i++) {
	ctxt->inst = sort;
	ctxt->xpathCtxt->contextSize = len;
	ctxt->xpathCtxt->proximityPosition = i + 1;
	ctxt->node = list->nodeTab[i];
	ctxt->xpathCtxt->node = ctxt->node;
	ctxt->xpathCtxt->namespaces = comp->nsList;
	ctxt->xpathCtxt->nsNr = comp->nsNr;
	res = xmlXPathCompiledEval(comp->comp, ctxt->xpathCtxt);
	if (res != NULL) {
	    if (res->type != XPATH_STRING)
		res = xmlXPathConvertString(res);
	    if (comp->number)
		res = xmlXPathConvertNumber(res);
	    res->index = i;	/* Save original pos for dupl resolv */
	    if (comp->number) {
		if (res->type == XPATH_NUMBER) {
		    results[i] = res;
		} else {
#ifdef WITH_XSLT_DEBUG_PROCESS
		    xsltGenericDebug(xsltGenericDebugContext,
			"xsltComputeSortResult: select didn't evaluate to a number\n");
#endif
		    results[i] = NULL;
		}
	    } else {
		if (res->type == XPATH_STRING) {
		    results[i] = res;
		} else {
#ifdef WITH_XSLT_DEBUG_PROCESS
		    xsltGenericDebug(xsltGenericDebugContext,
			"xsltComputeSortResult: select didn't evaluate to a string\n");
#endif
		    results[i] = NULL;
		}
	    }
	} else {
	    ctxt->state = XSLT_STATE_STOPPED;
	    results[i] = NULL;
	}
    }
    ctxt->node = oldNode;
    ctxt->inst = oldInst;
    ctxt->xpathCtxt->contextSize = oldSize;
    ctxt->xpathCtxt->proximityPosition = oldPos;
    ctxt->xpathCtxt->nsNr = oldNsNr;
    ctxt->xpathCtxt->namespaces = oldNamespaces;

    return(results);
}

/**
 * xsltDoSortFunction:
 * @ctxt:  a XSLT process context
 * @sorts:  array of sort nodes
 * @nbsorts:  the number of sorts in the array
 *
 * reorder the current node list accordingly to the set of sorting
 * requirement provided by the arry of nodes.
 */
void	
xsltDoSortFunction(xsltTransformContextPtr ctxt, xmlNodePtr *sorts,
	           int nbsorts) {
    xmlXPathObjectPtr *resultsTab[XSLT_MAX_SORT];
    xmlXPathObjectPtr *results = NULL, *res;
    xmlNodeSetPtr list = NULL;
    int descending, number, desc, numb;
    int len = 0;
    int i, j, incr;
    int tst;
    int depth;
    xmlNodePtr node;
    xmlXPathObjectPtr tmp;
    xsltStylePreCompPtr comp;

    if ((ctxt == NULL) || (sorts == NULL) || (nbsorts <= 0) ||
	(nbsorts >= XSLT_MAX_SORT))
	return;
    if (sorts[0] == NULL)
	return;
    comp = sorts[0]->_private;
    if (comp == NULL)
	return;

    list = ctxt->nodeList;
    if ((list == NULL) || (list->nodeNr <= 1))
	return; /* nothing to do */

    len = list->nodeNr;

    resultsTab[0] = xsltComputeSortResult(ctxt, sorts[0]);
    for (i = 1;i < XSLT_MAX_SORT;i++)
	resultsTab[i] = NULL;

    results = resultsTab[0];

    descending = comp->descending;
    number = comp->number;
    if (results == NULL)
	return;

    /* Shell's sort of node-set */
    for (incr = len / 2; incr > 0; incr /= 2) {
	for (i = incr; i < len; i++) {
	    j = i - incr;
	    if (results[i] == NULL)
		continue;
	    
	    while (j >= 0) {
		if (results[j] == NULL)
		    tst = 1;
		else {
		    if (number) {
			if (results[j]->floatval == results[j + incr]->floatval)
			    tst = 0;
			else if (results[j]->floatval > 
				results[j + incr]->floatval)
			    tst = 1;
			else tst = -1;
		    } else {
			tst = xmlStrcmp(results[j]->stringval,
				     results[j + incr]->stringval); 
		    }
		    if (descending)
			tst = -tst;
		}
		if (tst == 0) {
		    /*
		     * Okay we need to use multi level sorts
		     */
		    depth = 1;
		    while (depth < nbsorts) {
			if (sorts[depth] == NULL)
			    break;
			comp = sorts[depth]->_private;
			if (comp == NULL)
			    break;
			desc = comp->descending;
			numb = comp->number;

			/*
			 * Compute the result of the next level for the
			 * full set, this might be optimized ... or not
			 */
			if (resultsTab[depth] == NULL) 
			    resultsTab[depth] = xsltComputeSortResult(ctxt,
				                        sorts[depth]);
			res = resultsTab[depth];
			if (res == NULL) 
			    break;
			if (res[j] == NULL)
			    tst = 1;
			else {
			    if (numb) {
				if (res[j]->floatval == res[j + incr]->floatval)
				    tst = 0;
				else if (res[j]->floatval > 
					res[j + incr]->floatval)
				    tst = 1;
				else tst = -1;
			    } else {
				tst = xmlStrcmp(res[j]->stringval,
					     res[j + incr]->stringval); 
			    }
			    if (desc)
				tst = -tst;
			}

			/*
			 * if we still can't differenciate at this level
			 * try one level deeper.
			 */
			if (tst != 0)
			    break;
			depth++;
		    }
		}
		if (tst == 0) {
		    tst = results[j]->index > results[j + incr]->index;
		}
		if (tst > 0) {
		    tmp = results[j];
		    results[j] = results[j + incr];
		    results[j + incr] = tmp;
		    node = list->nodeTab[j];
		    list->nodeTab[j] = list->nodeTab[j + incr];
		    list->nodeTab[j + incr] = node;
		    depth = 1;
		    while (depth < nbsorts) {
			if (sorts[depth] == NULL)
			    break;
			if (resultsTab[depth] == NULL)
			    break;
			res = resultsTab[depth];
			tmp = res[j];
			res[j] = res[j + incr];
			res[j + incr] = tmp;
			depth++;
		    }
		    j -= incr;
		} else
		    break;
	    }
	}
    }

    for (j = 0; j < nbsorts; j++) {
	if (resultsTab[j] != NULL) {
	    for (i = 0;i < len;i++)
		xmlXPathFreeObject(resultsTab[j][i]);
	    xmlFree(resultsTab[j]);
	}
    }
}

/************************************************************************
 * 									*
 * 				Output					*
 * 									*
 ************************************************************************/

/**
 * xsltSaveResultTo:
 * @buf:  an output buffer
 * @result:  the result xmlDocPtr
 * @style:  the stylesheet
 *
 * Save the result @result obtained by applying the @style stylesheet
 * to an I/O output channel @buf
 *
 * Returns the number of byte written or -1 in case of failure.
 */
int
xsltSaveResultTo(xmlOutputBufferPtr buf, xmlDocPtr result,
	       xsltStylesheetPtr style) {
    const xmlChar *encoding;
    xmlNodePtr root;
    int base;
    const xmlChar *method;

    if ((buf == NULL) || (result == NULL) || (style == NULL))
	return(-1);
    if ((result->children == NULL) ||
	((result->children->type == XML_DTD_NODE) &&
	 (result->children->next == NULL)))
	return(0);

    if ((style->methodURI != NULL) &&
	((style->method == NULL) ||
	 (!xmlStrEqual(style->method, (const xmlChar *) "xhtml")))) {
        xsltGenericError(xsltGenericErrorContext,
		"xsltSaveResultTo : unknown ouput method\n");
        return(-1);
    }

    base = buf->written;

    XSLT_GET_IMPORT_PTR(method, style, method)
    XSLT_GET_IMPORT_PTR(encoding, style, encoding)

    if ((method == NULL) && (result->type == XML_HTML_DOCUMENT_NODE))
	method = (const xmlChar *) "html";

    if (method == NULL)
	root = xmlDocGetRootElement(result);
    else
	root = NULL;

    if ((method != NULL) &&
	(xmlStrEqual(method, (const xmlChar *) "html"))) {
	if (encoding != NULL) {
	    htmlSetMetaEncoding(result, (const xmlChar *) encoding);
	} else {
	    htmlSetMetaEncoding(result, (const xmlChar *) "UTF-8");
	}
	htmlDocContentDumpOutput(buf, result, (const char *) encoding);
	xmlOutputBufferFlush(buf);
    } else if ((method != NULL) &&
	(xmlStrEqual(method, (const xmlChar *) "xhtml"))) {
	if (encoding != NULL) {
	    htmlSetMetaEncoding(result, (const xmlChar *) encoding);
	} else {
	    htmlSetMetaEncoding(result, (const xmlChar *) "UTF-8");
	}
	htmlDocContentDumpOutput(buf, result, (const char *) encoding);
	xmlOutputBufferFlush(buf);
    } else if ((method != NULL) &&
	       (xmlStrEqual(method, (const xmlChar *) "text"))) {
	xmlNodePtr cur;

	cur = result->children;
	while (cur != NULL) {
	    if (cur->type == XML_TEXT_NODE)
		xmlOutputBufferWriteString(buf, (const char *) cur->content);

	    /*
	     * Skip to next node
	     */
	    if (cur->children != NULL) {
		if ((cur->children->type != XML_ENTITY_DECL) &&
		    (cur->children->type != XML_ENTITY_REF_NODE) &&
		    (cur->children->type != XML_ENTITY_NODE)) {
		    cur = cur->children;
		    continue;
		}
	    }
	    if (cur->next != NULL) {
		cur = cur->next;
		continue;
	    }
	    
	    do {
		cur = cur->parent;
		if (cur == NULL)
		    break;
		if (cur == (xmlNodePtr) style->doc) {
		    cur = NULL;
		    break;
		}
		if (cur->next != NULL) {
		    cur = cur->next;
		    break;
		}
	    } while (cur != NULL);
	}
	xmlOutputBufferFlush(buf);
    } else {
	int omitXmlDecl;
	int standalone;
	int indent;
	const xmlChar *version;

	XSLT_GET_IMPORT_INT(omitXmlDecl, style, omitXmlDeclaration);
	XSLT_GET_IMPORT_INT(standalone, style, standalone);
	XSLT_GET_IMPORT_INT(indent, style, indent);
	XSLT_GET_IMPORT_PTR(version, style, version)

	if (omitXmlDecl != 1) {
	    xmlOutputBufferWriteString(buf, "<?xml version=");
	    if (result->version != NULL) 
		xmlBufferWriteQuotedString(buf->buffer, result->version);
	    else
		xmlOutputBufferWriteString(buf, "\"1.0\"");
	    if (encoding == NULL) {
		if (result->encoding != NULL)
		    encoding = result->encoding;
		else if (result->charset != XML_CHAR_ENCODING_UTF8)
		    encoding = (const xmlChar *)
			       xmlGetCharEncodingName((xmlCharEncoding)
			                              result->charset);
	    }
	    if (encoding != NULL) {
		xmlOutputBufferWriteString(buf, " encoding=");
		xmlBufferWriteQuotedString(buf->buffer, (xmlChar *) encoding);
	    }
	    switch (standalone) {
		case 0:
		    xmlOutputBufferWriteString(buf, " standalone=\"no\"");
		    break;
		case 1:
		    xmlOutputBufferWriteString(buf, " standalone=\"yes\"");
		    break;
		default:
		    break;
	    }
	    xmlOutputBufferWriteString(buf, "?>\n");
	}
	if (result->children != NULL) {
	    xmlNodePtr child = result->children;

	    while (child != NULL) {
		xmlNodeDumpOutput(buf, result, child, 0, (indent == 1),
			          (const char *) encoding);
		if (child->type == XML_DTD_NODE)
		    xmlOutputBufferWriteString(buf, "\n");
		child = child->next;
	    }
	    xmlOutputBufferWriteString(buf, "\n");
	}
	xmlOutputBufferFlush(buf);
    }
    return(buf->written - base);
}

/**
 * xsltSaveResultToFilename:
 * @URL:  a filename or URL
 * @result:  the result xmlDocPtr
 * @style:  the stylesheet
 * @compression:  the compression factor (0 - 9 included)
 *
 * Save the result @result obtained by applying the @style stylesheet
 * to a file or URL @URL
 *
 * Returns the number of byte written or -1 in case of failure.
 */
int
xsltSaveResultToFilename(const char *URL, xmlDocPtr result,
			 xsltStylesheetPtr style, int compression) {
    xmlOutputBufferPtr buf;
    const xmlChar *encoding;
    int ret;

    if ((URL == NULL) || (result == NULL) || (style == NULL))
	return(-1);
    if (result->children == NULL)
	return(0);

    XSLT_GET_IMPORT_PTR(encoding, style, encoding)
    if (encoding != NULL) {
	xmlCharEncodingHandlerPtr encoder;

	encoder = xmlFindCharEncodingHandler((char *)encoding);
	if ((encoder != NULL) &&
	    (xmlStrEqual((const xmlChar *)encoder->name,
			 (const xmlChar *) "UTF-8")))
	    encoder = NULL;
	buf = xmlOutputBufferCreateFilename(URL, encoder, compression);
    } else {
	buf = xmlOutputBufferCreateFilename(URL, NULL, compression);
    }
    if (buf == NULL)
	return(-1);
    xsltSaveResultTo(buf, result, style);
    ret = xmlOutputBufferClose(buf);
    return(ret);
}

/**
 * xsltSaveResultToFile:
 * @file:  a FILE * I/O
 * @result:  the result xmlDocPtr
 * @style:  the stylesheet
 *
 * Save the result @result obtained by applying the @style stylesheet
 * to an open FILE * I/O.
 * This does not close the FILE @file
 *
 * Returns the number of bytes written or -1 in case of failure.
 */
int
xsltSaveResultToFile(FILE *file, xmlDocPtr result, xsltStylesheetPtr style) {
    xmlOutputBufferPtr buf;
    const xmlChar *encoding;
    int ret;

    if ((file == NULL) || (result == NULL) || (style == NULL))
	return(-1);
    if (result->children == NULL)
	return(0);

    XSLT_GET_IMPORT_PTR(encoding, style, encoding)
    if (encoding != NULL) {
	xmlCharEncodingHandlerPtr encoder;

	encoder = xmlFindCharEncodingHandler((char *)encoding);
	if ((encoder != NULL) &&
	    (xmlStrEqual((const xmlChar *)encoder->name,
			 (const xmlChar *) "UTF-8")))
	    encoder = NULL;
	buf = xmlOutputBufferCreateFile(file, encoder);
    } else {
	buf = xmlOutputBufferCreateFile(file, NULL);
    }

    if (buf == NULL)
	return(-1);
    xsltSaveResultTo(buf, result, style);
    ret = xmlOutputBufferClose(buf);
    return(ret);
}

/**
 * xsltSaveResultToFd:
 * @fd:  a file descriptor
 * @result:  the result xmlDocPtr
 * @style:  the stylesheet
 *
 * Save the result @result obtained by applying the @style stylesheet
 * to an open file descriptor
 * This does not close the descriptor.
 *
 * Returns the number of bytes written or -1 in case of failure.
 */
int
xsltSaveResultToFd(int fd, xmlDocPtr result, xsltStylesheetPtr style) {
    xmlOutputBufferPtr buf;
    const xmlChar *encoding;
    int ret;

    if ((fd < 0) || (result == NULL) || (style == NULL))
	return(-1);
    if (result->children == NULL)
	return(0);

    XSLT_GET_IMPORT_PTR(encoding, style, encoding)
    if (encoding != NULL) {
	xmlCharEncodingHandlerPtr encoder;

	encoder = xmlFindCharEncodingHandler((char *)encoding);
	if ((encoder != NULL) &&
	    (xmlStrEqual((const xmlChar *)encoder->name,
			 (const xmlChar *) "UTF-8")))
	    encoder = NULL;
	buf = xmlOutputBufferCreateFd(fd, encoder);
    } else {
	buf = xmlOutputBufferCreateFd(fd, NULL);
    }
    if (buf == NULL)
	return(-1);
    xsltSaveResultTo(buf, result, style);
    ret = xmlOutputBufferClose(buf);
    return(ret);
}

/************************************************************************
 * 									*
 * 		Generating profiling informations			*
 * 									*
 ************************************************************************/

static long calibration = -1;

/**
 * xsltCalibrateTimestamps:
 *
 * Used for to calibrate the xsltTimestamp() function
 * Should work if launched at startup and we don't loose our quantum :-)
 *
 * Returns the number of milliseconds used by xsltTimestamp()
 */
static long
xsltCalibrateTimestamps(void) {
    register int i;

    for (i = 0;i < 999;i++)
	xsltTimestamp();
    return(xsltTimestamp() / 1000);
}

/**
 * xsltCalibrateAdjust:
 * @delta:  a negative dealy value found
 *
 * Used for to correct the calibration for xsltTimestamp()
 */
void
xsltCalibrateAdjust(long delta) {
    calibration += delta;
}

/**
 * xsltTimestamp:
 *
 * Used for gathering profiling data
 *
 * Returns the number of tenth of milliseconds since the beginning of the
 * profiling
 */
long
xsltTimestamp(void)
{
#ifdef XSLT_WIN32_PERFORMANCE_COUNTER
    BOOL ok;
    LARGE_INTEGER performanceCount;
    LARGE_INTEGER performanceFrequency;
    LONGLONG quadCount;
    double seconds;
    static LONGLONG startupQuadCount = 0;
    static LONGLONG startupQuadFreq = 0;

    ok = QueryPerformanceCounter(&performanceCount);
    if (!ok)
        return 0;
    quadCount = performanceCount.QuadPart;
    if (calibration < 0) {
        calibration = 0;
        ok = QueryPerformanceFrequency(&performanceFrequency);
        if (!ok)
            return 0;
        startupQuadFreq = performanceFrequency.QuadPart;
        startupQuadCount = quadCount;
        return (0);
    }
    if (startupQuadFreq == 0)
        return 0;
    seconds = (quadCount - startupQuadCount) / (double) startupQuadFreq;
    return (long) (seconds * XSLT_TIMESTAMP_TICS_PER_SEC);

#else /* XSLT_WIN32_PERFORMANCE_COUNTER */
#ifdef HAVE_GETTIMEOFDAY
    static struct timeval startup;
    struct timeval cur;
    long tics;

    if (calibration < 0) {
        gettimeofday(&startup, NULL);
        calibration = 0;
        calibration = xsltCalibrateTimestamps();
        gettimeofday(&startup, NULL);
        return (0);
    }

    gettimeofday(&cur, NULL);
    tics = (cur.tv_sec - startup.tv_sec) * XSLT_TIMESTAMP_TICS_PER_SEC;
    tics += (cur.tv_usec - startup.tv_usec) /
                          (1000000l / XSLT_TIMESTAMP_TICS_PER_SEC);
    
    tics -= calibration;
    return(tics);
#else

    /* Neither gettimeofday() nor Win32 performance counter available */

    return (0);

#endif /* HAVE_GETTIMEOFDAY */
#endif /* XSLT_WIN32_PERFORMANCE_COUNTER */
}

#define MAX_TEMPLATES 10000

/**
 * xsltSaveProfiling:
 * @ctxt:  an XSLT context
 * @output:  a FILE * for saving the informations
 *
 * Save the profiling informations on @output
 */
void
xsltSaveProfiling(xsltTransformContextPtr ctxt, FILE *output) {
    int nb, i,j;
    int max;
    int total;
    long totalt;
    xsltTemplatePtr *templates;
    xsltStylesheetPtr style;
    xsltTemplatePtr template;

    if ((output == NULL) || (ctxt == NULL))
	return;
    if (ctxt->profile == 0)
	return;

    nb = 0;
    max = MAX_TEMPLATES;
    templates = xmlMalloc(max * sizeof(xsltTemplatePtr));
    if (templates == NULL)
	return;

    style = ctxt->style;
    while (style != NULL) {
	template = style->templates;
	while (template != NULL) {
	    if (nb >= max)
		break;

	    if (template->nbCalls > 0)
		templates[nb++] = template;
	    template = template->next;
	}

	style = xsltNextImport(style);
    }

    for (i = 0;i < nb -1;i++) {
	for (j = i + 1; j < nb; j++) {
	    if ((templates[i]->time <= templates[j]->time) ||
		((templates[i]->time == templates[j]->time) &&
	         (templates[i]->nbCalls <= templates[j]->nbCalls))) {
		template = templates[j];
		templates[j] = templates[i];
		templates[i] = template;
	    }
	}
    }

    fprintf(output, "%6s%20s%20s%10s  Calls Tot 100us Avg\n\n",
	    "number", "match", "name", "mode");
    total = 0;
    totalt = 0;
    for (i = 0;i < nb;i++) {
	fprintf(output, "%5d ", i);
	if (templates[i]->match != NULL) {
	    if (xmlStrlen(templates[i]->match) > 20)
		fprintf(output, "%s\n%26s", templates[i]->match, "");
	    else
		fprintf(output, "%20s", templates[i]->match);
	} else {
	    fprintf(output, "%20s", "");
	}
	if (templates[i]->name != NULL) {
	    if (xmlStrlen(templates[i]->name) > 20)
		fprintf(output, "%s\n%46s", templates[i]->name, "");
	    else
		fprintf(output, "%20s", templates[i]->name);
	} else {
	    fprintf(output, "%20s", "");
	}
	if (templates[i]->mode != NULL) {
	    if (xmlStrlen(templates[i]->mode) > 10)
		fprintf(output, "%s\n%56s", templates[i]->mode, "");
	    else
		fprintf(output, "%10s", templates[i]->mode);
	} else {
	    fprintf(output, "%10s", "");
	}
	fprintf(output, " %6d", templates[i]->nbCalls);
	fprintf(output, " %6ld %6ld\n", templates[i]->time,
		templates[i]->time / templates[i]->nbCalls);
	total += templates[i]->nbCalls;
	totalt += templates[i]->time;
    }
    fprintf(output, "\n%30s%26s %6d %6ld\n", "Total", "", total, totalt);

    xmlFree(templates);
}

