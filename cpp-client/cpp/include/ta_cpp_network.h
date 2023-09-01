
#ifndef ta_cpp_network_h
#define ta_cpp_network_h

#include <iostream>
#include <map>
#include <vector>

namespace thinkingdata {
using namespace std;

typedef map<string, string> HeaderFields;
typedef pair<string, string> HeaderFieldItem;

typedef struct {
  int code_;
  string body_;
  HeaderFields headers_;
} Response;

Response Post(const string &url, const string &data, int timeout_second);
}

#endif
