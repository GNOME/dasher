<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:key name="getelement" match="L10n_resource[@language='EN']//langdata" use="@key" /> 

<xsl:template match="/">"Hello" in this language is: <xsl:apply-templates select="key('getelement','hello')" />
</xsl:template>

</xsl:stylesheet>