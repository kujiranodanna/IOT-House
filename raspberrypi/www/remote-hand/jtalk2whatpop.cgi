#!/bin/bash
# The MIT License
# Copyright (c) 2025-2027 Isamu.Yamauchi ,2025.4.1 update 2025.4.1 

PATH=$PATH:/usr/local/bin
echo -n '
<HTML>
<HEAD>
<META http-equiv="Content-Type" content="text/html; charset=UTF-8">
<META NAME="auther" content="yamauchi.isamu">
<META NAME="copyright" content="pepolinux.jpn.org">
<META NAME="build" content="2025.4.1 ">
<META http-equiv="Refresh" content="2;URL=/remote-hand/wait_for.cgi">
<META NAME="reply-to" content="izamu@pepolinux.jpn.org">
<TITLE>Voice request process</TITLE>
<script type="text/javascript">
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
}
</script>
</HEAD>
<BODY onload="blink()" BGCOLOR="#E0FFFF">
<HR>
<TABLE ALIGN=CENTER BORDER=0 CELLPADDING=6 CELLSPACING=2>
<TR ALIGN=CENTER class="blink"><TD>Processing Upload Image File</TD></TR></TABLE>
<HR>
<TABLE ALIGN=RIGHT><TR><TD>&copy;2024-2026 pepolinux.jpn.org</TD><TR></TABLE>
</BODY>'

DIR=/www/remote-hand/tmp
CONV=./conv_get.cgi
. $CONV
if [ ! -n $voice ];then
  echo -n '</HTML>'
  exit
fi
prog=jtalk2whatpop
CMD=$DIR/${prog}.pepocmd
WHAT_POP=$DIR/what_pop.wav
LOCK=$DIR/${prog}.lock
OPENJTALK=/usr/bin/open_jtalk
VOICE_PITCH=0.7
VOICE=$(echo -n $voice |mawk '{gsub("@","",$0);print}')
if [ ! -e $LOCK ];then
  echo -en $$ >$LOCK
  cat>$CMD<<END
#!/bin/sh
echo -n $VOICE |$OPENJTALK -r $VOICE_PITCH -x /var/lib/mecab/dic/open-jtalk/naist-jdic -m /usr/share/hts-voice/mei/mei_normal.htsvoice -ow $WHAT_POP
rm $LOCK
END
    fi
  fi
fi
echo -n '
</HTML>'
