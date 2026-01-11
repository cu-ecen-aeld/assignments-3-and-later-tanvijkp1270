#!/bin/sh
set -x
DAEMON=/usr/bin/aesdsocket
PIDFILE=/var/run/aesdsocket.pid
DAEMON_OPTS="-d"

case "$1" in
  start)
    echo "Starting aesdsocket"
    start-stop-daemon -S -x "$DAEMON" -p "$PIDFILE" -- "$DAEMON" -d
    ;;
  stop)
    echo "Stopping aesdsocket"
    start-stop-daemon -K -s TERM -p "$PIDFILE"
    ;;
  *)
    echo "Usage: $0 {start|stop}"
  exit 1
esac
exit 0
