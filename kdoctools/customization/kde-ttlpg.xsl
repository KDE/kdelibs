<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<xsl:template match="releaseinfo" mode="titlepage.mode">
  <xsl:call-template name="gentext">
    <xsl:with-param name="key">revision</xsl:with-param>
  </xsl:call-template>
  <xsl:text> </xsl:text>
  <span class="{name(.)}">
    <xsl:apply-templates mode="titlepage.mode"/>
    <br/>
  </span>
</xsl:template>

<!-- turn off these from displaying, they just clutter it for the user -->

<xsl:template match="author" mode="titlepage.mode">
</xsl:template>

<xsl:template match="othercredit" mode="titlepage.mode">
</xsl:template>

<xsl:template match="contrib" mode="titlepage.mode">
</xsl:template>

<xsl:template match="copyright" mode="titlepage.mode">
</xsl:template>

<xsl:template match="abstract" mode="titlepage.mode">
  <div>
    <xsl:call-template name="semiformal.object"/>
  </div>
</xsl:template>

<xsl:template match="abstract/title" mode="titlepage.mode">
</xsl:template>

</xsl:stylesheet>
