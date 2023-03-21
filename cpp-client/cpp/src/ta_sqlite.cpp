//
// Created by wwango on 2022/11/11.
//
#include "ta_sqlite.h"
#include "ta_cpp_helper.h"
#include <tuple>
#include <vector>
#include<algorithm>
#include<string>
#include<iostream>
#include <sys/timeb.h>

namespace thinkingdata {

    using namespace std;
    using std::tuple;

    TASqliteDataQueue::TASqliteDataQueue(std::string appid): m_appid(appid), m_allmessagecount(0) {

        isStop = false;
        int openDBStatus = sqlite3_open(dataBaseFilePath.c_str(), &ta_database);
        if (openDBStatus)
        {
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(ta_database));
        }
        else
        {
            fprintf(stdout, "Opened database successfully\n");
        }

        try
        {
            // Create Table
            std::string dataTable = "create table if not exists TDData (id INTEGER PRIMARY KEY AUTOINCREMENT, content TEXT, appid TEXT, creatAt INTEGER, uuid TEXT)";

            char* createDataTableErrMsg = nullptr;
            int createDataTable = sqlite3_exec(ta_database, dataTable.c_str(), NULL, NULL, &createDataTableErrMsg);
            if (createDataTable != SQLITE_OK)
            {
                fprintf(stderr, "SQL error: %s\n", createDataTableErrMsg);
                sqlite3_free(createDataTableErrMsg);
            }
            else
            {
                fprintf(stdout, "Table created successfully\n");

                // allmessageCount
                m_allmessagecount = sqliteCount(appid);
            }
        }
        catch (const std::exception&)
        {

        }
        
    }

    long TASqliteDataQueue::sqliteCount(string appid) {
        if (isStop) {
            return 0;
        }
        string query;
        long count = 0;
        if (appid.empty() == true) {
            query = "select count(*) from TDData";
        } else {
            query = "select count(*) from TDData where appid=? ";
        }

        try
        {
            sqlite3_stmt* stmt = NULL;
            int rc = sqlite3_prepare_v2(ta_database, query.c_str(), -1, &stmt, NULL);

            if (rc == SQLITE_OK) {
                if (appid.empty() == false) {
                    sqlite3_bind_text(stmt, 1, appid.c_str(), -1, SQLITE_TRANSIENT);
                }
                if (sqlite3_step(stmt) == SQLITE_ROW) {
                    count = sqlite3_column_int(stmt, 0);
                }
            }
            sqlite3_finalize(stmt);
        }
        catch (const std::exception&)
        {

        }
        return count;
    }

    static int64_t ta_index = 0;

    int TASqliteDataQueue::addObject(std::string event, std::string appid) {
        if (isStop) {
            return 0;
        }
        if (event.empty() == true){
            return sqliteCount(appid);
        }

        try
        {
            timeb t;
            ftime(&t);
            long epochInterval = t.time * 1000 + t.millitm;
            string uuid = ta_cpp_helper::getEventID();
            uuid += to_string(ta_index);
            ta_index++;
            replace(uuid.begin(), uuid.end(), '-', 'a');
            string query = "INSERT INTO TDData(content, appid, creatAt, uuid) values(?, ?, ?, ?)";
            sqlite3_stmt* insertStatement;
            int rc;
            rc = sqlite3_prepare_v2(ta_database, query.c_str(), -1, &insertStatement, NULL);
            if (rc == SQLITE_OK) {
                sqlite3_bind_text(insertStatement, 1, event.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(insertStatement, 2, appid.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(insertStatement, 3, epochInterval);
                sqlite3_bind_text(insertStatement, 4, uuid.c_str(), -1, SQLITE_TRANSIENT);

                rc = sqlite3_step(insertStatement);
                if (rc == SQLITE_DONE) {
                    m_allmessagecount = m_allmessagecount + 1;
                }
            }

            sqlite3_finalize(insertStatement);
        }
        catch (const std::exception&)
        {

        }

        return sqliteCount(appid);
    }

    vector<tuple<string, string>> TASqliteDataQueue::getFirstRecords(int recordSize, string appid){

        vector<tuple<string, string>> records;
        if (isStop) {
            return records;
        }
        try
        {
            if (m_allmessagecount == 0) return records;
            string query = "SELECT id,content,uuid FROM TDData where appid=? ORDER BY id ASC LIMIT ?";
            sqlite3_stmt* stmt = NULL;
            int rc = sqlite3_prepare_v2(ta_database, query.c_str(), -1, &stmt, NULL);
            if (rc == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, appid.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(stmt, 2, (int)recordSize);
                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    sqlite3_int64 index = sqlite3_column_int64(stmt, 0);
                    char* jsonChar = (char*)sqlite3_column_text(stmt, 1);
                    char* uuidChar = (char*)sqlite3_column_text(stmt, 2);
                    if (!jsonChar) {
                        continue;
                    }
                    records.push_back(make_tuple(string(uuidChar), string(jsonChar)));
                }
            }
            sqlite3_finalize(stmt);
        }
        catch (const std::exception&)
        {

        }
        return records;
    }

    bool TASqliteDataQueue::removeData(vector<std::string> uuids) {

        if (isStop) {
            return false;
        }
        if (uuids.empty() == true) return false;

        try
        {
            string strData;
            for (auto data : uuids) {
                strData += "\'";
                strData += data + "\',";
            }
            strData = strData.substr(0, strData.size() - 1);
            string query = "DELETE FROM TDData WHERE uuid IN (" + strData + ");";
            sqlite3_stmt* stmt;

            if (sqlite3_prepare_v2(ta_database, query.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
                string errorstring = string("Delete records Error: ") + string(sqlite3_errmsg(ta_database));
                fprintf(stdout, errorstring.c_str());
                sqlite3_finalize(stmt);
                return false;
            }
            bool success = true;
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                string errorstring = string("Delete records Error: ") + string(sqlite3_errmsg(ta_database));
                fprintf(stdout, errorstring.c_str());
                success = false;
            }
            sqlite3_finalize(stmt);
            m_allmessagecount = sqliteCount(m_appid);
        }
        catch (const std::exception&)
        {

        }
        return true;
    }

    long  TASqliteDataQueue::getAllmessageCount(string appid){
        return m_allmessagecount;
    }


    void  TASqliteDataQueue::unInit() {
        isStop = true;
        sqlite3_close(ta_database);
    }
};
