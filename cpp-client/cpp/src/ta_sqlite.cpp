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
#include "ta_cpp_utils.h"

namespace thinkingdata {

    using namespace std;
    using std::tuple;
    char* G2U(const char* gb2312);

    TASqliteDataQueue::TASqliteDataQueue(std::string appid): m_appid(appid), m_allmessagecount(0) {

        isStop = false;
        int openDBStatus = sqlite3_open(dataBaseFilePath.c_str(), &ta_database);
        if (openDBStatus)
        {
            if (TAEnableLog::getEnableLog()) {
                cout << "\n[ThinkingEngine]  Can't open database: " << sqlite3_errmsg(ta_database) << endl;
            }
        }
        else
        {
            if (TAEnableLog::getEnableLog()) {
                cout << "\n[ThinkingEngine]  Opened database successfully " << endl;
            }
        }

        try
        {
            // Create Table
            std::string dataTable = "create table if not exists TDData (id INTEGER PRIMARY KEY AUTOINCREMENT, content TEXT, appid TEXT, creatAt INTEGER, uuid TEXT)";

            char* createDataTableErrMsg = nullptr;
            int createDataTable = sqlite3_exec(ta_database, dataTable.c_str(), NULL, NULL, &createDataTableErrMsg);
            if (createDataTable != SQLITE_OK)
            {
                if (TAEnableLog::getEnableLog()) {
                    cout << "\n[ThinkingEngine]  SQL error: " << createDataTableErrMsg << endl;
                }
                sqlite3_free(createDataTableErrMsg);
            }
            else
            {
                if (TAEnableLog::getEnableLog()) {
                    cout << "\n[ThinkingEngine]  Table created successfully" << endl;
                }

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

        sqlite3_stmt* stmt = NULL;
        try
        {
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
            if (stmt != NULL) {
                sqlite3_finalize(stmt);
            }
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

        sqlite3_stmt* insertStatement = NULL;
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
            int rc;
       
            rc = sqlite3_prepare_v2(ta_database, query.c_str(), -1, &insertStatement, NULL);
            if (rc == SQLITE_OK) {
#if defined(_WIN32) && defined(_MSC_VER)
                if (!CheckUtf8Valid(event.c_str())) {
                    char* str = G2U(event.c_str());
                    string tmpStr = string(str);
                    sqlite3_bind_text(insertStatement, 1, tmpStr.c_str(), -1, SQLITE_TRANSIENT);
                    delete str;
                }
                else {
                    sqlite3_bind_text(insertStatement, 1, event.c_str(), -1, SQLITE_TRANSIENT);
                }  
#else
                sqlite3_bind_text(insertStatement, 1, event.c_str(), -1, SQLITE_TRANSIENT);
#endif
                sqlite3_bind_text(insertStatement, 2, appid.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(insertStatement, 3, epochInterval);
                sqlite3_bind_text(insertStatement, 4, uuid.c_str(), -1, SQLITE_TRANSIENT);

                rc = sqlite3_step(insertStatement);
                if (rc == SQLITE_DONE) {
                    m_allmessagecount = m_allmessagecount + 1;
                }
            }
            sqlite3_finalize(insertStatement);
        } catch (const std::exception&) {
            if (insertStatement != NULL) {
                sqlite3_finalize(insertStatement);
            }
        }
        return sqliteCount(appid);
    }
   
    vector<tuple<string, string>> TASqliteDataQueue::getFirstRecords(int recordSize, string appid){

        vector<tuple<string, string>> records;
        if (isStop) {
            return records;
        }
        sqlite3_stmt* stmt = NULL;
        try
        {
            if (m_allmessagecount == 0) return records;
            string query = "SELECT id,content,uuid FROM TDData where appid=? ORDER BY id ASC LIMIT ?";
            
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
            if (stmt != NULL) {
                sqlite3_finalize(stmt);
            }
        }
    
        return records;
    }

    bool TASqliteDataQueue::removeData(vector<std::string> uuids) {

        if (isStop) {
            return false;
        }
        if (uuids.empty() == true) return false;

        sqlite3_stmt* stmt = NULL;
        try
        {
            string strData;
            for (auto data : uuids) {
                strData += "\'";
                strData += data + "\',";
            }
            strData = strData.substr(0, strData.size() - 1);
            string query = "DELETE FROM TDData WHERE uuid IN (" + strData + ");";
            

            if (sqlite3_prepare_v2(ta_database, query.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
                string errorString = string("Delete records Error: ") + string(sqlite3_errmsg(ta_database));
                fprintf(stdout, errorString.c_str());
                sqlite3_finalize(stmt);
                return false;
            }
            bool success = true;
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                string errorString = string("Delete records Error: ") + string(sqlite3_errmsg(ta_database));
                fprintf(stdout, errorString.c_str());
                success = false;
            }
            sqlite3_finalize(stmt);
            m_allmessagecount = sqliteCount(m_appid);
        }
        catch (const std::exception&)
        {
            if (stmt != NULL) {
                sqlite3_finalize(stmt);
            }
        }
        return true;
    }

    long  TASqliteDataQueue::getAllMessageCount(string appid){
        return m_allmessagecount;
    }


    void  TASqliteDataQueue::unInit() {
        isStop = true;
        sqlite3_close(ta_database);
    }
};
