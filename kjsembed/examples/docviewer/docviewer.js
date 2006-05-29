
// Load the view
var viewer = new Form('docviewer.ui', this);
var text = viewer.findChild('textBrowser');
var list = viewer.findChild('listWidget');

var html = '<h1>Text View</h1><ul>';
for( i in text ) {
    html += '<li>' + i;
}
html += '</ul>';

html += '<h1>List Widget</h1><ul>';
for( i in list ) {
    html += '<li>' + i;
}
html += '</ul>';

html += '<h1>Globals</h1><ul>';
for( i in this ) {
    html += '<li>' + i;
}
html += '</ul>';

text.html = html;

viewer.show();
exec();
