#ifndef __NAU_IFACE_H__
#define	__NAU_IFACE_H__


//////////////////////////////////////////////////////////////////////////
int
nau_iface_main(
	const char		*ifname);
int
nau_iface_addr(
	std::string&	addr,
	int				family,
	const char		*ifname);

std::string
nau_iface_4to6(
	std::string&	prefix,
	std::string&	address);
std::string
nau_iface_6to4(
	std::string&	prefix,
	std::string&	address);


#endif

