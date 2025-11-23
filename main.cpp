//////////////////////////////////////////////////////////////////////////
#include "nau_main.h"


//////////////////////////////////////////////////////////////////////////
void
nau_test_cfg(void)
{
	int			rc;

	rc = nau_json_cfg_load("/home/NAU_config.json");
	if(0 != rc) {

		return ;
	}
	nau_json_cfg_show();
}
void
nau_test_db(void)
{
	std::vector<NAU_ITEM_TM> items;
	NAU_ITEM_TM tm1;
	NAU_ITEM_TM tm2;
	NAU_ITEM_TM tm3;
	NAU_ITEM_TM tm4;
	int			val;

	// 准备数据
	tm1.uidip = "192.118.0.1/24";
	tm1.uidmac= "";
	tm1.ifup4 = "192.114.1.1/24";
	tm1.ifup6 = "192.116.1.1/24";
	tm1.proto = "6";
	tm1.nauid = "1";

	tm2.uidip = "192.128.0.1/24";
	tm2.uidmac= "";
	tm2.ifup4 = "192.124.1.1/24";
	tm2.ifup6 = "192.126.1.1/24";
	tm2.proto = "6";
	tm2.nauid = "2";

	tm3.uidip = "192.138.0.1/24";
	tm3.uidmac= "";
	tm3.ifup4 = "192.134.1.1/24";
	tm3.ifup6 = "192.136.1.1/24";
	tm3.proto = "6";
	tm3.nauid = "3";

	tm4.uidip = "192.148.0.1/24";
	tm4.uidmac= "";
	tm4.ifup4 = "192.144.1.1/24";
	tm4.ifup6 = "192.146.1.1/24";
	tm4.proto = "6";
	tm4.nauid = "4";

	// 进行测试
	nau_db_open("/home/itrust/NAU_DB");
	nau_db_init();

	nau_db_add(NAU_DB_LTB, &tm1);
	nau_db_add(NAU_DB_LTB, &tm2);
	nau_db_add(NAU_DB_LTB, &tm3);
	nau_db_add(NAU_DB_LTB, &tm4);

	tm1.ifup4 = "172.114.1.1/24";
	tm1.ifup6 = "172.116.1.1/24";
	nau_db_upt(NAU_DB_LTB, &tm1);

	val = nau_db_count(NAU_DB_LTB, tm1.uidip, tm1.nauid);
	std::cerr << "COUNT : " << val << std::endl;

	items = nau_db_query(NAU_DB_LTB);
	std::cerr << "ITEMS : " << items.size() << std::endl;
	nau_json_tm_save("/tmp/test.json", items);

	nau_db_close();
}
void
nau_test_arp(void)
{
	std::vector<NAU_ARP_TM> ipv4_neighbors = nau_arp_ipv4("eth0");
	nau_arp_out(ipv4_neighbors);
	std::vector<NAU_ARP_TM> ipv6_neighbors = nau_arp_ipv6("eth0");
	nau_arp_out(ipv6_neighbors);
}
void
nau_test_iface(void)
{
	std::string		addr;

	nau_iface_addr(addr, AF_INET6, "eth0");
	std::cerr << "iface:eth0, addr:" << addr << std::endl;
	nau_iface_main("eth0");
}
void
nau_test_http(
	bool        nau)
{
	if( nau ) {

		std::cerr << "NAU" << std::endl;
		nau_http_post("127.0.0.1", 8080, "/nau", "/home/itrust/test.ini");
	}else {

		std::cerr << "Gateway" << std::endl;
		nau_http_svr("0.0.0.0", 8080, "/nau", "/tmp/", NAU_TYPE_GW);
	}
}


//////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
//	nau_test_cfg();
//	nau_test_http();
//	nau_test_db();
//	nau_test_arp();
//	nau_test_iface();

	// 系统正常数据
	if(argc == 2 && std::string(argv[1]) == "ap") {
//		nau_test_http(true);
		nau_main_ap();
	}else if(argc == 2 && std::string(argv[1]) == "gw") {
//		nau_test_http(false);
		nau_main_gw();
	}else {

		std::cerr << "Usage:" << std::endl;
		std::cerr << "  NAU mode: " << argv[0] << " ap" << std::endl;
		std::cerr << "  GW  mode: " << argv[0] << " gw" << std::endl;
		return 1;
	}
	printf("EXIT....\n");
	return 0;
}

