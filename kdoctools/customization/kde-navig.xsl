<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
            	version="1.0">

<xsl:template name="header.navigation">
  <xsl:param name="prev" select="/foo"/>
  <xsl:param name="next" select="/foo"/>
  <xsl:variable name="home" select="/*[1]"/>
  <xsl:variable name="up" select="parent::*"/>

  <xsl:if test="$suppress.navigation = '0'">
      <div id="headline"
           style="position : absolute;
                  height : 85px;
                  z-index : 100;
                  background : transparent;
                  text-align : center;
                  width : 100%;
                  top : 0;
                  left : 0;
                  width : 100%;
                  color : #000000;">
        <h1><xsl:apply-templates select="/" mode="title.ref"/></h1>
       </div>

<div id="navbackground"
  style="position : absolute;
  width : 100%;
  height : 124px;
  background-image : url('common/doctop2.png');
  z-index : 5;
  left : 0;
  top : 0;
  padding : 0;">

<div id="bulb1"
 style="padding : 0;
  position : absolute;
  z-index : 15;
  width : 150px;
  height : 85px;
  top : 0;
  left : 0;
  background : url('common/doctop1.png') repeat;"></div>

<div id="gradient"
  style="  position : absolute;
  width : 275px;
  height : 85px;
  z-index : 19;
  top : 0;
  padding : 0;
  left : 150px;
  background-image : url('common/doctop1a.png');
  background-repeat : no-repeat;
  background-color : transparent;
  visibility : visible;"></div>

<div id="bulb-bit"
style="position : absolute;
  width : 100%;
  height : 25px;
  top : 85px;
  left : 0;
  background-image : url('common/doctop1b.png');
  background-repeat : no-repeat;
  background-color : transparent;
  z-index : 5;">

  <table width="100%" id="navtable">
   <tbody>
     <tr>
       <td align="left" class="left">
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
 
       <td align="center" class="center">
          <xsl:choose>
             <xsl:when test="count($up) > 0 and $up != $home">
               <xsl:apply-templates select="$up" mode="title.ref"/>
             </xsl:when>
             <xsl:otherwise>&#160;</xsl:otherwise>
          </xsl:choose>
       </td>
       <td align="right" class="right">
	   <xsl:if test="count($prev)>0">
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
        </tbody>
     </table>
     </div>
     </div>
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
                width : 100%;
                height : 59px;
                left : 0px;
		right : 0px;
                top : 100px;
                background-image : url('common/bottom1.png');
                background-repeat : repeat-x;
                background-color : transparent;
		margin-left: 0px;
		margin-right: 0px;
                z-index : 25;">
    <img src="common/bottom2.png" 
	 align="right" 
         height="59" 
         width="227" 
         alt="KDE Logo"></img>
	 <div id="navtable2"
         style="width : 100%;
		margin-left: 0px;
	        margin-right: 0px;     
                z-index : 15;	
                background-color : transparent;">
    
    <table width="100%" border="0" cellpadding="0" cellspacing="0"
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
            <xsl:apply-templates select="$prev" mode="title.ref"/>
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
            <xsl:apply-templates select="$next" mode="title.ref"/>
          </td>
        </tr>

      </table>
    </div>
    </div>
  </xsl:if>
</xsl:template>

</xsl:stylesheet>
