/*
 * attributes.c: Implementation of the XSLT attributes handling
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

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_MATH_H
#include <math.h>
#endif
#ifdef HAVE_FLOAT_H
#include <float.h>
#endif
#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif
#ifdef HAVE_NAN_H
#include <nan.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif

#include <libxml/xmlmemory.h>
#include <libxml/tree.h>
#include <libxml/hash.h>
#include <libxml/xmlerror.h>
#include <libxml/uri.h>
#include "xslt.h"
#include "xsltInternals.h"
#include "xsltutils.h"
#include "attributes.h"
#include "namespaces.h"
#include "templates.h"
#include "imports.h"
#include "transform.h"

#ifdef WITH_XSLT_DEBUG
#define WITH_XSLT_DEBUG_ATTRIBUTES
#endif

/*
 * TODO: merge attribute sets from different import precedence.
 *       all this should be precomputed just before the transformation
 *       starts or at first hit with a cache in the context.
 *       The simple way for now would be to not allow redefinition of
 *       attributes once generated in the output tree, possibly costlier.
 */

/*
 * Useful macros
 */

#define IS_BLANK(c) (((c) == 0x20) || ((c) == 0x09) || ((c) == 0xA) ||	\
                     ((c) == 0x0D))

#define IS_BLANK_NODE(n)						\
    (((n)->type == XML_TEXT_NODE) && (xsltIsBlank((n)->content)))


/*
 * The in-memory structure corresponding to an XSLT Attribute in
 * an attribute set
 */

typedef struct _xsltAttrElem xsltAttrElem;
typedef xsltAttrElem *xsltAttrElemPtr;
struct _xsltAttrElem {
    struct _xsltAttrElem *next;/* chained list */
    xmlNodePtr attr;	/* the xsl:attribute definition */
};

/************************************************************************
 *									*
 *			XSLT Attribute handling				*
 *									*
 ************************************************************************/

/**
 * xsltNewAttrElem:
 * @attr:  the new xsl:attribute node
 *
 * Create a new XSLT AttrElem
 *
 * Returns the newly allocated xsltAttrElemPtr or NULL in case of error
 */
static xsltAttrElemPtr
xsltNewAttrElem(xmlNodePtr attr) {
    xsltAttrElemPtr cur;

    cur = (xsltAttrElemPtr) xmlMalloc(sizeof(xsltAttrElem));
    if (cur == NULL) {
        xsltGenericError(xsltGenericErrorContext,
		"xsltNewAttrElem : malloc failed\n");
	return(NULL);
    }
    memset(cur, 0, sizeof(xsltAttrElem));
    cur->attr = attr;
    return(cur);
}

/**
 * xsltFreeAttrElem:
 * @attr:  an XSLT AttrElem
 *
 * Free up the memory allocated by @attr
 */
static void
xsltFreeAttrElem(xsltAttrElemPtr attr) {
    memset(attr, -1, sizeof(xsltAttrElem));
    xmlFree(attr);
}

/**
 * xsltFreeAttrElemList:
 * @list:  an XSLT AttrElem list
 *
 * Free up the memory allocated by @list
 */
static void
xsltFreeAttrElemList(xsltAttrElemPtr list) {
    xsltAttrElemPtr next;
    
    while (list != NULL) {
	next = list->next;
	xsltFreeAttrElem(list);
	list = next;
    }
}

/**
 * xsltAddAttrElemList:
 * @list:  an XSLT AttrElem list
 * @attr:  the new xsl:attribute node
 *
 * Add the new attribute to the list.
 *
 * Returns the new list pointer
 */
static xsltAttrElemPtr
xsltAddAttrElemList(xsltAttrElemPtr list, xmlNodePtr attr) {
    xsltAttrElemPtr next, cur;

    if (attr == NULL)
	return(list);
    if (list == NULL)
	return(xsltNewAttrElem(attr));
    cur = list;
    while (cur != NULL) {
	next = cur->next;
	if (cur->attr == attr)
	    return(cur);
	if (cur->next == NULL) {
	    cur->next = xsltNewAttrElem(attr);
	}
	cur = next;
    }
    return(list);
}

/**
 * xsltMergeAttrElemList:
 * @list:  an XSLT AttrElem list
 * @old:  another XSLT AttrElem list
 *
 * Add all the attributes from list @old to list @list,
 * but drop redefinition of existing values.
 *
 * Returns the new list pointer
 */
static xsltAttrElemPtr
xsltMergeAttrElemList(xsltAttrElemPtr list, xsltAttrElemPtr old) {
    xsltAttrElemPtr cur;
    int add;

    while (old != NULL) {

	/*
	 * Check that the attribute is not yet in the list
	 */
	cur = list;
	add = 1;
	while (cur != NULL) {
	    if (cur->attr == old->attr) {
		xsltGenericError(xsltGenericErrorContext,
	     "xsl:attribute-set : use-attribute-sets recursion detected\n");
		return(list);
	    }
	    if (xmlStrEqual(cur->attr->name, old->attr->name)) {
		if (cur->attr->ns == old->attr->ns) {
		    add = 0;
		    break;
		}
		if ((cur->attr->ns != NULL) && (old->attr->ns != NULL) &&
		    (xmlStrEqual(cur->attr->ns->href, old->attr->ns->href))) {
		    add = 0;
		    break;
		}
	    }
	    if (cur->next == NULL)
		break;
            cur = cur->next;
	}

	if (cur == NULL) {
	    list = xsltNewAttrElem(old->attr);
	} else if (add) {
	    cur->next = xsltNewAttrElem(old->attr);
	}

	old = old->next;
    }
    return(list);
}

/************************************************************************
 *									*
 *			Module interfaces				*
 *									*
 ************************************************************************/

/**
 * xsltParseStylesheetAttributeSet:
 * @style:  the XSLT stylesheet
 * @template:  the "preserve-space" element
 *
 * parse an XSLT stylesheet preserve-space element and record
 * elements needing preserving
 */

void
xsltParseStylesheetAttributeSet(xsltStylesheetPtr style, xmlNodePtr cur) {
    xmlChar *prop = NULL;
    xmlChar *ncname = NULL;
    xmlChar *prefix = NULL;
    xmlChar *attributes;
    xmlChar *attrib, *endattr;
    xmlNodePtr list;
    xsltAttrElemPtr values;

    if ((cur == NULL) || (style == NULL))
	return;

    prop = xsltGetNsProp(cur, (const xmlChar *)"name", XSLT_NAMESPACE);
    if (prop == NULL) {
	xsltGenericError(xsltGenericErrorContext,
	     "xsl:attribute-set : name is missing\n");
	goto error;
    }

    ncname = xmlSplitQName2(prop, &prefix);
    if (ncname == NULL) {
	ncname = prop;
	prop = NULL;
	prefix = NULL;
    }

    if (style->attributeSets == NULL) {
#ifdef WITH_XSLT_DEBUG_ATTRIBUTES
	xsltGenericDebug(xsltGenericDebugContext,
	    "creating attribute set table\n");
#endif
	style->attributeSets = xmlHashCreate(10);
    }
    if (style->attributeSets == NULL)
	goto error;

    values = xmlHashLookup2(style->attributeSets, ncname, prefix);

    /*
     * check the children list
     */
    list = cur->children;
    while (list != NULL) {
	if (IS_XSLT_ELEM(list)) {
	    if (!IS_XSLT_NAME(list, "attribute")) {
		xsltGenericError(xsltGenericErrorContext,
		    "xsl:attribute-set : unexpected child xsl:%s\n",
		                 list->name);
	    } else {
#ifdef WITH_XSLT_DEBUG_ATTRIBUTES
		xsltGenericDebug(xsltGenericDebugContext,
		    "add attribute to list %s\n", ncname);
#endif
                values = xsltAddAttrElemList(values, list);
	    }
	} else {
	    xsltGenericError(xsltGenericErrorContext,
		"xsl:attribute-set : unexpected child %s\n", list->name);
	}
	list = list->next;
    }

    /*
     * Check a possible use-attribute-sets definition
     */
    /* TODO check recursion */

    attributes = xsltGetNsProp(cur, (const xmlChar *)"use-attribute-sets",
	                      XSLT_NAMESPACE);
    if (attributes == NULL) {
	goto done;
    }

    attrib = attributes;
    while (*attrib != 0) {
	while (IS_BLANK(*attrib)) attrib++;
	if (*attrib == 0)
	    break;
        endattr = attrib;
	while ((*endattr != 0) && (!IS_BLANK(*endattr))) endattr++;
	attrib = xmlStrndup(attrib, endattr - attrib);
	if (attrib) {
	    xmlChar *ncname2 = NULL;
	    xmlChar *prefix2 = NULL;
	    xsltAttrElemPtr values2;
#ifdef WITH_XSLT_DEBUG_ATTRIBUTES
	    xsltGenericDebug(xsltGenericDebugContext,
		"xsl:attribute-set : %s adds use %s\n", ncname, attrib);
#endif
	    ncname2 = xmlSplitQName2(attrib, &prefix2);
	    if (ncname2 == NULL) {
		ncname2 = attrib;
		attrib = NULL;
		prefix = NULL;
	    }
	    values2 = xmlHashLookup2(style->attributeSets, ncname2, prefix2);
	    values = xsltMergeAttrElemList(values, values2);

	    if (attrib != NULL)
		xmlFree(attrib);
	    if (ncname2 != NULL)
		xmlFree(ncname2);
	    if (prefix2 != NULL)
		xmlFree(prefix2);
	}
	attrib = endattr;
    }
    xmlFree(attributes);

done:
    /*
     * Update the value
     */
    xmlHashUpdateEntry2(style->attributeSets, ncname, prefix, values, NULL);
#ifdef WITH_XSLT_DEBUG_ATTRIBUTES
    xsltGenericDebug(xsltGenericDebugContext,
	"updated attribute list %s\n", ncname);
#endif

error:
    if (prop != NULL)
        xmlFree(prop);
    if (ncname != NULL)
        xmlFree(ncname);
    if (prefix != NULL)
        xmlFree(prefix);
}

/**
 * xsltAttributeInternal:
 * @ctxt:  a XSLT process context
 * @node:  the node in the source tree.
 * @inst:  the xslt attribute node
 * @comp:  precomputed information
 * @fromset:  the attribute comes from an attribute-set
 *
 * Process the xslt attribute node on the source node
 */
static void
xsltAttributeInternal(xsltTransformContextPtr ctxt, xmlNodePtr node,
	      xmlNodePtr inst, xsltStylePreCompPtr comp, int fromset) {
    xmlChar *prop = NULL;
    xmlChar *ncname = NULL, *name, *namespace;
    xmlChar *prefix = NULL;
    xmlChar *value = NULL;
    xmlNsPtr ns = NULL;
    xmlAttrPtr attr;
    const xmlChar *URL = NULL;


    if (ctxt->insert == NULL)
	return;
    if (comp == NULL) {
	xsltPrintErrorContext(ctxt, NULL, inst);
	xsltGenericError(xsltGenericErrorContext,
	     "xsl:attribute : compilation failed\n");
	return;
    }

    if ((ctxt == NULL) || (node == NULL) || (inst == NULL) || (comp == NULL))
	return;
    if (!comp->has_name) {
	return;
    }
    if (ctxt->insert->children != NULL) {
	xsltPrintErrorContext(ctxt, NULL, inst);
	xsltGenericError(xsltGenericErrorContext,
	     "xsl:attribute : node already has children\n");
	return;
    }
    if (comp->name == NULL) {
	prop = xsltEvalAttrValueTemplate(ctxt, inst, (const xmlChar *)"name",
		                         XSLT_NAMESPACE);
	if (prop == NULL) {
	    xsltPrintErrorContext(ctxt, NULL, inst);
	    xsltGenericError(xsltGenericErrorContext,
		 "xsl:attribute : name is missing\n");
	    goto error;
	}
	name = prop;
    } else {
	name = comp->name;
    }

    ncname = xmlSplitQName2(name, &prefix);
    if (ncname == NULL) {
	prefix = NULL;
    } else {
	name = ncname;
    }
    if (!xmlStrncasecmp(prefix, (xmlChar *)"xml", 3)) {
#ifdef WITH_XSLT_DEBUG_PARSING
	xsltGenericDebug(xsltGenericDebugContext,
	     "xsltAttribute: xml prefix forbidden\n");
#endif
	goto error;
    }
    if ((comp->ns == NULL) && (comp->has_ns)) {
	namespace = xsltEvalAttrValueTemplate(ctxt, inst,
		(const xmlChar *)"namespace", XSLT_NAMESPACE);
	if (namespace != NULL) {
	    ns = xsltGetSpecialNamespace(ctxt, inst, namespace, prefix,
		                         ctxt->insert);
	    xmlFree(namespace);
	} else {
	    if (prefix != NULL) {
		ns = xmlSearchNs(inst->doc, inst, prefix);
		if (ns == NULL) {
		    xsltPrintErrorContext(ctxt, NULL, inst);
		    xsltGenericError(xsltGenericErrorContext,
			"xsl:attribute : no namespace bound to prefix %s\n", prefix);
		} else {
		    ns = xsltGetNamespace(ctxt, inst, ns, ctxt->insert);
		}
	    }
	}
    } else if (comp->ns != NULL) {
	ns = xsltGetSpecialNamespace(ctxt, inst, comp->ns, prefix,
				     ctxt->insert);
    }

    if ((fromset) && (ns != NULL))
	URL = ns->href;
    if ((fromset == 0) || (!xmlHasNsProp(ctxt->insert, name, URL))) {
	value = xsltEvalTemplateString(ctxt, node, inst);
	if (value == NULL) {
	    if (ns) {
		attr = xmlSetNsProp(ctxt->insert, ns, name, 
				    (const xmlChar *)"");
	    } else {
		attr = xmlSetProp(ctxt->insert, name, (const xmlChar *)"");
	    }
	} else {
	    if (ns) {
		attr = xmlSetNsProp(ctxt->insert, ns, name, value);
	    } else {
		attr = xmlSetProp(ctxt->insert, name, value);
	    }
	}
    }

error:
    if (prop != NULL)
        xmlFree(prop);
    if (ncname != NULL)
        xmlFree(ncname);
    if (prefix != NULL)
        xmlFree(prefix);
    if (value != NULL)
        xmlFree(value);
}

/**
 * xsltAttribute:
 * @ctxt:  a XSLT process context
 * @node:  the node in the source tree.
 * @inst:  the xslt attribute node
 * @comp:  precomputed information
 *
 * Process the xslt attribute node on the source node
 */
void
xsltAttribute(xsltTransformContextPtr ctxt, xmlNodePtr node,
	      xmlNodePtr inst, xsltStylePreCompPtr comp) {
    xsltAttributeInternal(ctxt, node, inst, comp, 0);
}

/**
 * xsltApplyAttributeSet:
 * @ctxt:  the XSLT stylesheet
 * @node:  the node in the source tree.
 * @inst:  the xslt attribute node
 * @attributes:  the set list.
 *
 * Apply the xsl:use-attribute-sets
 */

void
xsltApplyAttributeSet(xsltTransformContextPtr ctxt, xmlNodePtr node,
	              xmlNodePtr inst ATTRIBUTE_UNUSED, xmlChar *attributes) {
    xmlChar *ncname = NULL;
    xmlChar *prefix = NULL;
    xmlChar *attrib, *endattr;
    xsltAttrElemPtr values;
    xsltStylesheetPtr style;

    if (attributes == NULL) {
	return;
    }

    attrib = attributes;
    while (*attrib != 0) {
	while (IS_BLANK(*attrib)) attrib++;
	if (*attrib == 0)
	    break;
        endattr = attrib;
	while ((*endattr != 0) && (!IS_BLANK(*endattr))) endattr++;
	attrib = xmlStrndup(attrib, endattr - attrib);
	if (attrib) {
#ifdef WITH_XSLT_DEBUG_ATTRIBUTES
	    xsltGenericDebug(xsltGenericDebugContext,
		"apply attribute set %s\n", attrib);
#endif
	    ncname = xmlSplitQName2(attrib, &prefix);
	    if (ncname == NULL) {
		ncname = attrib;
		attrib = NULL;
		prefix = NULL;
	    }

	    style = ctxt->style;
	    while (style != NULL) {
		values = xmlHashLookup2(style->attributeSets, ncname, prefix);
		while (values != NULL) {
		    if (values->attr != NULL) {
			xsltAttributeInternal(ctxt, node, values->attr,
					      values->attr->_private, 1);
		    }
		    values = values->next;
		}
		style = xsltNextImport(style);
	    }
	    if (attrib != NULL)
		xmlFree(attrib);
	    if (ncname != NULL)
		xmlFree(ncname);
	    if (prefix != NULL)
		xmlFree(prefix);
	}
	attrib = endattr;
    }
}

/**
 * xsltFreeAttributeSetsHashes:
 * @style: an XSLT stylesheet
 *
 * Free up the memory used by attribute sets
 */
void
xsltFreeAttributeSetsHashes(xsltStylesheetPtr style) {
    if (style->attributeSets != NULL)
	xmlHashFree((xmlHashTablePtr) style->attributeSets,
		    (xmlHashDeallocator) xsltFreeAttrElemList);
    style->attributeSets = NULL;
}
