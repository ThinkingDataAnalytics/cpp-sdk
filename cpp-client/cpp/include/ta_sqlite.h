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

namespace thinkingdata {
    using namespace std;

    class TASqliteDataQueue  {
    public:
        bool isStop;
        sqlite3* ta_database;
        TASqliteDataQueue(string appid);
        int addObject(string event, string appid);
        vector<tuple<string, string>> getFirstRecords(int recordSize, string appid);
        bool removeData(vector<string> uuids);
        long getAllMessageCount(string appid);
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
