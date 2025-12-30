//////////////////////////////////////////////////////////////////////////
#include <sqlite3.h>
#include "nau_main.h"


//////////////////////////////////////////////////////////////////////////
static sqlite3		*nau_db = NULL;


//////////////////////////////////////////////////////////////////////////
static int count_sql(void *data, int argc, char **argv, char **azColName)
{
	unsigned int	*val =(unsigned int *)(data);

	val[0] = 0;
	if (argc > 0 && argv[0] != nullptr) {

		val[0] = std::stoi(argv[0]);
	}
	return 0;
}
static int query_sql(void *data, int argc, char **argv, char **azColName)
{
	PNAU_DB_TM	*head = (PNAU_DB_TM *)(data);
	PNAU_DB_TM	item;

	// 申请临时结点
	item = (PNAU_DB_TM)malloc(sizeof(NAU_DB_TM));
	if(NULL == item) {

		return 0;
	}
	// 保存记录
	strncpy(item->uidip, argv[0], sizeof(item->uidip));
	strncpy(item->uidmac,argv[1], sizeof(item->uidmac));
	strncpy(item->ifup4, argv[2], sizeof(item->ifup4));
	strncpy(item->ifup6, argv[3], sizeof(item->ifup6));
	strncpy(item->proto, argv[4], sizeof(item->proto));
	strncpy(item->nauid, argv[5], sizeof(item->nauid));

	// 加入结果链
	item->next = *head;
	*head = item;
    return 0;
}
static int query_sql_gre(void *data, int argc, char **argv, char **azColName)
{
	PNAU_DB_TM	*head = (PNAU_DB_TM *)(data);
	PNAU_DB_TM	item;

	// 申请临时结点
	item = (PNAU_DB_TM)malloc(sizeof(NAU_DB_TM));
	if(NULL == item) {

		return 0;
	}
	// 保存记录
	strncpy(item->uidip, argv[0], sizeof(item->uidip));
	strncpy(item->uidmac,argv[1], sizeof(item->uidmac));
	strncpy(item->ifup4, argv[2], sizeof(item->ifup4));
	strncpy(item->ifup6, argv[3], sizeof(item->ifup6));
	strncpy(item->proto, argv[4], sizeof(item->proto));
	strncpy(item->nauid, argv[5], sizeof(item->nauid));
	strncpy(item->ctime, argv[6], sizeof(item->ctime));
	strncpy(item->utime, argv[7], sizeof(item->utime));

	// 加入结果链
	item->next = *head;
	*head = item;
    return 0;
}
int execute_sql(sqlite3 *db, const std::string& sql)
{
	char		*err = NULL;
	int			rc;

	rc = sqlite3_exec(db, sql.c_str(), NULL, 0, &err);
	if(SQLITE_OK != rc) {

		std::cerr << "SQL Error: " << err << std::endl;
		sqlite3_free(err); // 释放错误消息内存
		return -1;
	}
	return 0;
}
std::string current_time(void)
{
	// 获取当前时间
	auto now = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	std::tm* local_tm = std::localtime(&now_c);
	if(local_tm == nullptr) {

		return "";
	}

	// 输出格式化时间
	std::stringstream ss;
	ss << std::put_time(local_tm, "%Y-%m-%d %H:%M:%S");
	return ss.str();
}


//////////////////////////////////////////////////////////////////////////
int
nau_db_open(
	const char		*fname)
{
	if( sqlite3_open(fname, &nau_db) ) {

		std::cerr << "Open DB Failed!" << sqlite3_errmsg(nau_db) << std::endl;
		return -1;
	}
	std::cout << "Open DB OK!" << std::endl;
	return 0;
}
void
nau_db_close(void)
{
	if(NULL != nau_db) {

		sqlite3_close(nau_db);
	}
	nau_db = NULL;
}

int
nau_db_init(void)
{
	std::string	tbl = NAU_DB_RTB;
	std::string flush = "DROP TABLE IF EXISTS " + tbl;
	std::string sql = "CREATE TABLE IF NOT EXISTS " + tbl + " ("
									"ID INTEGER PRIMARY KEY AUTOINCREMENT,"
									"UIDIP TEXT NOT NULL,"
									"UIDMAC TEXT NOT NULL,"
									"IFUP4 TEXT NOT NULL,"
									"IFUP6 TEXT NOT NULL,"
									"PROTO TEXT NOT NULL,"
									"NAUID TEXT NOT NULL,"
									"CTIME TEXT NOT NULL,"
									"UTIME TEXT NOT NULL);";
	execute_sql(nau_db,flush);//初始化时删除数据表
	execute_sql(nau_db, sql);

	tbl = NAU_DB_LTB;
	flush = "DROP TABLE IF EXISTS " + tbl;
	sql = "CREATE TABLE IF NOT EXISTS " + tbl + " ("
									"ID INTEGER PRIMARY KEY AUTOINCREMENT,"
									"UIDIP TEXT NOT NULL,"
									"UIDMAC TEXT NOT NULL,"
									"IFUP4 TEXT NOT NULL,"
									"IFUP6 TEXT NOT NULL,"
									"PROTO TEXT NOT NULL,"
									"NAUID TEXT NOT NULL,"
									"CTIME TEXT NOT NULL,"
									"UTIME TEXT NOT NULL);";
	execute_sql(nau_db,flush);//初始化时删除数据表
	return execute_sql(nau_db, sql);
}

int
nau_db_add(
	const char		*tb,
	PNAU_ITEM_TM	tm)
{
	std::string		strtb = tb;
	std::string		ctime = current_time();

	// 构造SQL语句
	std::string sql = "INSERT INTO " + strtb + " (UIDIP, UIDMAC, IFUP4, IFUP6, PROTO, NAUID, CTIME, UTIME) VALUES (\"";
	sql += tm->uidip + "\", \"";
	sql += tm->uidmac+ "\", \"";
	sql += tm->ifup4 + "\", \"";
	sql += tm->ifup6 + "\", \"";
	sql += tm->proto + "\", \"";
	sql += tm->nauid + "\", \"";
	sql += ctime + "\", \"" + ctime;
	sql += "\");";

	// 插入记录
	return execute_sql(nau_db, sql);
}
int
nau_db_upt(
	const char		*tb,
	PNAU_ITEM_TM	tm)
{
	std::string		strtb = tb;
	std::string		ctime = current_time();

	// 构造SQL语句
	std::string sql = "UPDATE " + strtb + " SET IFUP4=\"";
	sql += tm->ifup4 + "\", IFUP6=\"";
	sql += tm->ifup6 + "\", UTIME=\"";
	sql += ctime + "\" WHERE UIDIP=\"";
	sql += tm->uidip + "\" and NAUID=\"";
	sql += tm->nauid + "\";";

	// 修改数据库
	return execute_sql(nau_db, sql);
}

int
nau_db_clean(
	const char		*tbl)
{
	std::string		strtb = tbl;

	// 构造SQL语句
	std::string	sql = "DELETE FROM " + strtb + ";";

	// 查询记录数量
	return execute_sql(nau_db, sql);
}
int
nau_db_expire(
	const char		*tbl,
	unsigned int	sec)
{
	std::string		strtb = tbl;
	std::string		sql;

	sql = "DELETE FROM " + strtb + " WHERE UTIME < datetime('now', '-" + std::to_string(sec) + " seconds');";

	return execute_sql(nau_db, sql);
}
int
nau_db_count(
	const char		*tbl,
	std::string		uidip,
	std::string		nauid)
{
	std::string		strtb = tbl;
	unsigned int	count= 0;
	char			*err = NULL;
	int				rc;

	// 构造SQL语句
	std::string	sql = "SELECT COUNT(*) FROM " + strtb + " WHERE UIDIP=\"";
    sql += uidip + "\" and NAUID=\"";
    sql += nauid + "\";";

	// 查询记录数量
	rc = sqlite3_exec(nau_db, sql.c_str(), count_sql, &count, &err);
	if(SQLITE_OK != rc) {

		std::cerr << "SQL Error: " << err << std::endl;
		sqlite3_free(err); // 释放错误消息内存
		return -1;
	}
	return count;
}
int
nau_db_check(
	const char		*tbl,
	std::string		uidip,
	std::string		nauid,
	std::string		ifup4,
	std::string		ifup6)
{
	std::string		strtb = tbl;
	unsigned int	count= 0;
	char			*err = NULL;
	int				rc;

	// 构造SQL语句
	std::string	sql = "SELECT COUNT(*) FROM " + strtb + " WHERE UIDIP=\"";
    sql += uidip + "\" and IFUP4=\"";
    sql += ifup4 + "\" and IFUP6=\"";
    sql += ifup6 + "\" and NAUID=\"";
    sql += nauid + "\";";

	// 查询记录数量
	rc = sqlite3_exec(nau_db, sql.c_str(), count_sql, &count, &err);
	if(SQLITE_OK != rc) {

		std::cerr << "SQL Error: " << err << std::endl;
		sqlite3_free(err); // 释放错误消息内存
		return -1;
	}
	return count;
}
std::vector<NAU_ITEM_TM>
nau_db_query(
	const char		*tbl)
{
	std::vector<NAU_ITEM_TM> items;
	std::string		strtb = tbl;
	std::string		sql;
	PNAU_DB_TM		head = NULL;
	PNAU_DB_TM		curr, next;
	char			*err = NULL;
	int				rc;

	// 查询数据
	sql = "SELECT UIDIP, UIDMAC, IFUP4, IFUP6, PROTO, NAUID FROM " + strtb + " ORDER BY NAUID DESC;";
	rc = sqlite3_exec(nau_db, sql.c_str(), query_sql, &head, &err);
	if(SQLITE_OK != rc) {

		std::cerr << "SQL Error: " << err << std::endl;
		sqlite3_free(err); // 释放错误消息内存
		return items;
	}

	// 获得向量
	curr = head;
	while(NULL != curr) {

		items.push_back({curr->uidip, curr->uidmac, curr->ifup4, curr->ifup6, curr->proto, curr->nauid});
		next = curr->next;
		free(curr);
		curr = next;
	}
	return items;
}

std::vector<UNIT>
nau_db_query_for_gre(
	const char		*tb)
{
	std::vector<UNIT> items;
	std::string		strtb = tb;
	std::string		sql;
	PNAU_DB_TM		head = NULL;
	PNAU_DB_TM		curr, next;
	char			*err = NULL;
	int				rc;

	// 查询数据
	sql = "SELECT UIDIP, UIDMAC, IFUP4, IFUP6, PROTO, NAUID, CTIME, UTIME FROM " + strtb + " ORDER BY NAUID DESC;";
	rc = sqlite3_exec(nau_db, sql.c_str(), query_sql_gre, &head, &err);
	if(SQLITE_OK != rc) {

		std::cerr << "SQL Error: " << err << std::endl;
		sqlite3_free(err); // 释放错误消息内存
		return items;
	}

	// 获得向量
	curr = head;
	while(NULL != curr) {

		items.push_back({curr->uidip, curr->uidmac, curr->ifup4, curr->ifup6, curr->proto, curr->nauid, curr->ctime, curr->utime});
		next = curr->next;
		free(curr);
		curr = next;
	}
	return items;
}

