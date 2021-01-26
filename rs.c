#include <sys/socket.h> 
#include <netinet/in.h>
#include <netinet/icmp6.h>
#include <stdio.h>
#include <errno.h>
#include <net/if.h>

int main(int argc, char** argv) {
	if(argc != 2) {
		fprintf(stderr, "Pass in the interface on which to send the RS as an argument\n");
		return -1;
	}
	char* interface = argv[1];

	int s = socket(AF_INET6,SOCK_RAW,IPPROTO_ICMPV6);
	if(-1 == s) {
		fprintf(stderr,"Error %d creating socket\n",errno);
		return -1;
	}

	//See https://tools.ietf.org/html/rfc4861 section 4.1 (Router Solicitation Message Format)
	unsigned int hops=255;
	int err = setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &hops, sizeof(hops));
	if(-1 == err) {
		fprintf(stderr,"Couldn't set socket hops %d\n",errno);
		return -1;
	}

	struct sockaddr_in6 allrouters = {0};
	allrouters.sin6_family=AF_INET6;
	allrouters.sin6_len=sizeof(allrouters);
	//ff02::02 is the link-local all routers multicast address
	allrouters.sin6_addr = (struct in6_addr){{{0xff,0x02,0,0,0,0,0,0,0,0,0,0,0,0,0,0x02}}};
	//link local messages require a scope id so that the system knows which interface this is for
	allrouters.sin6_scope_id=if_nametoindex(interface);

	//The checksum is filled in by kernel on outgoing messages so we don't have to do anything here
	struct nd_router_solicit hdr = {0};
	hdr.nd_rs_hdr = (struct icmp6_hdr){ND_ROUTER_SOLICIT, 0, 0, 0};

	err = sendto(s, &hdr, sizeof(hdr), 0, (struct sockaddr*)&allrouters, sizeof(allrouters));
	if(-1 == err) {
		fprintf(stderr,"Error sending: %d\n",errno);
		return -1;
	}
	return 0;
}
