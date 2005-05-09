// XXX Automatically generated. DO NOT EDIT! XXX //

public:
TextParam();
TextParam(const TextParam&);
TextParam(const Q3CString&);
TextParam & operator = (TextParam&);
TextParam & operator = (const Q3CString&);
bool operator ==(TextParam&);
bool operator !=(TextParam& x) {return !(*this==x);}
bool operator ==(const Q3CString& s) {TextParam a(s);return(*this==a);} 
bool operator != (const Q3CString& s) {return !(*this == s);}

virtual ~TextParam();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "TextParam"; }

// End of automatically generated code           //
