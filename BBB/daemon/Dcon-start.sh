#!/bin/bash
#
#       /etc/rc.d/init.d/ns-slapd
# ns-slapd      This shell script takes care of starting and stopping
#               ns-slapd (the Fedora Directory Server)
#
# Author: Brian Jones 
 This e-mail address is being protected from spambots. You need JavaScript enabled to view it
 
#
# chkconfig: 2345 13 87
# description: ns-slapd is the Fedora Directory Service daemon. \
# FDS can serve as a repository for (and, subsequently, a source of) \
# data for most of the resources listed in /etc/nsswitch.conf, such as \
# passwd or group entries.

# Source function library.
. /etc/init.d/functions

SLAPD_HOST=`hostname -a`
SLAPD_DIR=/opt/fedora-ds/bin/slapd/server
PIDFILE=$SLAPD_DIR/logs/pid
STARTPIDFILE=$SLAPD_DIR/logs/startpid

if [ -f /etc/sysconfig/ns-slapd ]; then
        . /etc/sysconfig/ns-slapd
fi


start() {
        echo -n "Starting Fedora Directory Server: "
        if [ -f $STARTPIDFILE ]; then
                PID=`cat $STARTPIDFILE`
                echo ns-slapd already running: $PID
                exit 2;
        elif [ -f $PIDFILE ]; then
                PID=`cat $PIDFILE`
                echo ns-slapd already running: $PID
                exit 2;
        else
                cd $SLAPD_DIR
                daemon  ./ns-slapd $OPTIONS
                RETVAL=$?
                echo
                [ $RETVAL -eq 0 ] && touch /var/lock/subsys/ns-slapd
                return $RETVAL
        fi

}

stop() {
        echo -n "Shutting down Fedora Directory Server: "
        echo
        killproc ns-slapd
        echo
        rm -f /var/lock/subsys/ns-slapd
        return 0
}

case "$1" in
    start)
        start
        ;;
    stop)
        stop
        ;;
    status)
        status ns-slapd
        ;;
    restart)
        stop
        start
        ;;
    *)
        echo "Usage:  {start|stop|status|restart}"
        exit 1
        ;;
esac
exit $?
