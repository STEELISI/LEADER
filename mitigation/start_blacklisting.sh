rm /tmp/blacklistpipe
iptables -F
ipset -X blacklist
/usr/bin/python blacklistd.py 2>&1> /proj/FRADE/output_blacklistd &