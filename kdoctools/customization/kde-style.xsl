<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		version="1.0">

<xsl:template match="guilabel|guimenu|guisubmenu|guimenuitem|interface|guibutton">
  <span style="background-color: rgb(240,240,240); 
	color: rgb(0,0,0);">  
     <xsl:call-template name="inline.charseq"/>
  </span>
</xsl:template>

<xsl:template match="accel">
  <span style="background-color: rgb(240,240,240); 
	color: rgb(0,0,0); 
	text-decoration: underline;">  
     <xsl:call-template name="inline.charseq"/>
   </span>
</xsl:template>

<xsl:attribute-set name="kde.body.attrs">
</xsl:attribute-set>
</xsl:stylesheet>
