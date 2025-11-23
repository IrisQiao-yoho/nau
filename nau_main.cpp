//////////////////////////////////////////////////////////////////////////
#include "nlohmann/json.hpp"
#include "nau_main.h"


//////////////////////////////////////////////////////////////////////////
using json = nlohmann::json;
namespace fs = std::filesystem;


//////////////////////////////////////////////////////////////////////////
static pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;
static std::time_t	last_time;
static int			gw_update;
static int			ap_arp_t = 10; // ARP -A扫描周期10秒
static unsigned int	gw_min_t = 3;  // 网关最短下发时间
static unsigned int	gw_max_t = 10; // 网关最长下发时间
static unsigned int	tm_sec_t = 60; // 记录超时时间


//////////////////////////////////////////////////////////////////////////
void *nau_http_ipv4(void *arg)
{
	int	type = *((int*)(arg));
	int	port = 80;

	// 设置端口号
	if(NAU_TYPE_AP ==  type) {
		port = 8080;
	}
	// 启动服务器
	nau_http_svr("0.0.0.0", port, "/nau", "/tmp/", type);
	return NULL;
}
void *nau_http_ipv6(void *arg)
{
	int	type = *((int*)(arg));
	int	port = 80;

	// 设置端口号
	if(NAU_TYPE_AP ==  type) {
		port = 8080;
	}
	// 启动服务器
	nau_http_svr("::", port, "/nau", "/tmp/", type);
	return NULL;
}

int
nau_main_http(
	int			type)
{
	pthread_t	thrid;

	// 开启HTTP[IPv4]服务线程
	if(pthread_create(&thrid, NULL, nau_http_ipv4, &type) != 0) {

		perror("Error creating thread HTTP[IPv4]");
		return -1;
	}
	usleep(100 * 1000);

	// 开启HTTP[IPv6]服务线程
	if(pthread_create(&thrid, NULL, nau_http_ipv6, &type) != 0) {

		perror("Error creating thread HTTP[IPv6]");
		return -2;
	}
	usleep(100 * 1000);
	return 0;
}
void
nau_main_ap(void)
{
	fs::path	fpath= "/tmp/ap";
	pthread_t	thrid;
	int			rc;

	// Step0: 创建HTTP文件夹
	if( !fs::exists(fpath) )  {

		fs::create_directories(fpath);
	}
	// Step1: 开启HTTP服务
	if(0 != nau_main_http(NAU_TYPE_AP)) {

		return ;
	}

	// Step2: 打开数据库
	nau_db_open("/home/NAU_DB.ap");
	nau_db_init();

	// Step3: 读取系统配置
	rc = nau_json_cfg_load("/home/NAU_config.json");
	if(0 != rc) {

		nau_db_close();
		return ;
	}
	nau_json_cfg_show();

	// Step4: 开启ARP扫描线程
	if(pthread_create(&thrid, NULL, nau_ap_looparp, NULL) != 0) {

		perror("Error creating thread ARP");
		return;
	}
	usleep(100 * 1000);

	// Step5: 启动网卡监控
	nau_iface_main(nau_ifu_ifn.c_str());
	nau_db_close();
}
void
nau_main_gw(void)
{
	fs::path		fpath= "/tmp/gw";
	std::time_t		cur_time;

	// Step0: 创建HTTP文件夹
	if( !fs::exists(fpath) )  {

		fs::create_directories(fpath);
	}
	// Step1: 开启HTTP服务
	if(0 != nau_main_http(NAU_TYPE_GW)) {

		return ;
	}
	gw_update= 0;

	// Step2: 打开数据库
	nau_db_open("/home/NAU_DB.gw");
	nau_db_init();

	// Step3: 下发所有数据
	nau_gw_update();
	while( true ) {

		cur_time = std::time(nullptr);
		if((1 == gw_update) && ((last_time + gw_min_t) <= cur_time)) {

			nau_gw_update();
		}else if((last_time + gw_max_t) <= cur_time){

			nau_gw_update();
		}
		sleep(1);
	}
	nau_db_close();
}

void *
nau_ap_looparp(
	void			*arg)
{
	while( true ) {

		nau_ap_update();
		sleep(ap_arp_t);
	}
	return NULL;
}
void
nau_ap_update(void)
{
	std::vector<NAU_ARP_TM>	tms;
	NAU_ITEM_AP				ap;
	std::string		fname;
	std::string		ifuip;
	std::string		ifup4;
	std::string		ifup6;
	int				family;

	// Step1: 生成NAU自身数据
	ap.nauid = nau_ap_id;

	// Step2: 读取ARP数据
	if("4" == nau_proto) {

		family = AF_INET;
		tms = nau_arp_ipv4(nau_ifd_ifn);
	}else {

		family = AF_INET6;
		tms = nau_arp_ipv6(nau_ifd_ifn);
	}

	// Step3: 获得上行地址 & 计算IP地址
	nau_iface_addr(ifuip, family, nau_ifu_ifn.c_str());
	if("4" == nau_proto) {
		ifup4 = ifuip;
		ifup6 = nau_ifu_v6 + ifuip;
	}else {
		ifup6 = ifuip;
		ifup4 = nau_iface_6to4(nau_ifu_v6, ifuip);
	}

	// Step4: 生成本地NAU表
	ap.tm.push_back({nau_ifd_ip, "", ifup4, ifup6, nau_proto, nau_ap_id});
	for(const auto& tm : tms) {

		ap.tm.push_back({tm.addr, tm.mac, ifup4, ifup6, nau_proto, nau_ap_id});
	}
	pthread_mutex_lock(&db_mutex);

	// Step5: 更新本地表
	nau_tm_update(NAU_DB_LTB, "", ap.tm);

	// Step6: 老化本地表
	nau_tm_expire(NAU_DB_LTB, tm_sec_t);

	// Step7: 导出本地表
	fname = "/tmp/ap/ap-" + nau_ap_id + ".json";
	nau_tm_export(NAU_DB_LTB, fname.c_str());

	pthread_mutex_unlock(&db_mutex);

	// Step8: 上传本地表数据到网关
	nau_http_post(nau_mng_ip.c_str(), 80, "/nau", fname.c_str());
}
void
nau_ap_upload(
	const char		*fname)
{
	std::vector<NAU_ITEM_TM> tms;

	// Step1: 解析下发数据
	tms = nau_json_tm_load(fname);

	pthread_mutex_lock(&db_mutex);
	// Step2: 更新远端表
	nau_tm_update(NAU_DB_RTB, nau_ap_id, tms);

	// Step3: 老化远端表
	nau_tm_expire(NAU_DB_RTB, tm_sec_t);
	pthread_mutex_unlock(&db_mutex);
	std::cerr << "[AP]: update tm-db from file:" << fname << std::endl;
}

void
nau_gw_update(void)
{
	std::vector<NAU_ITEM_TM> tms;
	std::vector<std::string> aps;
	std::string			fname;
	std::string			iface;
	std::string			target;

	// Step1: 获得所有记录
	pthread_mutex_lock(&db_mutex);
	tms = nau_db_query(NAU_DB_LTB);
	pthread_mutex_unlock(&db_mutex);
	if(tms.size() <= 0) {

		return ;
	}
	// Step2: 生成下发文件
	fname = "/tmp/gw/gw-0.json";
	nau_json_tm_save(fname.c_str(), tms);

	// Step3: 下发数据到AP
	for(const auto& tm : tms) {

		if("" == tm.ifup4 || "" == tm.ifup6) {
			continue;
		}
		if(std::find(aps.begin(), aps.end(), tm.nauid) != aps.end()) {
			continue;
		}
		aps.push_back(tm.nauid);

		// 获得AP的HTTP地址
		if("4" == tm.proto) {
			iface = tm.ifup4;
		}else {
			iface = tm.ifup6;
		}
		target = iface;
		size_t pos = target.find('/');
		if(pos != std::string::npos) {

			target = iface.substr(0, pos);
		}
		// 下发文件
		nau_http_post(target.c_str(), 8080, "/nau", fname.c_str());
	}
	gw_update = 0;

	// Step4: 记录更新时间
	last_time = time(NULL);
}
void
nau_gw_upload(
	const char		*fname)
{
	std::vector<NAU_ITEM_TM> tms;

	// Step1: 解析上传数据
	tms = nau_json_tm_load(fname);

	pthread_mutex_lock(&db_mutex);
	// Step2: 更新网关数据库
	nau_tm_update(NAU_DB_LTB, "", tms);

	// Step3: 老化网关数据库
	nau_tm_expire(NAU_DB_LTB, tm_sec_t);
	pthread_mutex_unlock(&db_mutex);

	// Step3: 设置更新状态
	gw_update = 1;
	std::cerr << "[GW]: update tm-db from file:" << fname << std::endl;
}

