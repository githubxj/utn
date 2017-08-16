#include <sqlite3.h>
#include "../common/const.h"
#include "../common/logger.h"


void updateDSN(unsigned long *dsn, char* full_dsn)
{
	sqlite3 *db;  
	char *zErrMsg = 0;
	int rc;
	char sql_str[256];
	char dsn_str[32];
	int i;
	
	rc = sqlite3_open(DYNAMIC_DB, &db);

	if( rc ){
		logger ( LOG_ERR,   "Can't open database: %s\n", sqlite3_errmsg(db));
		return;
	}

	for(i=1; i<=MAX_ONU_NUM; i++)
	{
		if(dsn[i] == 0)
			continue;

//		printf("ONU-%d, dsn=%d\n", i, dsn[i]);
		memcpy(dsn_str, full_dsn + i*MAX_ONUDSN_LEN, MAX_ONUDSN_LEN);
		dsn_str[MAX_ONUDSN_LEN] = 0;
//		printf("FullDSN:%s\n", dsn_str);
		sprintf(sql_str, "delete from DSN where onuId=%d; insert into DSN(onuId, dsn) values (%d, '%s');", i, i, dsn_str);
		rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);

		if(rc != SQLITE_OK) {
			logger (LOG_ERR,  "Database insert error: sql=%s, err=%s\n", sql_str, sqlite3_errmsg(db));
		}
	}
	sqlite3_close(db);
	
}

void deleteDSN(unsigned char node)
{
	sqlite3 *db;  
	char *zErrMsg = 0;
	int rc;
	char sql_str[256];

	
	rc = sqlite3_open(DYNAMIC_DB, &db);

	if( rc ){
		logger ( LOG_ERR,   "Can't open database: %s\n", sqlite3_errmsg(db));
		return;
	}

	sprintf(sql_str, "delete from DSN where onuId=%d;", node);
	rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);

	if(rc != SQLITE_OK) {
		logger (LOG_ERR,  "Database insert error: sql=%s, err=%s\n", sql_str, sqlite3_errmsg(db));
	}

	sqlite3_close(db);
	
}

