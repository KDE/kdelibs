<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
            	version="1.0">

<xsl:template name="header.navigation">
  <xsl:param name="prev" select="/foo"/>
  <xsl:param name="next" select="/foo"/>
  <xsl:variable name="home" select="/*[1]"/>
  <xsl:variable name="up" select="parent::*"/>

  <xsl:if test="$suppress.navigation = '0'">
      <table class="navheader" width="100%" border="0" cellpadding="0" cellspacing="0">
        <tr>
          <td class="bulb">
	    <img src="common/doctop1.png"
		 alt=""
		 width="150"
		 height="85">
	    </img>
	  </td>
	  <td
	      colspan="2"
	      height="85"
	      class="headline"
	  >
           <h1>
	     <xsl:apply-templates select="/" mode="title.ref"/>
           </h1> 
          </td>
 	</tr>
	<tr style="background-color: #515151;" class="navrow">
	  <td align="left" class="left" height="25">
	   <xsl:if test="count($prev)>0">
              <a>
                <xsl:attribute name="href">
                  <xsl:call-template name="href.target">
                    <xsl:with-param name="object" select="$prev"/>
                  </xsl:call-template>
                </xsl:attribute>
                <xsl:call-template name="gentext.nav.prev"/>
              </a>
            </xsl:if>
	  </td>
	  <td align="center" class="center" height="25">
            <xsl:choose>
              <xsl:when test="count($up) > 0 and $up != $home">
                <xsl:apply-templates select="$up" mode="title.ref"/>
              </xsl:when>
              <xsl:otherwise>&#160;</xsl:otherwise>
            </xsl:choose>
	  </td>
	  <td align="right" class="right" height="25">
            <xsl:if test="count($next)>0">
              <a>
                <xsl:attribute name="href">
                  <xsl:call-template name="href.target">
                    <xsl:with-param name="object" select="$next"/>
                  </xsl:call-template>
                </xsl:attribute>
                <xsl:call-template name="gentext.nav.next"/>
              </a>
            </xsl:if>
	 </td>
	</tr>
	<tr>
	  <td colspan="3" class="shadow">
	   <img src="common/shadow.png" width="100%" alt="Shadow" height="14"></img>
	  </td>
	</tr>
     </table>
  </xsl:if>
</xsl:template>

<!-- ==================================================================== -->

<xsl:template name="footer.navigation">
  <xsl:param name="prev" select="/foo"/>
  <xsl:param name="next" select="/foo"/>
  <xsl:variable name="home" select="/*[1]"/>
  <xsl:variable name="up" select="parent::*"/>

  <xsl:if test="$suppress.navigation = '0'">
    <div class="navfooter">
       <table width="100%" border="0" cellpading="0" cellspacing="0">
	 <colgroup>
	   <col width="*"></col><col width="227"></col>
	 </colgroup>
	 <tbody>
	  <tr>
	   <td class="end" width="100%">
	     <img src="common/bottom1.png" alt="Bottom" height="59"></img>
	   </td>
	   <td>
	    <a href="http://www.kde.org/">
	     <img src="common/bottom1.png" alt="KDE Logo" width="227" border="0" height="59"></img>
	    </a>
	   </td>
	  </tr>
	  <tr>
	   <td height="50%" colspan="2"><xsl:text>&#160;</xsl:text></td>
	  </tr>
         </tbody>
	</table>
        <table width="100%" border="0" cellpadding="0" cellspacing="0">
	 <tr>
          <td width="33%" align="left" valign="top">
            <xsl:if test="count($prev)>0">
              <a>
                <xsl:attribute name="href">
                  <xsl:call-template name="href.target">
                    <xsl:with-param name="object" select="$prev"/>
                  </xsl:call-template>
                </xsl:attribute>
                <xsl:call-template name="gentext.nav.prev"/>
              </a>
            </xsl:if>
          </td>
          <td width="34%" align="center" valign="top">
            <xsl:choose>
              <xsl:when test="$home != .">
                <a>
                  <xsl:attribute name="href">
                    <xsl:call-template name="href.target">
                      <xsl:with-param name="object" select="$home"/>
                    </xsl:call-template>
                  </xsl:attribute>
                  <xsl:call-template name="gentext.nav.home"/>
                </a>
              </xsl:when>
              <xsl:otherwise>&#160;</xsl:otherwise>
            </xsl:choose>
          </td>

          <td width="33%" align="right" valign="top">
            <xsl:if test="count($next)>0">
              <a>
                <xsl:attribute name="href">
                  <xsl:call-template name="href.target">
                    <xsl:with-param name="object" select="$next"/>
                  </xsl:call-template>
                </xsl:attribute>
                <xsl:call-template name="gentext.nav.next"/>
              </a>
            </xsl:if>
          </td>
        </tr>

      </table>
    </div>
  </xsl:if>
</xsl:template>

</xsl:stylesheet>
