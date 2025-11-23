#ifndef __NAU_DB_H__
#define	__NAU_DB_H__


//////////////////////////////////////////////////////////////////////////
#define	NAU_DB_RTB		"remote_map"
#define	NAU_DB_LTB		"local_map"


//////////////////////////////////////////////////////////////////////////
typedef struct _NAU_DB_TM {

	char		uidip[64];
	char		uidmac[24];
	char		ifup4[64];
	char		ifup6[64];
	char		proto[4];
	char		nauid[8];

	struct _NAU_DB_TM *next;
}NAU_DB_TM, *PNAU_DB_TM;


//////////////////////////////////////////////////////////////////////////
int
nau_db_open(
	const char		*fname);
void
nau_db_close(void);

int
nau_db_init(void);
int
nau_db_add(
	const char		*tb,
	PNAU_ITEM_TM	tm);
int
nau_db_upt(
	const char		*tb,
	PNAU_ITEM_TM	tm);

int
nau_db_clean(
	const char		*tbl);
int
nau_db_expire(
	const char		*tbl,
	unsigned int	sec);
int
nau_db_count(
	const char		*tbl,
	std::string		uidip,
	std::string		nauid);
int
nau_db_check(
	const char		*tbl,
	std::string		uidip,
	std::string		nauid,
	std::string		ifup4,
	std::string		ifup6);
std::vector<NAU_ITEM_TM>
nau_db_query(
	const char		*tbl);


#endif

