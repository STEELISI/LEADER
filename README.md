# LEADER: Low-Rate Denial of Service Defense

## Members and Collaborators
* Haoda Wang
* Christophe Hauser
* Jelena Mirkovic
* Nicolaas Weideman
* Rajat Tandon

## Motivation
Low-rate denial-of-service(LRD) attacks are often hard to detect at the network level as they consume little bandwidth. It is the intricacies in the payloads and the dynamics of the attack trafic that induces denial-of-service on servers when processed by specific hardware and software. We introduce LEADER, a hybrid approach for application-agnostic and attack-agnostic detection and mitigation of LRD attacks. LEADER operates by learning normal patterns of network, application and system-level resources when processing legitimate external requests. It relies on a novel combination of runtime, system and network monitoring, as well as offline binary program analysis.

## Our Solution
LEADER leverages a novel combination of runtime monitoring and offline binary program analysis to protect a deploying server against different variants of Low-Rate DDoS attacks. It effectively prevents external service requests from misusing system resources. The novelty of our approach lies in our usage of connection life stages and code path abstractions, which are built from monitoring the system at network, OS and application levels. Figure 1 shows a high-level view of the abstraction levels at which Leader operates and the system overview. It also illustrates the trade-offs between the accuracy of semantic reasoning and monitoring cost and delay. OS-level instrumentation allows an observer to gain more insights about the application’s semantics and program analysis offers the highest level of semantic reasoning. LRD attacks usually involve several incoming service requests arriving at the server that are expensive/slow processing leading to resource depletion. Figure 2 shows the architecture diagram of LEADER and its sub-modules.

![architecture](https://steel.isi.edu/Projects/leader/images/1.png)
![architecture 2](https://steel.isi.edu/Projects/leader/images/3.png)

## Behaviour Profiling and Attack Detection
SystemTap is an open source project that allows users to dynamically instrument running production Linux operating systems. It assists the diagnosis of performance related issues, but we use it to probe individual connections at system call level details. For each function call to \texttt{socket.c}, we collect information including the caller's thread ID, source IP address and port number, the time spent by the function call, memory consumed, page faults occurred, file descriptors used, and CPU cycles elapsed.

A thread of a given process of the Apache server is associated with a single connection at a given time. Utilizing this information, we came up with novel structures called Connection Life Stages for every connection, which captures the sequence of internal function calls to net/socket.c and the resource usage by each call for the function. We set the start of a connection at the `sockfd_lookup_light` call after a `SyS_accept4` call and the end of a connection at the `SyS_shutdown` or `sock_destroy_inode` function call. The same thread also includes internal connections, if any, that may be triggered as a result of serving a request (eg: a database query).

![connection life stages](https://steel.isi.edu/Projects/leader/images/2.png)

## Experimental Setup
The experimental topology for LEADER includes a node for running the Web Server, a node for generating legitimate traffic, and 8 attacker nodes connected by a LAN on Emulab Testbed, as shown in figure 4. The server node rus Apache 2.4.29 and a vulnerable version of PHP 5.6. We simulated legitimate traffic with 100 active legitimate users requesting the homepage, and created attack traffic with up to 1000 attack connections/second per attacker. The prober module captured the profiling data throughout the experiment, using Systemtap scripts, which is used for building Connection Life Stages. For training the baseline model, we captured and analyzed only legitimate traffic, while both the attack and legitimate traffic generator were turned on to capture the data for testing. We classify connections as legitimate/attack using different approaches.

![topology](https://steel.isi.edu/Projects/leader/images/4.png)

## Scoring Connections
We scored each connection to classify it either as a legitimate connection or a malicious connection using different Machine Learning classification approaches such as Naive Bayes, Decision Trees, and Random Forests. These work well when trained and tested on the same attack set, but do not work well for unseen attack scenarios. While 1-class SVM intuitively seemed to fit the use case, it did not perform well. We also used Mean + 5 times standard deviation and Elliptic Envelope, both of which had good overall accuracy. The same is shown below.

![success](https://steel.isi.edu/Projects/leader/images/5.png)
