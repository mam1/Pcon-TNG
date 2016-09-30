#!/bin/bash
###############################################################
#
#		Pcon_daemon.sh
#       /etc/rc.d/init.d/Pcon_daemon
#
#      This shell script takes care of starting and stopping
#      the Pcon_daemon 
#
###############################################################

# Source function library.
. /etc/init.d/functions

SLAPD_HOST=`hostname -a`
SLAPD_DIR=/opt/fedora-ds/bin/slapd/server
PIDFILE=$SLAPD_DIR/logs/pid
STARTPIDFILE=$SLAPD_DIR/logs/startpid



if [ -f /etc/sysconfig/Pcon_daemon ]; then
        . /etc/sysconfig/Pcon_daemon
fi


start() {
        echo -n "Starting Pcon Pcon_daemon: "
        if [ -f $STARTPIDFILE ]; then
                PID=`cat $STARTPIDFILE`
                echo Pcon_daemon already running: $PID
                exit 2;
        elif [ -f $PIDFILE ]; then
                PID=`cat $PIDFILE`
                echo Pcon_daemon already running: $PID
                exit 2;
        else
                cd $SLAPD_DIR
                Pcon_daemon  ./Pcon_daemon $OPTIONS
                RETVAL=$?
                echo
                [ $RETVAL -eq 0 ] && touch /var/lock/subsys/Pcon_daemon
                return $RETVAL
        fi

}

stop() {
        echo -n "Shutting down Fedora Directory Server: "
        echo
        killproc Pcon_daemon
        echo
        rm -f /var/lock/subsys/Pcon_daemon
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
        status Pcon_daemon
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
