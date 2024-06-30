USER_NAME=simple
USER_PASSWORD=HTTPS1mple
BIN_DIR=HTTPSimple
WORK_DIR=/var/www/simple
SERVICE_NAME=simple.service

setup() {
  HTTPSimple
}

clean() {
  rm -rf $BIN_DIR
}

HTTPSimple() {
  gcc -c -o main.o main.c
  gcc -c -o auth.o auth.c
  gcc -o $BIN_DIR main.o auth.o -lssl -lcrypto -lpam -lpam_misc
}

pam() {
  cat >> /etc/pam.d/check_user << EOF
auth      required  pam_unix.so shadow
account   required  pam_unix.so
EOF
}

install() {
  HTTPSimple

  useradd -c "$BIN_DIR user" -r -d $WORK_DIR $USER_NAME
  echo "$USER_NAME:$USER_PASSWORD" | chpasswd

  command install -o root -g root -m 0755 $BIN_DIR /usr/local/sbin/
  command install -o root -g root -m 0644 $SERVICE_NAME /etc/systemd/system/

  mkdir -p $WORK_DIR
  cp -r webroot -t $WORK_DIR/

  bash key-gen.sh $USER_NAME $USER_PASSWORD
  cp -r keys -t $WORK_DIR/

  chown -R $USER_NAME:$USER_NAME $WORK_DIR

  rm -f ./$BIN_DIR
  rm -rf keys
  rm -f main.o auth.o

  pam
  systemctl daemon-reload
  systemctl restart $SERVICE_NAME
}

uninstall() {
  systemctl stop $SERVICE_NAME
  rm -rf $WORK_DIR
  rm -f /usr/local/sbin/$BIN_DIR
  rm -f /etc/systemd/system/$SERVICE_NAME
  rm /etc/pam.d/check_user
  systemctl daemon-reload
  userdel -f $USER_NAME
}

if [ $# -gt 0 ]; then
  f_call=$1; shift; $f_call "$@"
else
  setup
fi