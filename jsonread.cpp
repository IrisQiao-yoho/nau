#include <iostream>
#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// 1. 结构体
typedef struct Unit {
    std::string UIDIP;
    std::string UIDMAC;
    std::string IFUP4;
    std::string IFUP6;
    std::string sat_protocol;
    std::string NAU_ID;
    std::string CTIME;
    std::string UTIME;
}UNIT;

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
				std::string 	sat_protocol = tm.value("proto", "N/A");
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

Unit find_subnet_fromLocalMap(std::vector<Unit> tms)
{
    for (const auto& tm : tms) 
	{
    	if (tm.UIDMAC.empty()) 
		{
        	return tm;
    	}
	}
	return Unit{}; 
}

std::vector<Unit> find_subnet_fromRemoteMap(std::vector<Unit> tms)
{
    std::vector<Unit> remote_units;
    for (const auto& tm : tms) {
    if (tm.UIDMAC.empty()) {
        remote_units.push_back(tm);
    }
}
	return remote_units;
}

// 4. 测试
int main()
{
	char path[256] = "/root/yangtg/nau/remote_test_gre.json";
	Unit test;
	std::vector<Unit> remotes;
    std::vector<UNIT> local;
    local = nau_json_tm_load_gre(path);
	remotes = find_subnet_fromRemoteMap(local);
	for(const auto& tm : remotes)
	{
		std::cout << tm.UTIME << std::endl;
	}
    //std::cout << test.UTIME << std::endl;
	return 0;
}