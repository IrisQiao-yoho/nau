//////////////////////////////////////////////////////////////////////////
#include "nau_main.h"


//////////////////////////////////////////////////////////////////////////
bool nau_arp_str2mac(unsigned char mac_bin[6], const std::string& mac_str)
{
	return true;
}
std::string nau_arp_mac2str(const unsigned char mac[6])
{
	std::stringstream ss;
	ss << std::hex << std::setfill('0');
	for(int i = 0; i < 6; ++i) {
		ss << std::setw(2) << (int)mac[i];
		if(i < 6 - 1) {
			ss << ":";
		}
	}
	return ss.str();
}


//////////////////////////////////////////////////////////////////////////
std::vector<NAU_ARP_TM>
nau_arp_ipv4(
	std::string			iface)
{
	std::vector<NAU_ARP_TM>	items;
	std::ifstream	arp_file("/proc/net/arp");
	std::string		line;
	NAU_ARP_TM		item;

	// Step1: 检查文件是否存在
	if( !arp_file.is_open() ) {

		std::cerr << "Open the file(/proc/net/arp) is failed!" << std::endl;
		return items;
	}
	getline(arp_file, line); // 忽略标题行

	// Step2: 解析文件内容
	while( getline(arp_file, line) ) {

		if( line.empty() ) {
			continue;
		}
		std::stringstream ss(line);
		std::string ip_address, hw_type, flags, hw_address, mask, device;

		// 解析格式: IP-address HW-type     Flags HW-address Mask Device
		ss >> ip_address >> hw_type >> flags >> hw_address >> mask >> device;
		if(hw_address == "00:00:00:00:00:00" || device != iface) {
			continue;
		}
		items.push_back({ip_address + "/32", hw_address});
	}
	return items;
}
std::vector<NAU_ARP_TM>
nau_arp_ipv6(
	std::string			iface)
{
	std::vector<NAU_ARP_TM>	items;
	char			buff[512];
	std::string		line;

	// Step1: 执行输出命令
	FILE *pipe = popen("ip -6 neigh show", "r");
	if(NULL == pipe) {

		std::cerr << "Get IPv6 Neigh Failed!" << std::endl;
		return items;
	}

	// Step2: 逐行解析命令输出
	while(fgets(buff, sizeof(buff), pipe) != NULL) {

		line = buff;
		std::stringstream ss(line);
		std::string token;
		std::vector<std::string> tokens;

		// 解析一行
		while(ss >> token) {

			tokens.push_back(token);
		}
		if(tokens.size() < 6 || "lladdr" != tokens[3] || iface != tokens[2]) {

			continue;
		}
		if("REACHABLE" == tokens[5] || "PERMANENT" == tokens[5]) {

			items.push_back({tokens[0] + "/120", tokens[4]});
		}
	}

	// Step3: 关闭管道
	pclose(pipe);
	return items;
}

void
nau_arp_out(std::vector<NAU_ARP_TM>& items)
{
	std::cout << std::left << std::setw(40) << "IP-Address" << std::setw(20) << "HW-Address" << std::endl;

	// 输出ARP信息
	for(const auto& item : items) {

		std::cout << std::left << std::setw(40) << item.addr << std::setw(20) << item.mac << std::endl;
	}
}

