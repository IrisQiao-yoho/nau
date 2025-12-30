#include <sqlite3.h>
#include "nau_main.h"

int main()
{
    nau_db_open("/home/NAU_DB.ap");
	nau_db_init();
    std::vector<UNIT> tmsR = nau_db_query_for_gre(NAU_DB_RTB);
    std::vector<UNIT> tmsL = nau_db_query_for_gre(NAU_DB_LTB);
    nau_db_close();


    nau_json_tm_save_gre("/home/remote_test_gre.json", tmsR);
    nau_json_tm_save_gre("/home/local_test_gre.json", tmsL);
    return 0;
}
