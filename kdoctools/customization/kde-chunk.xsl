<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		version="1.0">

<xsl:import href="../docbook/xsl/html/chunk-common.xsl"/>
<xsl:include href="kde-navig.xsl"/>
<xsl:include href="kde-ttlpg.xsl"/>
<xsl:include href="kde-style.xsl"/>

<xsl:template name="make-relative-filename">
  <xsl:param name="base.dir" select="'./'"/>
  <xsl:param name="base.name" select="''"/>

  <!-- XT makes chunks relative -->
  <xsl:choose>
    <xsl:when test="count(parent::*) = 0">
      <xsl:value-of select="concat($base.dir,$base.name)"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$base.name"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name="write.chunk">
  <xsl:param name="filename" select="''"/>
  <xsl:param name="method" select="'html'"/>
  <xsl:param name="encoding" select="'utf-8'"/>
  <xsl:param name="content" select="''"/>
  
  <FILENAME filename="{$filename}"> 
  <xsl:copy-of select="$content"/>
  </FILENAME>
</xsl:template>

<xsl:param name="use.id.as.filename">1</xsl:param>
<xsl:param name="generate.section.toc">1</xsl:param>
<xsl:param name="use.extensions">0</xsl:param>
<xsl:param name="html.stylesheet">common/kde-default.css</xsl:param>

<xsl:template name="dbhtml-filename">
<xsl:choose>
     <xsl:when test=". != /*">
      <xsl:value-of select="@id"/>
      <xsl:value-of select="$html.ext"/>
     </xsl:when>
     <xsl:otherwise>
	<xsl:text>index.html</xsl:text>
      </xsl:otherwise>
</xsl:choose>
</xsl:template>

<!-- libxslt has problems with customizing this one, so 
    I changed the docbook xsl itself for now
    (FF) body attrs are set to kde ones: attributes cannot be "unset"
         set a new attributes-set must be defined

<xsl:template name="chunk-element-content">
  <xsl:param name="prev"></xsl:param>
  <xsl:param name="next"></xsl:param>
 
  <html>
    <xsl:call-template name="html.head">
      <xsl:with-param name="prev" select="$prev"/>
      <xsl:with-param name="next" select="$next"/>
    </xsl:call-template>
 
    <body xsl:use-attribute-sets="kde.body.attrs">
      <xsl:call-template name="header.navigation">
        <xsl:with-param name="prev" select="$prev"/>
        <xsl:with-param name="next" select="$next"/>
      </xsl:call-template>
 
      <xsl:call-template name="user.header.content"/>

      <div id="body_text"> 
        <xsl:apply-imports/>
      </div>

      <xsl:call-template name="user.footer.content"/>
 
      <xsl:call-template name="footer.navigation">
        <xsl:with-param name="prev" select="$prev"/>
        <xsl:with-param name="next" select="$next"/>
      </xsl:call-template>
    </body>
  </html>
</xsl:template>

-->

<!-- try with olinks: it nearly works --><!--
  <xsl:template match="olink">
    <a>
      <xsl:attribute name="href">
	<xsl:choose>
	  <xsl:when test="@type = 'kde-installation'">
	    <xsl:choose>
	      <xsl:when test="@linkmode = 'kdems-man'">
		<xsl:value-of select="id(@linkmode)"/>
		<xsl:value-of select="@targetdocent"/>
		<xsl:text>(</xsl:text>
		<xsl:value-of select="@localinfo"/>
		<xsl:text>)</xsl:text>
	      </xsl:when>
	      <xsl:when test="@linkmode = 'kdems-help'">
		<xsl:value-of select="id(@linkmode)"/>
		<xsl:text>/</xsl:text>
		<xsl:value-of select="@targetdocent"/>
<xsl:variable name="targetdocent" select="@targetdocent"/>
<xsl:value-of select="$targetdocent"/>
          <xsl:if test="@targetdocent">
            <xsl:value-of select="unparsed-entity-uri(string($targetdocent))"/>
          </xsl:if>
                <xsl:for-each select="document('/home/fouvry/kdeutils/doc/kedit/index.docbook')">
		  <xsl:value-of select=".//*[@id=$localinfo]"/>
                </xsl:for-each>
		<xsl:text>#</xsl:text>
		<xsl:value-of select="@localinfo"/>
	      </xsl:when>
	    </xsl:choose>
	  </xsl:when>
	</xsl:choose>
      </xsl:attribute>
      <xsl:value-of select="."/>
    </a>
  </xsl:template>
-->
</xsl:stylesheet>
