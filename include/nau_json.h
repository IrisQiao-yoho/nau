#ifndef __NAU_JSON_H__
#define	__NAU_JSON_H__


//////////////////////////////////////////////////////////////////////////
extern std::string		nau_proto;
extern std::string		nau_ifd_ifn;
extern std::string		nau_ifd_ip;
extern std::string		nau_ifu_ifn;
extern std::string		nau_ifu_ip;
extern std::string		nau_ifu_v6;
extern std::string		nau_ap_id;
extern std::string		nau_mng_ip;


//////////////////////////////////////////////////////////////////////////
int
nau_json_cfg_load(
	const char		*fname);
void
nau_json_cfg_show(void);

int
nau_json_tm_save(
	const char		*fname,
	std::vector<NAU_ITEM_TM>& items);
std::vector<NAU_ITEM_TM>
nau_json_tm_load(
	const char		*fname);

int
nau_json_tm_save_gre(
	const char		*fname,
	std::vector<UNIT>& items);

std::vector<UNIT>
nau_json_tm_load_gre(
	const char		*fname);

#endif

