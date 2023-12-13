#!/bin/bash
# The MIT License
# Copyright (c) 2020-2027 Isamu.Yamauchi , update 2018.12.1

echo -en '
<HTML>
<HEAD>
<META http-equiv="Content-Type" content="text/HTML; charset=UTF-8">
<META NAME="Auther" content="yamauchi.isamu">
<META NAME="Copyright" content="pepolinux.jpn.org">
<META NAME="Build" content="2018.12.1">
<META NAME="reply-to" content="izamu@pepolinux.jpn.org">
<META http-equiv="Refresh" content="10;URL=/remote-hand/tmp/gpio_temp.png">
<TITLE>GPIO Tempature Graph Create</TITLE>
<script type="text/javascript">
function blink() {
//  if (!document.all) { return; }
  for (i = 0; i < document.all.length; i++) {
    obj = document.all(i);
    if (obj.className == "blink") {
      if (obj.style.visibility == "visible") {
        obj.style.visibility = "hidden";
      } else {
        obj.style.visibility = "visible";
      }
    }
  }
  setTimeout("blink()",1000);
}
</script>
</HEAD>
<BODY onload="blink()" BGCOLOR="#E0FFFF">
<HR>
<TABLE ALIGN=CENTER BORDER=0 CELLPADDING=6 CELLSPACING=2>
<TR ALIGN=CENTER class="blink"><TD>GPIO Tempature graph create</TD></TR>
</TABLE>
<HR>
<TABLE ALIGN=RIGHT><TR><TD>&copy;2021-2025 pepolinux.jpn.org</TD><TR></TABLE>
</BODY>
</HTML>'
WORKDIR=/www/remote-hand
DSFILE=${WORKDIR}/tmp/.gpio_temp_hum.rrd
RRDSTART=${WORKDIR}/tmp/.gpio_temp_hum_start_time
GRAP_TEMPFILE=${WORKDIR}/tmp/gpio_temp.png
END=`date "+%Y%m%d %H:%M:00" -d "-10 minute"`
END=`date -d "${END}" +%s`
CMD=${WORKDIR}/tmp/gpio_temp.pepocmd
if [ -e ${DSFILE} ];then
  [ -e ${RRDSTART} ] && . ${RRDSTART}
  TEMPSTART=`date "+%Y%m%d %H:%M:00" -d "-7 days"`
  TEMPSTART=`date -d "${TEMPSTART}" +%s`
  if [ ${TEMPSTART} -gt ${START} ];then
    START=${TEMPSTART}
  fi
  [ -e $GRAP_TEMPFILE ] && rm -f $GRAP_TEMPFILE
cat >${CMD}<<EOF
#!/bin/bash
rrdtool graph ${GRAP_TEMPFILE} --width 600 --height 120 --start ${START} --end ${END} --upper-limit 50 --lower-limit -10 DEF:a=${DSFILE}:gpio_temp:MAX LINE1:a#FF0000:"GPIO Tempature"
EOF
fi
