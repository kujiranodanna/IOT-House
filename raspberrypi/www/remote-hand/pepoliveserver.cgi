#!/bin/bash
PATH=$PATH:/usr/local/bin:/usr/local/sbin
WORKDIR=/www/remote-hand/tmp
CMD=$WORKDIR/liveserver.pepocmd
LIVESERVER=/usr/local/bin/pepoliveserver
LIVEMP4CTL=/usr/local/bin/pepomp4ctl
LIVEIMG=remote-hand.jpg
echo -en '
<HTML>
<HEAD>
<META http-equiv="Content-Type" content="text/HTML; charset=utf-8">
<META NAME="Auther" content="yamauchi.isamu">
<META NAME="Copyright" content="pepolinux.com">
<META NAME="Build" content="2018.5.8">
<META NAME="reply-to" content="izamu@pepolinux.com">
<TITLE>Live Webcam</TITLE>
<script type="text/javascript">
<!--
function blink() {
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
}</script>
</HEAD>
<BODY onload="blink()" BGCOLOR="#E0FFFF">
<TABLE ALIGN=CENTER BORDER=0 CELLPADDING=6 CELLSPACING=2>
<TR ALIGN=CENTER class="blink"><TD>動画配信中です</TD></TR>
</TABLE>
</BODY>
</HTML>
'
CONV=./conv_get.cgi
. $CONV
DEV=/dev/${dev}
[ ! -e $DEV ] && exit
TIMER=$live_timer
if [ ${TIMER} = 0 ];then
cat >$CMD<<END
#!/bin/bash
$LIVEMP4CTL $DEV $LIVEIMG `echo \$\$`
END
else
cat >$CMD<<END
#!/bin/bash
$LIVESERVER $DEV $TIMER
END
fi
