#ifndef __NAU_ITEM_H__
#define	__NAU_ITEM_H__


//////////////////////////////////////////////////////////////////////////
typedef struct _NAU_ITEM_TM {

	std::string		uidip;
	std::string		uidmac;
	std::string		ifup4;
	std::string		ifup6;
	std::string		proto;
	std::string		nauid;
}NAU_ITEM_TM, *PNAU_ITEM_TM;

typedef struct _NAU_ITEM_AP {

	std::string		nauid;
	std::vector<NAU_ITEM_TM> tm;
}NAU_ITEM_AP, *PNAU_ITEM_AP;

typedef struct _UNIT_DB_TM {
	std::string UIDIP;                                        // 对端用户IP地址
    std::string UIDMAC;                                       // 对端用户MAC地址
    std::string IFUP4;                                        // 对端接入单元上行口IPv4地址
    std::string IFUP6;                                        // 对端接入单元上行口IPv6地址
    std::string sat_protocol;                                 // 对端设备协议版本
    std::string NAU_ID;                                       // 对端设备ID
    std::string CTIME;                                        // 创建时间
	std::string UTIME;										  // 更新时间
}UNIT,  Unit, *PNAU_UNIT;

//////////////////////////////////////////////////////////////////////////
void
nau_tm_clean(
	const char		*tbl);
void
nau_tm_expire(
	const char		*tbl,
	unsigned int	sec);

void
nau_tm_update(
	const char		*tbl,
	std::string		nauid,
	std::vector<NAU_ITEM_TM>& tms);
void
nau_tm_export(
	const char		*tbl,
	const char		*fname);


#endif

