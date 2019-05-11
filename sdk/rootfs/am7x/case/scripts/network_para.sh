#!/bin/sh
echo "set the tcp paragram"
echo "873200" > proc/sys/net/core/wmem_max
echo "873200" > /proc/sys/net/core/rmem_max

echo "8192 436600 873200" > /proc/sys/net/ipv4/tcp_wmem
echo "32768 436600 873200" > /proc/sys/net/ipv4/tcp_rmem
echo "786432 1048576 1572864" > /proc/sys/net/ipv4/tcp_mem
echo "3000" > /proc/sys/net/core/netdev_max_backlog




echo "256" > /proc/sys/net/core/somaxconn
echo "2048" > /proc/sys/net/ipv4/tcp_max_syn_backlog
echo "5"  > /proc/sys/net/ipv4/tcp_retries2
echo "1800"  > /proc/sys/net/ipv4/tcp_keepalive_time
echo "30"  > /proc/sys/net/ipv4/tcp_keepalive_intvl
echo "3"  > /proc/sys/net/ipv4/tcp_keepalive_probes

echo "set ok!"