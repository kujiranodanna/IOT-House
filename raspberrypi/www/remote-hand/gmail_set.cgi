#!/bin/bash
# The MIT License
# Copyright (c) 2020-2027 Isamu.Yamauchi , update 2025.4.17

# gamil_set.cgi
echo -n '
<HTML>
<HEAD>
<META http-equiv="Content-Type" content="text/HTML; charset=UTF-8">
<META NAME="Auther" content="yamauchi.isamu">
<META NAME="Copyright" content="pepolinux.jpn.org">
<META http-equiv="Refresh" content="0;URL=/remote-hand/wait_for.cgi">
<META NAME="Build" content="2025.3.6">
<META NAME="reply-to" content="izamu@pepolinux.jpn.org">
<TITLE>Setting in DIO operation in Gmail</TITLE>
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
<TABLE ALIGN=CENTER BGCOLOR="#E0FFFF" BORDER=0 CELLPADDING=6 CELLSPACING=2>
<TR ALIGN=CENTER class="blink"><TD>Setting in DIO operation in Gmail</TD></TR>
</TABLE>
<HR>
<TABLE ALIGN=RIGHT><TR><TD>&copy;2021-2025 pepolinux.jpn.org</TD></TR></TABLE>
</BODY>
</HTML>'
CONV=./conv_get.cgi
. $CONV
GMAILUSER="$gmailuser"
GMAILPASSWORD="$gmailpassword"
PERMITMAIL="$permitmail"
KEYWORD="$keyword"
JITTER="$jitter"
REG="$reg"
LOOPTIME="$looptime"
DIR=/www/remote-hand/tmp
CONF=$DIR/.pepogmail4dio.conf
CMD=$DIR/pepogmail4dio.pepocmd
MAIL_CONF_ORG=/etc/exim4/passwd.client.org
MAIL_CONF=/etc/exim4/passwd.client
MAIL_CONF_CONF=/etc/exim4/update-exim4.conf.conf
MAIL_CONF_CONF_ORG=/etc/exim4/update-exim4.conf.conf.org
tMAIL_CONF=$DIR/.gmail.cgi.tmp
ROOT_MURC=/root/.mutt/muttrc
tROOT_MURC=$DIR/muttrc
if [ $REG = "del" ];then
  GMAILUSER="root"
  cat>$CMD<<END
#!/bin/sh
cat>$tROOT_MURC<<EOF
set mbox_type = Mbox
set copy = no
set charset = "utf-8"
set send_charset = "iso-2022-jp:utf-8"
set realname = "$GMAILUSER"
set from = "${GMAILUSER}@localhost"
set use_from = "yes"
set envelope_from = "yes"
EOF
mv $tROOT_MURC $ROOT_MURC
cat >$tMAIL_CONF<<EOF
# password file used when the local exim is authenticating to a remote
# host as a client.
#
# see exim4_passwd_client(5) for more documentation
#
# Example:
### target.mail.server.example:login:password
EOF
mv $tMAIL_CONF $MAIL_CONF
cat >$tMAIL_CONF<<EOF
# /etc/exim4/update-exim4.conf.conf
#
# Edit this file and /etc/mailname by hand and execute update-exim4.conf
# yourself or use 'dpkg-reconfigure exim4-config'
#
# Please note that this is _not_ a dpkg-conffile and that automatic changes
# to this file might happen. The code handling this will honor your local
# changes, so this is usually fine, but will break local schemes that mess
# around with multiple versions of the file.
#
# update-exim4.conf uses this file to determine variable values to generate
# exim configuration macros for the configuration file.
#
# Most settings found in here do have corresponding questions in the
# Debconf configuration, but not all of them.
#
# This is a Debian specific file

dc_eximconfig_configtype='local'
dc_other_hostnames=$(hostname)
dc_local_interfaces='127.0.0.1'
dc_readhost=''
dc_relay_domains=''
dc_minimaldns='false'
dc_relay_nets=''
dc_smarthost=''
CFILEMODE='644'
dc_use_split_config='false'
dc_hide_mailname=''
dc_mailname_in_oh='true'
dc_localdelivery='mail_spool'
EOF
mv $tMAIL_CONF $MAIL_CONF_CONF
END
msleep 100
update-exim4.conf
END
  exit 0
fi
cat >$CONF<<END
GMAILUSER="$gmailuser"
GMAILPASSWORD="$gmailpassword"
PERMITMAIL="$permitmail"
KEYWORD="$keyword"
JITTER="$jitter"
LOOPTIME="$looptime"
END
cat>$CMD<<END
#!/bin/sh
if [ ! -e ${MAIL_CONF_ORG} ];then
cat >${MAIL_CONF_ORG}<<EOF
# password file used when the local exim is authenticating to a remote
# host as a client.
#
# see exim4_passwd_client(5) for more documentation
#
# Example:
### target.mail.server.example:login:password
gmail-smtp.l.google.com:YOUR-USER-NAME@gmail.com:YOUR-USER-PASSWORD
*.google.com:YOUR-USER-NAME@gmail.com:YOUR-USER-PASSWORD
smtp.gmail.com:YOUR-USER-NAME@gmail.com:YOUR-USER-PASSWORD
EOF
fi
cat $MAIL_CONF_ORG | awk '
/^gmail-smtp.l.google.com:/{print "gmail-smtp.l.google.com:${GMAILUSER}@gmail.com:${GMAILPASSWORD}";next}
/^*.google.com:/{print "*.google.com:${GMAILUSER}@gmail.com:${GMAILPASSWORD}";next}
/^smtp.gmail.com:/{print "smtp.gmail.com:${GMAILUSER}@gmail.com:${GMAILPASSWORD}";next}
  {print \$0}' >$tMAIL_CONF
[ -e $tMAIL_CONF ] && mv $tMAIL_CONF $MAIL_CONF
cat>$MAIL_CONF_CONF<<EOF
# 2018.2.11 pepo
dc_eximconfig_configtype='smarthost'
dc_other_hostnames=$(hostname)
dc_local_interfaces='127.0.0.1'
dc_readhost=''
dc_relay_domains=''
dc_minimaldns='false'
dc_relay_nets=''
dc_smarthost='smtp.gmail.com::587'
CFILEMODE='644'
dc_use_split_config='false'
dc_hide_mailname='false'
dc_mailname_in_oh='true'
dc_localdelivery='mail_spool'
EOF
cat>$tROOT_MURC<<EOF
set mbox_type = Mbox
set copy = no
set charset = "utf-8"
set send_charset = "iso-2022-jp:utf-8"
set realname = "$GMAILUSER"
set from = "${GMAILUSER}@gmail.com"
set use_from = "yes"
set envelope_from = "yes"
EOF
mv $tROOT_MURC $ROOT_MURC
update-exim4.conf
END
