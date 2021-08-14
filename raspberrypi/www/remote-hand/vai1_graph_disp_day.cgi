#!/bin/bash
# The MIT License
# Copyright (c) 2020-2027 Isamu.Yamauchi , update 2018.8.25

echo -en '
<HTML>
<HEAD>
<META http-equiv="Content-Type" content="text/HTML; charset=UTF-8">
<META NAME="Auther" content="yamauchi.isamu">
<META NAME="Copyright" content="pepolinux.com">
<META NAME="Build" content="2018.8.25">
<META NAME="reply-to" content="izamu@pepolinux.com">
<META http-equiv="Refresh" content="8;URL=/remote-hand/tmp/i2c_vai1.png">
<TITLE>I2C Analog Input Graph Create</TITLE>
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
<TR ALIGN=CENTER class="blink"><TD>I2C Analog input-1 graph create</TD></TR>
</TABLE>
<HR>
<TABLE ALIGN=RIGHT><TR><TD>&copy;2021-2025 pepolinux.com</TD><TR></TABLE>
</BODY>
</HTML>'
WORKDIR=/www/remote-hand
DSFILE=${WORKDIR}/tmp/.i2c_temp_hum.rrd
RRDSTART=${WORKDIR}/tmp/.i2c_start_time
GRAP_TEMPFILE=${WORKDIR}/tmp/i2c_vai1.png
END=`date "+%Y%m%d %H:%M:00" -d "-10 minute"`
END=`date -d "${END}" +%s`
CMD=${WORKDIR}/tmp/i2c_vai1.pepocmd
if [ -e ${DSFILE} ];then
  [ -e ${RRDSTART} ] && . ${RRDSTART}
  TEMPSTART=`date "+%Y%m%d %H:00:00" -d "-1 days"`
  TEMPSTART=`date -d "${TEMPSTART}" +%s`
  if [ ${TEMPSTART} -gt ${START} ];then
    START=${TEMPSTART}
  fi
  [ -e $GRAP_TEMPFILE ] && rm -f $GRAP_TEMPFILE
cat >${CMD}<<EOF
#!/bin/bash
rrdtool graph ${GRAP_TEMPFILE} --width 600 --height 120 --start ${START} --end ${END} --upper-limit 2000 --lower-limit 0 DEF:a=${DSFILE}:vai1:MAX LINE1:a#FF0000:"I2C Analog input-1"
EOF
fi
