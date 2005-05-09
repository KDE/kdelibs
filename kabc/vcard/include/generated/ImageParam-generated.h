// XXX Automatically generated. DO NOT EDIT! XXX //

public:
ImageParam();
ImageParam(const ImageParam&);
ImageParam(const Q3CString&);
ImageParam & operator = (ImageParam&);
ImageParam & operator = (const Q3CString&);
bool operator ==(ImageParam&);
bool operator !=(ImageParam& x) {return !(*this==x);}
bool operator ==(const Q3CString& s) {ImageParam a(s);return(*this==a);} 
bool operator != (const Q3CString& s) {return !(*this == s);}

virtual ~ImageParam();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "ImageParam"; }

// End of automatically generated code           //
