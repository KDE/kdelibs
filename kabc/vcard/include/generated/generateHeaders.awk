#!/bin/awk -f

{
	outfile = $1 "-generated.h"
	name = $1
	
	OFS=""
	
	print "// XXX Automatically generated. DO NOT EDIT! XXX //\n" > outfile
	
	if ($2 == "v") { pre = "virtual " } else { pre = "" }
	
	print "public:" >> outfile
	print name "();" >> outfile
	print name "(const " name "&);" >> outfile
	print name "(const QCString&);" >> outfile
	print pre name " & operator = (" name "&);" >> outfile
	print pre name " & operator = (const QCString&);" >> outfile
	print pre "bool operator ==(" name "&);" >> outfile
	print pre "bool operator !=(" name "& x) {return !(*this==x);}" \
			>> outfile
	print pre "bool operator ==(const QCString& s) {" name " a(s);" \
			"return(*this==a);} " >> outfile
	print pre "bool operator != (const QCString& s) {return !(*this == s);}\n" \
			>> outfile
	print "virtual ~" name "();" >> outfile
	print pre "void parse() " \
			"{if(!parsed_) _parse();parsed_=true;assembled_=false;}\n" \
			>> outfile
	print pre "void assemble() " \
			"{if(assembled_) return;parse();_assemble();assembled_=true;}\n" \
			>> outfile
	print pre "void _parse();" >> outfile
	print pre "void _assemble();" >> outfile
	print pre "const char * className() const { return \"" name "\"; }" \
		>> outfile
	
	print "\n// End of automatically generated code           //" >> outfile
}

