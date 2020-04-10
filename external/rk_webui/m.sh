CC=/home/xy/RK3308_KUGOU/prebuilts/gcc/linux-x86/arm/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc

${CC} wifi/lollipop_wifiScanAp.c utility/string_ext.c utility/utility.c wifi/list_network.c wifi/operate.c www/html_string.c -fPIC -shared -o liboper.so

${CC} www/home.c -o home.cgi

${CC} www/wifi.c liboper.so -o wifi.cgi

${CC} www/scan.c liboper.so -o scan.cgi

${CC} www/connect.c liboper.so -o connect.cgi

${CC} www/add_network.c liboper.so -o add_network.cgi

${CC} www/add_network_connect.c liboper.so -o add_network_connect.cgi

${CC} www/disconnect.c liboper.so -o disconnect.cgi

${CC} www/forget.c liboper.so -o forget.cgi

${CC} www/dialog.c liboper.so -o dialog.cgi

${CC} www/connectSavedAp.c liboper.so -o connectSavedAp.cgi

${CC} www/key.c liboper.so -o key.cgi

${CC} www/settings.c liboper.so -o settings.cgi

${CC} www/overscan.c liboper.so -o overscan.cgi

${CC} www/language.c liboper.so -o language.cgi

${CC} www/softap_freq.c liboper.so -o softap_freq.cgi

${CC} www/password.c liboper.so -o password.cgi

${CC} www/inputUrl.c liboper.so -o inputUrl.cgi

${CC} www/debug.c liboper.so -o debug.cgi
