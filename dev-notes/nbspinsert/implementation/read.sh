#!/bin/sh

touch infeed.lock
mkfifo infeed.fifo

while true
do
      read line < infeed.fifo
      echo $line
      [ "$line" = "OK" ] && { break; }
done

rm -f infeed.fifo infeed.lock
