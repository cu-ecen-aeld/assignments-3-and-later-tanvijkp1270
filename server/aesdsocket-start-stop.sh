#!/bin/sh
case "$1" in
  start)
    echo "Starting aesdsocket"
     start-stop-daemon -S -n aesdsocket -d -a /usr/bin/aesdsocket
    ;;
  stop)
    echo "Stopping aesdsocket"
    start-stop-daemon -K --signal SIGTERM -n aesdsocket -d
    ;;
  *)
    echo "Usage: $0 {start|stop}"
  exit 1
esac
exit 0
