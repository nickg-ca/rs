#include <sys/socket.h> 
#include <netinet/in.h>
#include <netinet/icmp6.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h> 
#include <netinet/in.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <string.h>

int main(int argc, char** argv) {
	char* interface = "em0";

	struct protoent * pent = getprotobyname("ipv6-icmp");
	if(0 == pent) {
		fprintf(stderr,"Error finding ipv6-icmp protocol\n");
		return -1;
	}
	int proto = pent->p_proto;//IPPROTO_ICMPV6
	int s = socket(AF_INET6,SOCK_RAW,proto);
	if(-1 == s) {
		fprintf(stderr,"Error %d creating socket\n",errno);
		return -1;
	}
//Wasn't sure if this was needed but apparently not
#if 0
	int err = setsockopt(s, SOL_SOCKET, SO_BROADCAST, 0, 0);
	if(-1 == err) {
		fprintf(stderr,"Couldn't set socket to broadcast %d\n",errno);
		return -1;
	}
#endif
	struct sockaddr_in6 lladdr = {0};

	struct ifaddrs * ifap = 0;
	int err = getifaddrs(&ifap);
	if(-1 == err) {
		fprintf(stderr, "Error %d obtaining interface addresses\n",errno);
		return -1;
	}
	struct ifaddrs* ifa_next = ifap->ifa_next;
	int found = 0;
	while(ifa_next != 0) {
		if(!strcmp(interface,ifa_next->ifa_name) &&
				ifa_next->ifa_addr->sa_family == AF_INET6 &&
				((struct sockaddr_in6*)ifa_next->ifa_addr)->sin6_addr.__u6_addr.__u6_addr16[0]==0x80fe) {
			memcpy(&lladdr,ifa_next->ifa_addr,sizeof(lladdr));
			found = 1;
			break;
		}
		ifa_next = ifa_next->ifa_next;
	}
	ifa_next = 0;//don't want to accidentally reference this later

	//clean up ifap memory
	freeifaddrs(ifap);
	ifap=0;

	if(!found) {
		fprintf(stderr,"Could not obtain link local address for interface %s\n",interface);
		return -1;
	}

	//Note checksum is allegedly filled in by kernel on outgoing messages
	struct icmp6_hdr hdr = {ND_ROUTER_SOLICIT, 0, 0, 0};

	err = sendto(s, &hdr, sizeof(hdr), 0, (struct sockaddr*)&lladdr, sizeof(lladdr));
	if(-1 == err) {
		fprintf(stderr,"Error sending: %d\n",errno);
		return -1;
	}
	return 0;
}
