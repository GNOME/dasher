xsltproc jp.xslt jp.xml > jp.rc_
xsltproc en.xslt en.xml > en.rc_
copy /Y top.rc_/B+jp.rc_/B+en.rc_/B resource.rc
