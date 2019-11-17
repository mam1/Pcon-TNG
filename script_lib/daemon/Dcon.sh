#! /bin/sh

### BEGIN INIT INFO
# Provides:		Dcon
# Required-Start:	$remote_fs $syslog
# Required-Stop:	$remote_fs $syslog
# Default-Start:	2 3 4 5
# Default-Stop:		0 1 6
# Short-Description:	Dcon MQTT v3.1 message broker
# Description: 
#  This is a message broker that supports version 3.1 of the MQ Telemetry
#  Transport (MQTT) protocol.
#  
#  MQTT provides a method of carrying out messaging using a publish/subscribe
#  model. It is lightweight, both in terms of bandwidth usage and ease of
#  implementation. This makes it particularly useful at the edge of the network
#  where a sensor or other simple device may be implemented using an arduino for
#  example.
### END INIT INFO

#stops the execution of a script if a command or pipeline has an error 
set -e  

PIDFILE=/var/run/Dcon.pid
DAEMON=/usr/sbin/Dcon

# /etc/init.d/Dcon: start and stop the Dcon MQTT message broker

test -x ${DAEMON} || exit 0

umask 022

. /lib/lsb/init-functions

# Are we running from init?
run_by_init() {
    ([ "$previous" ] && [ "$runlevel" ]) || [ "$runlevel" = S ]
}

export PATH="${PATH:+$PATH:}/usr/sbin:/sbin"

case "$1" in
  start)
	if init_is_upstart; then
	    exit 1
	fi
	log_daemon_msg "Starting network daemon:" "Dcon"
	if start-stop-daemon --start --quiet --oknodo --background  --make-pidfile --pidfile ${PIDFILE} --exec ${DAEMON} -- -c /etc/Dcon/Dcon.conf ; then
	    log_end_msg 0
	else
	    log_end_msg 1
	fi
	;;
  stop)
	if init_is_upstart; then
	    exit 0
	fi
	log_daemon_msg "Stopping network daemon:" "Dcon"
	if start-stop-daemon --stop --quiet --oknodo --pidfile ${PIDFILE}; then
	    log_end_msg 0
	    rm -f ${PIDFILE}
	else
	    log_end_msg 1
	fi
	;;


  reload|force-reload)
	if init_is_upstart; then
	    exit 1
	fi
	log_daemon_msg "Reloading network daemon configuration:" "Dcon"
        if start-stop-daemon --stop --signal HUP --quiet --oknodo --pidfile $PIDFILE; then
            log_end_msg 0
        else
            log_end_msg 1
        fi	
	;;

  restart)
	if init_is_upstart; then
	    exit 1
	fi
	log_daemon_msg "Restarting network daemon:" "Dcon"
	if start-stop-daemon --stop --quiet --oknodo --retry 30 --pidfile ${PIDFILE}; then
	    rm -f ${PIDFILE}
	fi
	if start-stop-daemon --start --quiet --oknodo --background --make-pidfile --pidfile ${PIDFILE} --exec ${DAEMON} -- -c /etc/Dcon/Dcon.conf ; then
	    log_end_msg 0
	else
	    log_end_msg 1
	fi
	;;

  try-restart)
	if init_is_upstart; then
	    exit 1
	fi
	log_daemon_msg "Restarting Dcon message broker" "Dcon"
	set +e
	start-stop-daemon --stop --quiet --retry 30 --pidfile ${PIDFILE}
	RET="$?"
	set -e
	case $RET in
	    0)
		# old daemon stopped
		rm -f ${PIDFILE}
		if start-stop-daemon --start --quiet --oknodo --background --make-pidfile --pidfile ${PIDFILE} --exec ${DAEMON} -- -c /etc/Dcon/Dcon.conf ; then
		    log_end_msg 0
		else
		    log_end_msg 1
		fi
		;;
	    1)
		# daemon not running
		log_progress_msg "(not running)"
		log_end_msg 0
		;;
	    *)
		# failed to stop
		log_progress_msg "(failed to stop)"
		log_end_msg 1
		;;
	esac
	;;

  status)
	if init_is_upstart; then
	    exit 1
	fi
	status_of_proc -p ${PIDFILE} ${DAEMON} Dcon && exit 0 || exit $?
	;;

  *)
	log_action_msg "Usage: /etc/init.d/Dcon {start|stop|reload|force-reload|restart|try-restart|status}"
	exit 1
esac

exit 0








































# ### BEGIN INIT INFO
# # Provides: control of relay boards         
# # Required-Start:    $remote_fs $syslog
# # Required-Stop:     $remote_fs $syslog
# # Default-Start:     2 3 4 5
# # Default-Stop:      0 1 6
# # Short-Description: Dcon daemon
# # Description:       The daemon updates relays
# ### END INIT INFO

# # Author: Mark Mara
# #

# # Do NOT "set -e"

# # PATH should only include /usr/* if it runs after the mountnfs.sh script
# PATH=/sbin:/usr/sbin:/bin:/usr/bin
# DESC="Programmable controller deamon"
# NAME=Dcon
# DAEMON=/usr/local/sbin/$NAME
# DAEMON_ARGS="--options args"
# PIDFILE=/var/run/$NAME.pid
# SCRIPTNAME=/etc/init.d/$NAME

# # Exit if the package is not installed
# test -f $DAEMON || exit 0

# # Read configuration variable file if it is present
# [ -r /etc/default/$NAME ] && . /etc/default/$NAME

# # Load the VERBOSE setting and other rcS variables
# . /lib/init/vars.sh

# # Define LSB log_* functions.
# # Depend on lsb-base (>= 3.2-14) to ensure that this file is present
# # and status_of_proc is working.
# . /lib/lsb/init-functions

# #
# # Function that starts the daemon/service
# #
# do_start()
# {
# 	# Return
# 	#   0 if daemon has been started
# 	#   1 if daemon was already running
# 	#   2 if daemon could not be started
# 	log_daemon_msg "Starting scheduler daemon" "Dcon"
# 	start-stop-daemon --start --quiet --pidfile $PIDFILE --exec $DAEMON --test > /dev/null \
# 		|| return 1
# 	start-stop-daemon --start --quiet --pidfile $PIDFILE --exec $DAEMON -- \
# 		$DAEMON_ARGS \
# 		|| return 2
# 	# Add code here, if necessary, that waits for the process to be ready
# 	# to handle requests from services started subsequently which depend
# 	# on this one.  As a last resort, sleep for some time.
# }

# #
# # Function that stops the daemon/service
# #
# do_stop()
# {
# 	# Return
# 	#   0 if daemon has been stopped
# 	#   1 if daemon was already stopped
# 	#   2 if daemon could not be stopped
# 	#   other if a failure occurred
# 	log_daemon_msg "Stopping Don daemon" "Dcon"
# 	start-stop-daemon --stop --quiet --retry=TERM/30/KILL/5 --pidfile $PIDFILE --name $NAME
# 	RETVAL="$?"
# 	[ "$RETVAL" = 2 ] && return 2
# 	# Wait for children to finish too if this is a daemon that forks
# 	# and if the daemon is only ever run from this initscript.
# 	# If the above conditions are not satisfied then add some other code
# 	# that waits for the process to drop all resources that could be
# 	# needed by services started subsequently.  A last resort is to
# 	# sleep for some time.
# 	start-stop-daemon --stop --quiet --oknodo --retry=0/30/KILL/5 --exec $DAEMON
# 	[ "$?" = 2 ] && return 2
# 	# Many daemons don't delete their pidfiles when they exit.
# 	rm -f $PIDFILE
# 	return "$RETVAL"
# }

# #
# # Function that sends a SIGHUP to the daemon/service
# #
# do_reload() {
# 	#
# 	# If the daemon can reload its configuration without
# 	# restarting (for example, when it is sent a SIGHUP),
# 	# then implement that here.
# 	#
# 	start-stop-daemon --stop --signal 1 --quiet --pidfile $PIDFILE --name $NAME
# 	return 0
# }

# case "$1" in
#   start)
# 	[ "$VERBOSE" != no ] && log_daemon_msg "Starting $DESC" "$NAME"
# 	do_start
# 	case "$?" in
# 		0|1) [ "$VERBOSE" != no ] && log_end_msg 0 ;;
# 		2) [ "$VERBOSE" != no ] && log_end_msg 1 ;;
# 	esac
# 	;;
#   stop)
# 	[ "$VERBOSE" != no ] && log_daemon_msg "Stopping $DESC" "$NAME"
# 	do_stop
# 	case "$?" in
# 		0|1) [ "$VERBOSE" != no ] && log_end_msg 0 ;;
# 		2) [ "$VERBOSE" != no ] && log_end_msg 1 ;;
# 	esac
# 	;;
#   status)
# 	status_of_proc "$DAEMON" "$NAME" && exit 0 || exit $?
# 	;;
#   #reload|force-reload)
# 	#
# 	# If do_reload() is not implemented then leave this commented out
# 	# and leave 'force-reload' as an alias for 'restart'.
# 	#
# 	#log_daemon_msg "Reloading $DESC" "$NAME"
# 	#do_reload
# 	#log_end_msg $?
# 	#;;
#   restart|force-reload)
# 	#
# 	# If the "reload" option is implemented then remove the
# 	# 'force-reload' alias
# 	#
# 	log_daemon_msg "Restarting $DESC" "$NAME"
# 	do_stop
# 	case "$?" in
# 	  0|1)
# 		do_start
# 		case "$?" in
# 			0) log_end_msg 0 ;;
# 			1) log_end_msg 1 ;; # Old process is still running
# 			*) log_end_msg 1 ;; # Failed to start
# 		esac
# 		;;
# 	  *)
# 		# Failed to stop
# 		log_end_msg 1
# 		;;
# 	esac
# 	;;
#   *)
# 	#echo "Usage: $SCRIPTNAME {start|stop|restart|reload|force-reload}" >&2
# 	echo "Usage: $SCRIPTNAME {start|stop|status|restart|force-reload}" >&2
# 	exit 3
# 	;;
# esac