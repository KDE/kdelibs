#ifndef RENDER_HTML
#define RENDER_HTML

#include "render_flow.h"

namespace khtml {
    
    class RenderHtml : public RenderFlow 
    {
    public:
	RenderHtml();
	virtual ~RenderHtml();
	
	virtual const char *renderName() const { return "RenderHtml"; }
	
	virtual void print( QPainter *, int x, int y, int w, int h, int tx, int ty);
	virtual void repaint();
    protected:
	virtual void printBoxDecorations(QPainter *p,int _x, int _y,
					 int _w, int _h, int _tx, int _ty);
 
    };
};
#endif
