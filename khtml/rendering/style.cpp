#include "style.h"

namespace DOM {

Style::Style()
{
    box.createData();
    border.createData();    
    text.createData();
    visual.createData();
    background.createData();
    list.createData();
    table.createData();

}

Style::Style(const Style& parent)
{    
    box = parent.box;
    border = parent.border;
    text = parent.text;
    visual = parent.visual;
    background = parent.background;
    list = parent.list;
    table = parent.table;
}

Style::~Style()
{    
}

} // namespace


