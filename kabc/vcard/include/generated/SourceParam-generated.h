// XXX Automatically generated. DO NOT EDIT! XXX //

public:
SourceParam();
SourceParam(const SourceParam&);
SourceParam(const Q3CString&);
SourceParam & operator = (SourceParam&);
SourceParam & operator = (const Q3CString&);
bool operator ==(SourceParam&);
bool operator !=(SourceParam& x) {return !(*this==x);}
bool operator ==(const Q3CString& s) {SourceParam a(s);return(*this==a);} 
bool operator != (const Q3CString& s) {return !(*this == s);}

virtual ~SourceParam();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "SourceParam"; }

// End of automatically generated code           //
