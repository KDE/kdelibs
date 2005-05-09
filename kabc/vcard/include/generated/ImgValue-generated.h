// XXX Automatically generated. DO NOT EDIT! XXX //

public:
ImgValue();
ImgValue(const ImgValue&);
ImgValue(const Q3CString&);
ImgValue & operator = (ImgValue&);
ImgValue & operator = (const Q3CString&);
bool operator ==(ImgValue&);
bool operator !=(ImgValue& x) {return !(*this==x);}
bool operator ==(const Q3CString& s) {ImgValue a(s);return(*this==a);} 
bool operator != (const Q3CString& s) {return !(*this == s);}

virtual ~ImgValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
virtual const char * className() const { return "ImgValue"; }

// End of automatically generated code           //
