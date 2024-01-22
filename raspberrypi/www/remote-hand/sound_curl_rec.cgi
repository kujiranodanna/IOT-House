#!/bin/bash
# The MIT License
# Copyright (c) 2020-2027 Isamu.Yamauchi ,2023.11.10 update  2024.1.14
PATH=$PATH:/usr/local/bin
echo -en '
<HTML>
<HEAD>
<META http-equiv="Content-Type" content="text/html; charset=UTF-8">
<META NAME="auther" content="yamauchi.isamu">
<META NAME="copyright" content="pepolinux.jpn.org">
<META NAME="build" content="2023.11.10>
<META http-equiv="Refresh" content="2;URL=/remote-hand/wait_for.cgi">
<META NAME="reply-to" content="izamu@pepolinux.jpn.org">
<TITLE>Upload Sound File for curl</TITLE>
<script type="text/javascript">
function blink() {
  // if (!document.all) { return; }
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
<TR ALIGN=CENTER class="blink"><TD>Processing Upload Sound File</TD></TR></TABLE>
<HR>
<TABLE ALIGN=RIGHT><TR><TD>&copy;2023-2026 pepolinux.jpn.org</TD><TR></TABLE>
</BODY>
<HTML>'

DIR=/www/remote-hand/tmp
prog=sound_curl_rec
SOUND_FILE=$DIR/${prog}.$$
tSOUND_FILE=$DIR/${prog}_tmp.$$
tFILE=$DIR/${prog}_tmp_tmp.$$
LOCK=$DIR/${prog}_lock
MAXFILESIZE=$((2048 * 1024))
error(){
  [ -e ${tFILE_NAME} ] && rm ${tFILE_NAME}
  [ -e ${SOUND_FILE} ] && rm ${SOUND_FILE}
  [ -e ${tSOUND_FILE} ] && rm ${tSOUND_FILE}
  [ -e ${PLAYFILE} ] && rm ${PLAYFILE}
  exit 0
}
trap error TERM HUP KILL INT QUIT
CMD=$DIR/sound_curl_file_$$.pepocmd
cat >$tSOUND_FILE
if [ -e $tSOUND_FILE ];then
  dd if=$tSOUND_FILE bs=1 count=128 |mawk '/^[a-z]/{gsub("\r","",$0);print}' >$tFILE
  . $tFILE  
  if [ $filesize -gt $MAXFILESIZE ];then
    rm $tSOUND_FILE
    exit
  fi
  cat  $tSOUND_FILE |sed -n 6,6p |mawk '{gsub("\r","",$0);gsub(";","",$0);printf("%s\n%s\n",$3,$4)}' >$tFILE
  . $tFILE
else
  exit
fi
PLAYFILE=$DIR/$filename
MP3_YES_NO=$(echo $filename |mawk 'BEGIN{TMP="NO"};/mp3$/{TMP="YES"};END{printf TMP}')
WAV_YES_NO=$(echo $filename |mawk 'BEGIN{TMP="NO"};/wav$/{TMP="YES"};END{printf TMP}')
cat >$CMD<<END
#!/bin/bash
if [ ! -e $LOCK ];then
  touch $LOCK
  cat $tSOUND_FILE | sed '1,8d' >$SOUND_FILE
  dd if=$SOUND_FILE of=$PLAYFILE bs=$filesize count=1
  if [ $MP3_YES_NO = "YES" ];then
    mpg321 $PLAYFILE
  elif [ $WAV_YES_NO = "YES" ];then
    aplay $PLAYFILE
  fi
  [ -e $tSOUND_FILE ] && rm $tSOUND_FILE
  [ -e $tFILE ] && rm $tFILE
  [ -e $SOUND_FILE ] && rm $SOUND_FILE
  [ -e $PLAYFILE ] && rm $PLAYFILE
  [ -e $LOCK ] && rm $LOCK
else
  [ -e $tSOUND_FILE ] && rm $tSOUND_FILE
  [ -e $tFILE ] && rm $tFILE
  [ -e $SOUND_FILE ] && rm $SOUND_FILE
  [ -e $PLAYFILE ] && rm $PLAYFILE
fi
END