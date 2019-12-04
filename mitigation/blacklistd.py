import random
import sys
import socket, struct
import subprocess
import thread, threading
import time
import os
import errno

blacklisted = {}
SLEEP = 2 
q = []
#PIPE = "/proj/FRADE/blklst-mod-test-data/ips_extracted_from_rsyslog"
PIPE = "/tmp/blacklistpipe"
#PIPE = "out"
ipsetTIMEOUT = str(300)
pktsthresh = 600000
attackchecker_sleeptime = 300
my_mutex = threading.Lock()  

"""
def getDataFromConf():
	global PIPE
	try:
		with open(frade_conf,"r") as fc:
			for line in fc:
				if line == "\n" or line.startswith("#"):
					continue
				else:
					key = line.split("=")[0]
					if key == "PIPE":
						PIPE = line.split("=")[1].rstrip("\n")
					else:
						continue
			if len(PIPE) == 0:
				raise ValueError("PIPE is not defined in CONF file")
	except IOError:
		print "could not find/open conf file!!"

"""

def ip2long(ip):
    """
    Convert an IP string to long.
    It is nothing but a mapping into integer space i.e 0.0.0.0 is zero and 255.255.255.255 is ~4bn.
    """
    print ip
    packedIP = socket.inet_aton(ip)
    return struct.unpack("<L", packedIP)[0]

def long2ip(longint):
	"""
	Convert long to an IP string
	"""
        try:
            return socket.inet_ntoa(struct.pack('<L', longint))
        except:
            return "255.255.255.255"

def readPipe():
	"""
	This function reads ip from pipe. Checks if it is already there in the blacklisted dictionary.
	If it is not there, it inserts the ip into queue.
	"""
	global blacklisted
	global q, my_mutex
	
	try:
		os.mkfifo(PIPE)
	except OSError as oe:
		if oe.errno != errno.EEXIST:
			raise

	with open(PIPE,"r") as f:
		while True:
			line = f.readline()
			if len(line) != 0:
                            #Convert IP into interger.
                            #print "Read line: " + line
                            ip = line.rstrip("\n")
                            longip = long2ip(long(ip)) #longip = ip2long(ip)
                            if longip not in blacklisted:
                                blacklisted[longip] = ""
                                my_mutex.acquire()
                                q.append(longip)
                                my_mutex.release()
                                #print "Queued: " + longip + " time: " + str(time.time())
			else:
                            print "Total len ", len(q)
                            time.sleep(SLEEP)
                            continue


def blacklist():
	
	#This function continuously reads from q and blacklist IPs using IPtables
	
	global q, my_mutex
	try:
		#ipsetcmd = "ipset create blacklist hash:ip timeout "+ ipsetTIMEOUT +" hashsize 1000000 maxelem 1000000"
		ipsetcmd = "ipset create blacklist hash:ip hashsize 1000000 maxelem 1000000"
        	run_ipsetcmd = subprocess.check_output(ipsetcmd,shell=True)
	except:
		print "ipset blacklist already created"

	
	try:
                #cmd = "iptables -A FORWARD -m set --match-set blacklist src -j DROP"
        	cmd = "iptables -A INPUT -m set --match-set blacklist src -j DROP"
        	result = subprocess.check_output(cmd, shell=True)
	except:
		print "iptable rule already inserted"

        print "Starting"
	while True:
            if len(q) == 0:
                print "Done"
                time.sleep(1)
            else:
                print "Inserting  ", len(q);
                my_mutex.acquire()
                cmd = ""
                for longip in q:
                    ip = longip
                    cmd = cmd +  "ipset add blacklist " + ip + "&\n"
                    if (len(cmd) > 10000):
                        try:
                            print "Blacklisting1 ", len(cmd)
                            result = subprocess.check_output(cmd, shell=True)
                            #print "blacklisted: " + ip + " time: " + str(time.time())
                            #	sys.stdout.flush()
                            cmd = ""
                        except:
                            cmd=""
                            pass
                if (len(cmd) > 0):
                    try:
                        print "Blacklisting1 ", len(cmd)
                        result = subprocess.check_output(cmd, shell=True)
                        #print "blacklisted: " + ip + " time: " + str(time.time())
                        #	sys.stdout.flush()
                    except:
                        cmd=""
                        pass
                del q[:]
                my_mutex.release()


def checkIfAttackStopped():
	'''
	This function keeps checking the number of packets matched against our blacklisting iptable rule.
	If the # of packets drop below certain threshold, we consider that the attack has stopped. So we remove all ips from ipset.
	'''

	while True:
	        #cmd = "iptables -L FORWARD -v"
                cmd = "iptables -L INPUT -v"
        	result = subprocess.check_output(cmd,shell=True)
		pkts = result.split("\n")[2].lstrip().split(" ")[0]
        	pkts = long(pkts.split("K")[0])
        	if pkts <= pktsthresh:
                	cmd1 = "ipset flush blacklist"
			try:
                        	res = subprocess.check_output(cmd1,shell=True)
                	except:
                        	raise ValueError("failed to flush the blacklist ipset!!")
		cmd = "iptables -Z INPUT 1"
		try:
			result = subprocess.check_output(cmd,shell=True)
		except:
			raise ValueError("failed to reset the packet counter!!")
		time.sleep(attackchecker_sleeptime)


if __name__ == "__main__":
	#getDataFromConf()

	try:
		thread.start_new_thread(readPipe,())
		thread.start_new_thread(blacklist,())
		#thread.start_new_thread(checkIfAttackStopped,())
	except:
		print "Error: Unable to start thread"

	while 1:
		pass
					

