rm /tmp/blacklistpipe
iptables -F
#ipset -X blacklist
pkill -9 python
