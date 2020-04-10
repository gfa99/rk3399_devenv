#!/system/bin/sh

/system/bin/cp /system/bin/*.cgi /data/boa/www/cgi-bin/
/system/bin/cp /system/etc/boa/www/res/* /data/boa/www/res/
/system/bin/cp /system/etc/boa/www/css/* /data/boa/www/css/
/system/bin/chmod 0755 /data/boa/www/cgi-bin/*.cgi

