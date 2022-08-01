#!/bin/sh

while true
do
      read line < infeed.fifo
      echo $line
      [ "$line" = "OK" ] && { exit 0; }
done
