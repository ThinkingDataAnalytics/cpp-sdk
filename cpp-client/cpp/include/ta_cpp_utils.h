
#ifndef ta_cpp_utils_h
#define ta_cpp_utils_h

#include <stdio.h>

#include <iostream>
#include <map>
#include <vector>

namespace thinkingdata {
using namespace std;

class TAEnableLog {
public:
    static bool getEnableLog();
    static void setEnableLog(bool enableLog);
};



class UrlParser {
 public:
  static UrlParser *parseUrl(string urlstr);
  string scheme;
  string hostName;
  string port;
  string path;
  string query;
  map<string, string> queryItems;
  string fragment;

 private:
  void parse();
  string mRawUrl;
};

string UrlWithoutQuery(UrlParser *parser);

vector<string> Split(const string &str, const string &pattern);
string Splice(const vector<string> &array, const string &pattern);
bool CheckUtf8Valid(const string& str);
#if defined(_WIN32) && defined(_MSC_VER)
char* G2U(const char* gb2312);
char* U2G(const char* utf8);
#endif

}

#endif
