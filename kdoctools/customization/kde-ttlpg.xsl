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

<xsl:template match="author" mode="titlepage.mode">
  <h3 class="{name(.)}"><!--Documentation by--> <!-- to internationalise -->
    <xsl:call-template name="person.name"/>
  </h3>
  <xsl:apply-templates mode="titlepage.mode" select="./affiliation"/>
</xsl:template>

<!-- other things to be added:
disable email-link of author on title page, 
add date phrase: Last updated, 
revision?
move license under toc -->

<!-- Reduces affiliation to emailaddress -->
<xsl:template match="affiliation" mode="titlepage.mode">
    <xsl:apply-templates mode="titlepage.mode" select="./address/email"/>
</xsl:template>

<!-- Don't add a link to the author's email address on the page -->
<xsl:template match="email" mode="titlepage.mode">
  <xsl:call-template name="inline.monoseq">
    <xsl:with-param name="content">
      <xsl:text>&lt;</xsl:text>
       <xsl:apply-templates/>
      <xsl:text>&gt;</xsl:text>
    </xsl:with-param>
  </xsl:call-template>
</xsl:template>


<xsl:template match="othercredit" mode="titlepage.mode">
  <p class="{name(.)}">
  <xsl:apply-templates mode="titlepage.mode" select="./contrib"/>
  <xsl:text>: </xsl:text>
  <a><!-- removed the name="./@id" attribute: it was wrong and I don't understand what it's supposed to do -->
    <xsl:call-template name="person.name"/>
  </a>
  </p>
</xsl:template>

<xsl:template match="contrib" mode="titlepage.mode">
  <span class="{name(.)}">
    <xsl:apply-templates mode="titlepage.mode"/>
  </span>
</xsl:template>

<xsl:template match="abstract" mode="titlepage.mode">
  <div>
    <xsl:call-template name="semiformal.object"/>
  </div>
</xsl:template>

<xsl:template match="abstract/title" mode="titlepage.mode">
</xsl:template>

</xsl:stylesheet>
