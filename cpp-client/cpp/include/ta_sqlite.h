//
// Created by wwango on 2022/11/6.
//

#ifndef UNTITLED1_TA_SQLITE_H
#define UNTITLED1_TA_SQLITE_H

#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <vector>
#include <deque>
#include <tuple>
#include <utility>
#include "sqlite3.h"
#include "ta_encrypt.h"
#include "ta_analytics_sdk.h"

namespace thinkingdata {
    using namespace std;

    class TASqliteDataQueue  {
    public:
        bool isStop;
        sqlite3* ta_database;
        TDRSAEncrypt* encrypt;
        TASqliteDataQueue(string appid,bool &initStatus,bool enableCrypt,int v,string &pKey,string &dbPath);
        int addObject(string event, string appid);
        void getFirstRecords(int recordSize, string appid,vector<tuple<string, string>>& records);
        bool removeData(vector<string> uuids);
        void removeExpressionData();
        long getAllMessageCount(string appid);
        void updateSecretKey(int version, const string &publicKey);
        void unInit();
    private:
        long m_allmessagecount;
        const string dataBaseFilePath = "TDData-data.db";

    private:
        string m_appid;
        long sqliteCount(string appid);
    };
};

#endif //UNTITLED1_TA_SQLITE_H
