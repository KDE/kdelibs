
// Load the view
var viewer = new Form('docviewer.ui', this);
var text = viewer.findChild('textBrowser');

var html = '<h1>KJSEmbed Documentation</h1>';

// Generate TOC
html += '<h2>Contents</h2><ul>'
for( i in this ) {
    html += '<li><a href="' + i + '">' + i + '</a>';
}
html += '</ul>';

// Object Docs
for( i in this ) {
    html += '<a name="' + i + '"><h2>' + i + '</h2></a><ul>';

    // Create an instance
    try {
      var inst = eval( 'new ' + i + '()' );
    
      for( j in inst ) {
	html += '<li>' + j;
      }
      html += '</ul>';
    }
    catch( err ) {
      html += '<li>Could not create</ul>';
    }
}

text.html = html;

viewer.show();
exec();
