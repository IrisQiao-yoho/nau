//////////////////////////////////////////////////////////////////////////
#include "nau_main.h"


//////////////////////////////////////////////////////////////////////////
void
nau_tm_clean(
	const char		*tbl)
{
	nau_db_clean(tbl);
}
void
nau_tm_expire(
	const char		*tbl,
	unsigned int	sec)
{
	nau_db_expire(tbl, sec);
}

void
nau_tm_update(
	const char		*tbl,
	std::string		nauid,
	std::vector<NAU_ITEM_TM>& tms)
{
	// 遍历终端
	for(const auto& tm : tms) {

		// 过滤其它接入点【网关有效】
		if("" != nauid && nauid == tm.nauid) {

			continue;
		}
		// 数据是否已变化
		if(nau_db_check(tbl, tm.uidip, tm.nauid, tm.ifup4, tm.ifup6) > 0) {

			continue;
		}

		// 终端是否已存在
		if(nau_db_count(tbl, tm.uidip, tm.nauid) > 0) {

			nau_db_upt(tbl, (PNAU_ITEM_TM)(&tm));
		}else {

			nau_db_add(tbl, (PNAU_ITEM_TM)(&tm));
		}
	}
}
void
nau_tm_export(
	const char		*tbl,
	const char		*fname)
{
	std::vector<NAU_ITEM_TM>	tms;

	tms = nau_db_query(tbl);
	nau_json_tm_save(fname, tms);
}

