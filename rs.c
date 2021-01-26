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
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char** argv) {
	if(argc != 2) {
		fprintf(stderr, "Pass in the interface on which to send the RS as an argument\n");
		return -1;
	}
	char* interface = argv[1];

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
	int if_index = if_nametoindex(interface);
	int err = setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_IF, &if_index, sizeof(if_index));
	if(-1 == err) {
		fprintf(stderr,"Couldn't set socket to broadcast %d\n",errno);
		return -1;
	}
	unsigned int hops=255;
	err = setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &hops, sizeof(hops));
	if(-1 == err) {
		fprintf(stderr,"Couldn't set socket hops %d\n",errno);
		return -1;
	}

	struct sockaddr_in6 allrouters = {0};
	allrouters.sin6_family=AF_INET6;
	allrouters.sin6_len=sizeof(allrouters);
	allrouters.sin6_addr = (struct in6_addr){{{0xff,0x02,0,0,0,0,0,0,0,0,0,0,0,0,0,0x02}}};
	allrouters.sin6_scope_id=if_nametoindex(interface);

	struct in6_addr lladdr = {0};

	char buf[INET6_ADDRSTRLEN+1] = {0};
	const char* res = inet_ntop(AF_INET6,&allrouters.sin6_addr,buf,sizeof(buf));
	printf("allrouters is %s\n",res);

	//Note checksum is allegedly filled in by kernel on outgoing messages
	struct nd_router_solicit hdr = {0};
	hdr.nd_rs_hdr = (struct icmp6_hdr){ND_ROUTER_SOLICIT, 0, 0, 0};

	err = sendto(s, &hdr, sizeof(hdr), 0, (struct sockaddr*)&allrouters, sizeof(allrouters));
	if(-1 == err) {
		fprintf(stderr,"Error sending: %d\n",errno);
		return -1;
	}
	return 0;
}
