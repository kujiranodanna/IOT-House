#!/bin/bash
# The MIT License
# Copyright (c) 2020-2027 Isamu.Yamauchi , update 2025.4.5
PATH=$PATH:/usr/local/bin
# get ppp_user name & ppp mode
DIR=/www/remote-hand/tmp
CMD=$DIR/wait_for.pepocmd
PPP_USR=$DIR/.ppp_user
PPP_SEC=/etc/ppp/chap-secrets
PPP_MODE=$DIR/.ppp_mode
PPP_DIAL_MODE=/etc/ppp/ppp-dialer-mode
LOCKFILE="$DIR/LCK..wait_for.cgi"
LOCKPID="$DIR/LCK..wait_for.cgi.pid"
LOCKCGI="$DIR/LCK..pi_int.cgi"
LOCKCGIPID="$DIR/LCK..pi_int.cgi.pid"
RASPI=$(cat /proc/cpuinfo| grep " Raspberry Pi"| wc -l)
if [ $RASPI = 1 ];then
  HOMEPAGE="pi_int.html"
  HOMEPAGE_CGI="pi_int_gpio.cgi"
  HOMEPAGE_CGI_PI="pi_int.cgi"
else
  HOMEPAGE="pi_int_cp2112.html"
  HOMEPAGE_CGI="pi_int_cp2112.cgi"
fi
RMHOMEPAGE="NO"
NOWTIME=`date +%s`
JITTER=5
ALIAS_DI=$DIR/.alias_di
if [ -e $ALIAS_DI ];then
  timeSTAMP=`date +%s -r $ALIAS_DI`
  [ $(($NOWTIME - $timeSTAMP)) -lt $JITTER ] && RMHOMEPAGE="YES"
fi
ALIAS_DO=$DIR/.alias_do
if [ -e $ALIAS_DO ];then
  timeSTAMP=`date +%s -r $ALIAS_DO`
  [ $(($NOWTIME - $timeSTAMP)) -lt $JITTER ] && RMHOMEPAGE="YES"
fi
DICHANG1=$DIR/.di_change1
if [ -e $DICHANG1 ];then
  timeSTAMP=`date +%s -r $DICHANG1`
  [ $(($NOWTIME - $timeSTAMP)) -lt $JITTER ] && RMHOMEPAGE="YES"
fi
DICHANG2=$DIR/.di_change2
if [ -e $DICHANG2 ];then
  timeSTAMP=`date +%s -r $DICHANG2`
  [ $(($NOWTIME - $timeSTAMP)) -lt $JITTER ] && RMHOMEPAGE="YES"
fi
AUTOACT_LIST=$DIR/.auto_act.list
if [ -e $AUTOACT_LIST ];then
  timeSTAMP=`date +%s -r $AUTOACT_LIST`
  [ $(($NOWTIME - $timeSTAMP)) -lt $JITTER ] && RMHOMEPAGE="YES"
fi
STARTUP=$DIR/.startup.s.tmp
if [ -e $STARTUP ];then
  timeSTAMP=`date +%s -r $STARTUP`
  [ $(($NOWTIME - $timeSTAMP)) -lt $JITTER ] && RMHOMEPAGE="YES"
fi
SOUND=$DIR/.sound_file_name
if [ -e $SOUND ];then
  timeSTAMP=`date +%s -r $SOUND`
  [ $(($NOWTIME - $timeSTAMP)) -lt $JITTER ] && RMHOMEPAGE="YES"
fi
GMAIL=$DIR/.pepogmail4dio.conf
if [ -e $GMAIL ];then
  timeSTAMP=`date +%s -r $GMAIL`
  [ $(($NOWTIME - $timeSTAMP)) -lt $JITTER ] && RMHOMEPAGE="YES"
fi
[ -e $ALIAS_DI ] && . $ALIAS_DI
[ -z "$DI_TTY" ] && DI_TTY="gpio"
if [ $DI_TTY != "gpio" ];then
   PI_INT=$HOMEPAGE_CGI_PI
   cat>$CMD<<END
#!/bin/sh
rm -f /www/remote-hand/pepopiface
ln -s /usr/local/bin/pepopiface_local /www/remote-hand/pepopiface
END
else
  PI_INT=$HOMEPAGE_CGI
   cat>$CMD<<END
#!/bin/sh
rm -f /www/remote-hand/pepopiface
ln -s /usr/local/bin/pepogpiohelp /www/remote-hand/pepopiface
END
fi
echo -n '
<HTML>
<HEAD>
<META http-equiv="Content-Type" content="text/HTML; charset=utf-8">
<META NAME="Auther" content="yamauchi.isamu">
<META NAME="Copyright" content="pepolinux.jpn.org">
<META NAME="Build" content="2025.4.5">
<META NAME="reply-to" content="izamu@pepolinux.jpn.org">
<TITLE>Remote-hand wait for process</TITLE>
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
}'
# wait proceed
BUSY=`ls $DIR/|grep -E ".pepocmd$"|wc -w`
while [ "$BUSY" != 0 ]
do
  msleep 500
  BUSY=`ls $DIR/|grep -E ".pepocmd$"|wc -w`
done
while [ -e ${LOCKFILE} ]
do
  msleep 5000
  rm -rf ${LOCKFILE}
done
lockfile -3 -r 5 ${LOCKFILE} >/dev/null 2>&1
if [ $? != 0 ];then
echo -n '
var jump_url = setTimeout("jump_href()", 10000);
function jump_href() {
  var jump_location = "/index.html?" + (new Date().getTime());
  location.href=jump_location;
  clearTimeout(jump_url);
}
</script>
</HEAD>
<BODY onload="blink()" BGCOLOR="#E0FFFF">
<HR>
<TABLE ALIGN=CENTER BGCOLOR="#E0FFFF" BORDER=0 CELLPADDING=6 CELLSPACING=2>
<TR ALIGN=CENTER class="blink"><TD>Server busy</TD></TR>
<TR ALIGN=CENTER><TD>Please wait</TD></TR>
</TABLE>
<HR>
<TABLE ALIGN=RIGHT><TR><TD>&copy;2025 pepolinux.jpn.org</TD><TR></TABLE>
</BODY>
</HTML>'
exit -1
else
while [ -e ${LOCKCGI} ]
do
  echo -n '
var jump_url = setTimeout("jump_href()", 10000);
function jump_href() {
  var jump_location = "/index.html?" + (new Date().getTime());
  location.href=jump_location;
  clearTimeout(jump_url);
}
</script>
</HEAD>
<BODY onload="blink()" BGCOLOR="#E0FFFF">
<HR>
<TABLE ALIGN=CENTER BGCOLOR="#E0FFFF" BORDER=0 CELLPADDING=6 CELLSPACING=2>
<TR ALIGN=CENTER class="blink"><TD>Server busy</TD></TR>
<TR ALIGN=CENTER><TD>Please wait</TD></TR>
</TABLE>
<HR>
<TABLE ALIGN=RIGHT><TR><TD>&copy;2025 pepolinux.jpn.org</TD><TR></TABLE>
</BODY>
</HTML>'
  msleep 20000
  rm -rf ${LOCKCGI}
  rm -rf ${LOCKCGIPID}
  exit -1
done
  echo -n $$ >${LOCKPID}
fi
if [ -e $PPP_SEC ]; then
cat>$CMD<<EOF
#!/bin/sh
ppp_user=\`cat $PPP_SEC |grep -F -v "#"|mawk 'BEGIN{FS="\""};{print \$2}'\`
echo \$ppp_user > $PPP_USR
chown www-data:www-data $PPP_USR
ppp_mode=\`cat $PPP_DIAL_MODE |grep -E "64k=" |mawk '{
split(\$1,I,"=")
if (I[2] == "yes")
  printf("yes 64kDigital communications")
else if (I[2]  == "no")
  printf("no Packet communications")
else if (I[2] == "NONE")
  printf("NONE none")
}'\`

echo \$ppp_mode >$PPP_MODE
chown www-data:www-data $PPP_MODE
EOF
fi

# get web user name
WEBPASS=/etc/rc.pepo/password
TMPWEB=$DIR/.htpasswd
if [ -e $WEBPASS ]; then
cat>>$CMD<<EOF
WEB_USER=\`cat $WEBPASS |grep -F -v "#"|mawk 'BEGIN{FS=":"};{print \$1}'\`
echo "\$WEB_USER" > $TMPWEB
chown www-data:www-data $TMPWEB
EOF
fi

cat>>$CMD<<EOF
# get server setting data
STARTUP=/usr/src/pepolinux/startup.s
tSTARTUP=$DIR/.startup.s.tmp
rm -f \$tSTARTUP
if [ -e \$STARTUP ];then
  cat \$STARTUP |mawk '/^SET_/{print \$0}' >\$tSTARTUP
  . \$tSTARTUP
  echo "vWEBUSER=\$SET_WEBUSER" >> \$tSTARTUP
  echo "vWEBPASSWORD=\$SET_WEBPASSWORD" >> \$tSTARTUP
  echo "vLINENOTIFY=\$SET_LINENOTIFY" >> \$tSTARTUP
  chown www-data:www-data \$tSTARTUP
fi
EOF

# wait proceed
BUSY=`ls /$DIR|grep -E ".pepocmd$"|wc -w`
while [ "$BUSY" != 0 ]
do
  msleep 500
  BUSY=`ls $DIR/|grep -E ".pepocmd$"|wc -w`
done
[ $RMHOMEPAGE = "YES" ] && rm $HOMEPAGE

while [ -e ${LOCKCGI} ]
do
  msleep 20000
  rm -rf ${LOCKCGI}
  rm -rf ${LOCKCGIPID}
done
if [ -e "./pi_int.html" ];then
  echo -n '
var jump_url = setTimeout("jump_href()", 500);
function jump_href() {
  var jump_location = "./pi_int.html?" + (new Date().getTime());
  location.href=jump_location;
  clearTimeout(jump_url);
}
</script>
</HEAD>
<BODY onload="blink()" BGCOLOR="#E0FFFF">
<HR>
<TABLE ALIGN=CENTER BORDER=0 CELLPADDING=6 CELLSPACING=2>
<TR ALIGN=CENTER class="blink"><TD>Now processing</TD></TR>
<TR ALIGN=CENTER><TD>Please wait</TD></TR>
</TABLE>
<HR>
<TABLE ALIGN=RIGHT><TR><TD>&copy;2025 pepolinux.jpn.org</TD></TR></TABLE>
</BODY>
</HTML>'
  msleep 1000
  ./${PI_INT} >/dev/null 2>&1
else
  if [ $DI_TTY = "gpio" ];then
    echo -n '
var jump_url = setTimeout("jump_href()", 1500);
function jump_href() {
  var jump_location = "/remote-hand/pi_int_gpio.cgi?" + (new Date().getTime());
'
  else
    echo -n '
var jump_url = setTimeout("jump_href()", 1500);
function jump_href() {
  var jump_location = "/remote-hand/pi_int.cgi?" + (new Date().getTime());
'
  fi
  echo -n '
  location.href=jump_location;
  clearTimeout(jump_url);
}
</script>
</HEAD>
<BODY onload="blink()" BGCOLOR="#E0FFFF">
<HR>
<TABLE ALIGN=CENTER BORDER=0 CELLPADDING=6 CELLSPACING=2>
<TR ALIGN=CENTER class="blink"><TD>Now processing</TD></TR>
<TR ALIGN=CENTER><TD>Please wait</TD></TR>
</TABLE>
<HR>
<TABLE ALIGN=RIGHT><TR><TD>&copy;2025 pepolinux.jpn.org</TD></TR></TABLE>
</BODY>
</HTML>'
fi
if [ -e ${LOCKFILE} ];then
  if [ $$ = $(cat ${LOCKPID}) ];then
    rm ${LOCKFILE}
    rm ${LOCKPID}
  fi
fi
