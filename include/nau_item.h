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

