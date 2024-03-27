USER_NAME=foxweb
BIN_DIR=PICOFoxweb
WORK_DIR=/var/www/foxweb
SERVICE_NAME=foxweb.service

setup() {
    PICOFoxweb
}

clean() {
    rm -rf $BIN_DIR
}

PICOFoxweb() {
    gcc -c -o main.o main.c
    gcc -c -o httpd.o httpd.c
    gcc -c -o logger.o logger.c
    gcc -o $BIN_DIR main.o httpd.o logger.o
}

install() {
    PICOFoxweb
    useradd -c "$BIN_DIR user" -r -s /sbin/nologin -d $WORK_DIR $USER_NAME
    command install -o root -g root -m 0755 $BIN_DIR /usr/local/sbin/
    command install -o root -g root -m 0644 $SERVICE_NAME /etc/systemd/system/
    systemctl daemon-reload
    systemctl restart $SERVICE_NAME
    mkdir -p $WORK_DIR
    cp -r webroot -t $WORK_DIR/
    chown -R $USER_NAME:$USER_NAME $WORK_DIR
    rm -f ./$BIN_DIR
}

uninstall() {
	systemctl stop $SERVICE_NAME
	rm -rf $WORK_DIR
	rm -f /usr/local/sbin/$BIN_DIR
	rm -f /etc/systemd/system/$SERVICE_NAME
	systemctl daemon-reload
	userdel -f $USER_NAME
}

if [ $# -gt 0 ]; then
    f_call=$1; shift; $f_call "$@"
else
    setup
fi