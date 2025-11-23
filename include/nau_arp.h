#ifndef __NAU_ARP_H__
#define	__NAU_ARP_H__


/////////////////////////////////////////////////////////////////////////
typedef struct _NAU_ARP_TM {

	std::string		addr;
	std::string		mac;
}NAU_ARP_TM, *PNAU_ARP_TM;


//////////////////////////////////////////////////////////////////////////
std::vector<NAU_ARP_TM> nau_arp_ipv4(
	std::string			iface);
std::vector<NAU_ARP_TM> nau_arp_ipv6(
	std::string			iface);

void
nau_arp_out(std::vector<NAU_ARP_TM>& items);


#endif

