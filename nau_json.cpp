//////////////////////////////////////////////////////////////////////////
#include "nlohmann/json.hpp"
#include "nau_main.h"


//////////////////////////////////////////////////////////////////////////
using json = nlohmann::json;


//////////////////////////////////////////////////////////////////////////
std::string		nau_proto;
std::string		nau_ifd_ifn;
std::string		nau_ifd_ip;
std::string		nau_ifu_ifn;
std::string		nau_ifu_ip;
std::string		nau_ifu_v6;
std::string		nau_ap_id;
std::string		nau_mng_ip;


//////////////////////////////////////////////////////////////////////////
int
nau_json_cfg_load(
	const char		*fname)
{
	int			rc = 0;

	// Step1: 打开配置文件
	std::ifstream input_file(fname);
	if( !input_file.is_open() ) {

		std::cerr << "Open the file failed!" << fname << std::endl;
		return -1;
	}

	// 解析配置
	try {

		json data;
		input_file >> data;
		nau_proto  = data["sat_protocol"].get<std::string>();

		const json& data0 = data["if_down"].at(0);
		nau_ifd_ifn= data0["iface"].get<std::string>();
		nau_ifd_ip = data0["address"].get<std::string>();

		const json& data1 = data["if_up"].at(0);
		nau_ifu_ifn= data1["iface"].get<std::string>();
		nau_ifu_ip = data1["address"].get<std::string>();
		nau_ifu_v6 = data1["prefix"].get<std::string>();

		nau_ap_id  = data["NAU_ID"].get<std::string>();
	    nau_mng_ip = data["MNG_ip"].get<std::string>();
	} catch (const json::parse_error& e) {

		std::cerr << "Parse Error!" << e.what() << std::endl;
		rc = -2;
	} catch (const json::out_of_range& e) {

		std::cerr << "Key is not exist!" << e.what() << std::endl;
		rc = -3;
	}
	return rc;
}
void
nau_json_cfg_show(void)
{
	std::cerr << "Protocol    : " << nau_proto  << std::endl;
	std::cerr << "if_down ifn : " << nau_ifd_ifn<< std::endl;
	std::cerr << "if_down ip  : " << nau_ifd_ip << std::endl;
	std::cerr << "if_up ifn   : " << nau_ifu_ifn<< std::endl;
	std::cerr << "if_up ip    : " << nau_ifu_ip << std::endl;
	std::cerr << "if_up prefix: " << nau_ifu_v6 << std::endl;
	std::cerr << "nau_id      : " << nau_ap_id << std::endl;
	std::cerr << "mng_id      : " << nau_mng_ip << std::endl;
}
int
nau_json_tm_save(
	const char		*fname,
	std::vector<NAU_ITEM_TM>& items)
{
	json		new_data;

	// 创建数据
	new_data["tm"] = json::array();
	for(const auto& item : items) {

		json dict;
		dict["uidip"] = item.uidip;
		dict["uidmac"]= item.uidmac;
		dict["ifup4"] = item.ifup4;
		dict["ifup6"] = item.ifup6;
		dict["proto"] = item.proto;
		dict["nauid"] = item.nauid;
		new_data["tm"].push_back(dict);
	}

	// 打开输出文件流
	std::ofstream out_file(fname);
	if( !out_file.is_open() ) {

		std::cerr << "Create file failed!"<< std::endl;
		return -1;
	}
	// 写入文件, 缩进为4个空格
	out_file << std::setw(4) << new_data << std::endl;
	return 0;
}
std::vector<NAU_ITEM_TM>
nau_json_tm_load(
	const char		*fname)
{
	std::vector<NAU_ITEM_TM>	items;

	// Step1: 打开JSON文件
	std::ifstream input_file(fname);
	if( !input_file.is_open() ) {

		std::cerr << "Open the file failed!" << fname << std::endl;
		goto EXIT;
	}

	// 解析数据
	try {

		json data;
		input_file >> data;

		if(data.contains("tm") && data["tm"].is_array()) {

			const json& tm_array = data["tm"];
			for(const auto& tm : tm_array) {

				if( !tm.is_object() ) {

					continue;
				}
				std::string 	uidip = tm.value("uidip", "N/A");
				std::string 	uidmac= tm.value("uidmac", "N/A");
				std::string 	ifup4 = tm.value("ifup4", "N/A");
				std::string 	ifup6 = tm.value("ifup6", "N/A");
				std::string 	proto = tm.value("proto", "N/A");
				std::string 	nauid = tm.value("nauid", "N/A");
				items.push_back({uidip, uidmac, ifup4, ifup6, proto, nauid});
			}
		}
	} catch (const json::parse_error& e) {

		std::cerr << "Error: JSON parsing failed:" << e.what() << std::endl;
	} catch (const json::out_of_range& e) {

		std::cerr << "Key is not exist!" << e.what() << std::endl;
	}

EXIT:
	// 删除文件
	remove(fname);
	return items;
}

int
nau_json_tm_save_gre(
	const char		*fname,
	std::vector<UNIT>& items)
{
	json		new_data;

	// 创建数据
	new_data["tm"] = json::array();
	for(const auto& item : items) {

		json dict;
		dict["uidip"] = item.UIDIP;
		dict["uidmac"]= item.UIDMAC;
		dict["ifup4"] = item.IFUP4;
		dict["ifup6"] = item.IFUP6;
		dict["sat_protocol"] = item.sat_protocol;
		dict["nauid"] = item.NAU_ID;
		dict["ctime"] = item.CTIME;
		dict["utime"] = item.UTIME;
		new_data["tm"].push_back(dict);
	}

	// 打开输出文件流
	std::ofstream out_file(fname);
	if( !out_file.is_open() ) {

		std::cerr << "Create file failed!"<< std::endl;
		return -1;
	}
	// 写入文件, 缩进为4个空格
	out_file << std::setw(4) << new_data << std::endl;
	return 0;
}

std::vector<UNIT>
nau_json_tm_load_gre(
	const char		*fname)
{
	std::vector<UNIT>	items;

	// Step1: 打开JSON文件
	std::ifstream input_file(fname);
	if( !input_file.is_open() ) {

		std::cerr << "Open the file failed!" << fname << std::endl;
		goto EXIT;
	}

	// 解析数据
	try {

		json data;
		input_file >> data;

		if(data.contains("tm") && data["tm"].is_array()) {

			const json& tm_array = data["tm"];
			for(const auto& tm : tm_array) {

				if( !tm.is_object() ) {

					continue;
				}
				std::string 	UIDIP = tm.value("uidip", "N/A");
				std::string 	UIDMAC= tm.value("uidmac", "N/A");
				std::string 	IFUP4 = tm.value("ifup4", "N/A");
				std::string 	IFUP6 = tm.value("ifup6", "N/A");
				std::string 	sat_protocol = tm.value("sat_protocol", "N/A");
				std::string 	NAU_ID = tm.value("nauid", "N/A");
				std::string 	CTIME = tm.value("ctime", "N/A");
				std::string 	UTIME = tm.value("utime", "N/A");
				items.push_back({UIDIP, UIDMAC, IFUP4, IFUP6, sat_protocol, NAU_ID, CTIME, UTIME});
			}
		}
	} catch (const json::parse_error& e) {

		std::cerr << "Error: JSON parsing failed:" << e.what() << std::endl;
	} catch (const json::out_of_range& e) {

		std::cerr << "Key is not exist!" << e.what() << std::endl;
	}

EXIT:
	return items;
}