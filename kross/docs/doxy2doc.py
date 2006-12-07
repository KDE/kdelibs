#!/usr/bin/env python

"""
This python script creates a "scripting handbook" from doxygen XML files.

To generate the handbook;
    cd kspread/plugins/scripting/docs
    doxygen kspreadscripting.doxyfile
    cd xml
    xsltproc combine.xslt index.xml | ../doxy2doc.py ../kspread.html
"""

import os, sys, re, xml.dom.minidom

class Class:

    class Slot:
        def __init__(self, id, node):
            self.id = id
            self.node = node
            self.description = node.getElementsByTagName("detaileddescription")[0].toxml()

            d = self.node.getElementsByTagName("definition")[0].childNodes[0].data #e.g. "virtual QString sheet"
            d = d.replace("virtual ","")
            a = self.node.getElementsByTagName("argsstring")[0].childNodes[0].data #e.g. "(const QString &amp;name)"
            a = re.sub("=[\s]*0$","",a)
            a = re.sub("(^|[^a-zA-Z0-9])const($|[^a-zA-Z0-9])", "\\1\\2", "%s%s" % (d,a))
            a = re.sub("&|\*","",a)
            a = re.sub("[\s]*(\(|\))[\s]*","\\1",a)
            self.definition = a.strip()

    def __init__(self, node):
        self.node = node
        self.description = " ".join( [ n.toxml() for n in node.childNodes if n.nodeName == "detaileddescription" ] )
        self.memberDict = {}
        self.memberList = []
        for n in self.node.getElementsByTagName("memberdef"):
            id = n.getAttribute("id")
            kind = n.getAttribute("kind")
            if kind == "slot":
                self.memberDict[id] = Class.Slot(id, n)
                self.memberList.append(id)
                #print "  Adding class-member id=%s kind=%s" % (id,kind)
            else:
                print "  Skipping class-member id=%s kind=%s" % (id, kind)

class Page:
    def __init__(self, node):
        self.title = node.getElementsByTagName("title")[0].childNodes[0].data #e.g. "KSpread Scripting Plugin"
        self.description = " ".join( [ n.toxml() for n in node.childNodes if n.nodeName == "detaileddescription" ] )

class Writer:

    def __init__(self, doc):
        self.doc = doc
        self.CompoundDict = {}
        self.CompoundList = []

        for node in doc.getElementsByTagName("compounddef"):
            id = node.getAttribute("id")
            kind = node.getAttribute("kind") #e.g. "class"
            #prot = node.getAttribute("prot") #e.g. "public"
            name = node.getElementsByTagName("compoundname")[0].childNodes[0].data #e.g. "ScriptingPart"

            if name.startswith('Q') or name.startswith('KPart'):
                continue

            if kind == "class":
                impl = Class(node)
            elif kind == "page":
                impl = Page(node)
            else:
                print "Skipping id=%s name=%s kind=%s" % (id, name, kind)
                #raise RuntimeError("Unknown kind '%s'" % kind)
                continue
            impl.id = id
            impl.name = name
            impl.kind = kind
            self.CompoundDict[id] = impl
            self.CompoundList.append(id)

    def writeHtml(self, file):
        title = self.CompoundDict["indexpage"].title

        class HtmlParser:
            def __init__(self, writer): self.writer = writer
            def para(self, m): return "<%sp%s>" % (m.group(1),m.group(3))
            def sp(self, m): return "&nbsp;"
            def programlisting(self, m): return "<%spre%s>" % (m.group(1),m.group(3))
            def listitem(self, m): return "<%sli%s>" % (m.group(1),m.group(3))
            def ref(self, m):
                if m.group().__contains__('refid'):
                    refid = re.search("refid=\"(.*?)\"",m.group()).group(1)
                    if self.writer.CompoundDict.__contains__(refid):
                        if self.writer.CompoundDict[refid].kind in ["page","class"]:
                            return "<a href=\"#%s\">" % refid
                return "<%sa%s>" % (m.group(1),m.group(3))
                #raise AttributeError, "AAAAAAAAAAAAAAAAAAAAAA"
            def title(self, m):
                return "<%sh3%s>" % (m.group(1),m.group(3))

        parser = HtmlParser(self)
        def htmlReplacer( match ):
            tagname = match.group(2).strip()
            try:
                tagname = tagname[ : tagname.index(' ') ]
            except ValueError:
                pass
            if hasattr(parser,tagname):
                return getattr(parser,tagname)(match)
            #raise AttributeError, tagname
            return ""

        def parseToHtml( xmlstring ):
            xmlstring = re.compile( "<([\/\s]*)(.*?)([\/\s]*)>" ).sub(htmlReplacer, xmlstring)
            xmlstring = re.sub("(?<!\")((http|https|ftp)://[a-zA-Z0-9\.\_\-\;\?\&\/\=]*)", "<a href=\"\\1\">\\1</a>", xmlstring)
            xmlstring = re.sub("<li>[\s]*<p>","<li>", xmlstring)
            xmlstring = re.sub("</p>[\s]*</li>","</li>", xmlstring)
            return xmlstring

        file.write( "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n" )
        file.write( "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n" )
        file.write( "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">\n" )
        file.write( "<head><title>%s</title>" % title )
        file.write( "<style type=\"text/css\">" )
        file.write( "HTML { background-color:#eef; } ")
        file.write( "BODY { margin:1em; border:1em; padding:1em; font-size:100%; color:#003; background-color:#fff; border:#99a 1px solid; } ")
        file.write( "H1 { margin:0em 0em 1em 0em; font-size:1.5em; color:#009; text-align:center; } ")
        file.write( "H2 { margin:0em 0em 1em 0em; font-size:1.3em; color:#009; border-bottom:#699 1px dotted; } ")
        file.write( "H3,H4,H5,H6 { margin:2em 0em 1em 1em; font-size:1.1em; color:#009; border-bottom:#699 1px dotted; } ")
        file.write( "PRE { margin-left:1em; padding:0.5em; background-color:#f3f3ff; } ")
        file.write( ".member { margin:0.5em; background-color:#f3f3ff; } ")
        file.write( "</style>" )

        file.write( "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n" )
        file.write( "</head><body><h1>%s</h1>\n" % title )

        file.write("<ol>")

        file.write("<li><a href=\"#pages\">Pages</a><ol>")
        for i in self.CompoundList:
            if self.CompoundDict[i].kind != "page": continue
            file.write( "<li><a href=\"#%s\">%s</a></li>" % (i, self.CompoundDict[i].name) )
        file.write("</ol></li>")

        file.write("<li><a href=\"#objects\">Objects</a><ol>")
        for i in self.CompoundList:
            if self.CompoundDict[i].kind != "class": continue
            file.write( "<li><a href=\"#%s\">%s</a></li>" % (i, self.CompoundDict[i].name) )
        file.write("</ol></li>")

        file.write("</ol>")

        file.write("<h2><a name=\"indexpage\" />%s</h2>" % self.CompoundDict["indexpage"].name)
        file.write( parseToHtml( self.CompoundDict["indexpage"].description ) )

        file.write("<h2><a name=\"objects\" />Objects</h2>")
        for i in self.CompoundList:
            if self.CompoundDict[i].kind != "class": continue
            file.write("<h3><a name=\"%s\" />%s</h3>" % (i,self.CompoundDict[i].name))
            file.write( "%s<br />" % parseToHtml( self.CompoundDict[i].description ) )
            for m in self.CompoundDict[i].memberList:
                s = self.CompoundDict[i].memberDict[m].definition
                if len(self.CompoundDict[i].memberDict[m].description) > 0:
                    s += "<br /><blockquote>%s</blockquote>" % parseToHtml( self.CompoundDict[i].memberDict[m].description )
                file.write("<li class=\"member\">%s</li>" % s)

        file.write("</table></body></html>")

if __name__=="__main__":
    if len(sys.argv) != 2:
        print "%s outputfilename.xml" % sys.argv[0]
        sys.exit()

    outputfilename = sys.argv[1]
    doc = xml.dom.minidom.parse( sys.stdin )

    try:
        file = open(outputfilename, "w")
    except IOError, (errno, strerror):
        raise "Failed to create file \"%s\":\n%s" % (filename, strerror)

    writer = Writer(doc)
    writer.writeHtml(file)
