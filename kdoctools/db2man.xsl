<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version='1.0'>

<xsl:output method="text"
            encoding="ISO-8859-1"
            indent="no"/>

<xsl:template match="section">
  <xsl:text>.SH "</xsl:text>
  <xsl:value-of select="title[1]"/>
  <xsl:text>"
</xsl:text>
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="section/title"></xsl:template>

<xsl:template match="para">
     <xsl:text>.PP
</xsl:text>
     <xsl:apply-templates/>
</xsl:template>

<xsl:template match="*">
   <xsl:apply-templates/>
</xsl:template>

<xsl:template match="article">
  <xsl:text>." DO NOT MODIFY THIS FILE!
.TH "</xsl:text>
  <xsl:value-of select="title[1]"/>
  <xsl:text>" "</xsl:text>
  <xsl:value-of select="articleinfo/date[1]"/>
  <xsl:text>"
</xsl:text>
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="article/title"></xsl:template>
<xsl:template match="article/articleinfo/*"></xsl:template>

<xsl:template match="term/option">
  <xsl:text>\fB</xsl:text><xsl:apply-templates/><xsl:text>\fR</xsl:text>
</xsl:template>

<xsl:template match="varlistentry">
  <xsl:text>.TP
</xsl:text>
<xsl:apply-templates select="./term"/>
<xsl:apply-templates select="./listitem"/>
</xsl:template>

<xsl:template match="varlistentry/listitem/para">
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="filename">
  <xsl:text>.FN </xsl:text><xsl:apply-templates/>
</xsl:template>

<xsl:template match="userinput">
  <xsl:text>.IP
.B </xsl:text><xsl:apply-templates/>
</xsl:template>

<xsl:template match="envar">
  <xsl:text>\fB</xsl:text><xsl:apply-templates/><xsl:text>\fR</xsl:text>
</xsl:template>

<xsl:template match="filename">
  <xsl:text>\fI</xsl:text><xsl:apply-templates/><xsl:text>\fR</xsl:text>
</xsl:template>

</xsl:stylesheet>
