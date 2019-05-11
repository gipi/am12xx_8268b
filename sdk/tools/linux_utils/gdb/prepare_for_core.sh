#!/bin/sh

echo "1" > /proc/sys/kernel/core_uses_pid
cat /proc/sys/kernel/core_uses_pid
ulimit -c unlimited

