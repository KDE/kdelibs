// XXX Automatically generated. DO NOT EDIT! XXX //

public:
GeoValue();
GeoValue(const GeoValue&);
GeoValue(const Q3CString&);
GeoValue & operator = (GeoValue&);
GeoValue & operator = (const Q3CString&);
bool operator ==(GeoValue&);
bool operator !=(GeoValue& x) {return !(*this==x);}
bool operator ==(const Q3CString& s) {GeoValue a(s);return(*this==a);} 
bool operator != (const Q3CString& s) {return !(*this == s);}

virtual ~GeoValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "GeoValue"; }

// End of automatically generated code           //
