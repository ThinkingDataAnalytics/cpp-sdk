
#ifndef ta_cpp_utils_h
#define ta_cpp_utils_h

#include <stdio.h>

#include <iostream>
#include <map>
#include <vector>

namespace thinkingdata {
using namespace std;

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

}

#endif
