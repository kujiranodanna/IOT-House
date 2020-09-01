#!/bin/bash
# licence GPLv3 ; this scripts designed by Isamu Yamauchi 2020.3.27
# utilities for Voice Request Delete
VOICE_REQ=/www/remote-hand/tmp/.voice_req.tmp
VOICE_REQ_CMD=/www/remote-hand/tmp/voice_req.pepocmd
CONV=./conv_get.cgi
. $CONV
cat >>$VOICE_REQ_CMD<END
#!/bin/bash
rm $VOICE_REQ
END
