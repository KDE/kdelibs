<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
            	version="1.0">


<xsl:template name="header.navigation">
  <xsl:param name="prev" select="/foo"/>
  <xsl:param name="next" select="/foo"/>
  <xsl:variable name="home" select="/*[1]"/>
  <xsl:variable name="up" select="parent::*"/>

  <xsl:if test="$suppress.navigation = '0'">

  <table width="596" height="110" background="{$kde.common}web-doctop.png" class="topbanner">
    <tr>
       <td width="145" height="80"> </td>
       <td colspan="2" width="451" height="80">
	<h1><xsl:apply-templates select="." mode="title.markup"/></h1>
       </td>
    </tr>	 
    <tr>
       <td align="left" class="left" width="145" height="30">          
	<xsl:if test="count($prev)>0">
          <a accesskey="p">
            <xsl:attribute name="href">
              <xsl:call-template name="href.target">
                <xsl:with-param name="object" select="$prev"/>
              </xsl:call-template>
            </xsl:attribute>
           <xsl:call-template name="gentext.nav.prev"/>
          </a>
         </xsl:if> 
	</td>
       <td align="center" class="center" width="306" height="30">
          <xsl:choose>
             <xsl:when test="count($up) > 0 and $up != $home">
               <xsl:apply-templates select="$up" mode="title.markup"/>
             </xsl:when>
             <xsl:otherwise>&#160;</xsl:otherwise>
          </xsl:choose>
        </td>
        <td width="145" height="30" class="right" align="right"> 
	   <xsl:if test="count($next)>0">
              <a accesskey="n">
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
  </xsl:if>
</xsl:template>

<!-- ==================================================================== -->

<xsl:template name="footer.navigation">
  <xsl:param name="prev" select="/foo"/>
  <xsl:param name="next" select="/foo"/>
  <xsl:variable name="home" select="/*[1]"/>
  <xsl:variable name="up" select="parent::*"/>

  <xsl:if test="$suppress.navigation = '0'">

    <!-- Bottom Navigation -->
    <div id="bottom-nav"
         style="position : relative;
                width : 596px;
                height : 66px;
		margin-top: 100px;
                background-image : url('{$kde.common}web-docbottom.png');
                background-color : transparent;
		margin-left: auto;
		margin-right: auto;
                z-index : 25;">
    <table width="596" border="0" cellpadding="0" cellspacing="0"
	style="position: absolute;
	       top: 70px;
	       left: 0px;">	
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

        <tr>
          <td width="33%" align="left">
            <xsl:apply-templates select="$prev" mode="title.markup"/>
            <xsl:text>&#160;</xsl:text>
          </td>
          <td width="34%" align="center">
            <xsl:choose>
              <xsl:when test="count($up)>0">
                <a>
                  <xsl:attribute name="href">
                    <xsl:call-template name="href.target">
                      <xsl:with-param name="object" select="$up"/>
                    </xsl:call-template>
                  </xsl:attribute>
                  <xsl:call-template name="gentext.nav.up"/>
                </a>
              </xsl:when>
              <xsl:otherwise>&#160;</xsl:otherwise>
            </xsl:choose>
          </td>
          <td width="33%" align="right">
            <xsl:text>&#160;</xsl:text>
            <xsl:apply-templates select="$next" mode="title.markup"/>
          </td>
        </tr>
      </table>
    </div>
  </xsl:if>
</xsl:template>

</xsl:stylesheet>
