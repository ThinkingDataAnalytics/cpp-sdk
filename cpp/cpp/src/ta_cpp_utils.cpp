
#include "ta_cpp_utils.h"
#include <sstream>
#include <iostream>
#include <iomanip>

namespace thinkingdata {

vector<string> Split(const string &str, const string &pattern) {
  vector<string> res;
  string last;
  if (str == "") return res;
  last = str.substr(str.size() - 1, 1);
  string strs = str;
  if (last != pattern) {
    strs = str + pattern;
  }
  size_t pos = strs.find(pattern);

  while (pos != strs.npos) {
    string temp = strs.substr(0, pos);
    res.push_back(temp);
    strs = strs.substr(pos + 1, strs.size());
    pos = strs.find(pattern);
  }
  return res;
}

map<string, string> ParserQueryItems(const string &query) {
  map<string, string> result;
  if (query.length() < 1) {
    return result;
  }
  vector<string> query_arr = Split(query, "&");
  for (vector<string>::iterator iter = query_arr.begin();
       iter != query_arr.end(); ++iter) {
    vector<string> item_arr = Split(*iter, "=");
    if (item_arr.size() > 1) {
      string first = item_arr[0];
      string second = item_arr[1];
      result.insert(pair<string, string>(first, second));
    }
  }
  return result;
}

#define CHECK_LEN_END(POS, LEN) \
  if (POS >= LEN) {             \
    _url_errorno = 100;         \
    goto __PARSE_END;           \
  }
#define WALK_SP(POS, LEN, BUF) for (; POS < LEN && BUF[POS] == ' '; POS++)
#define WALK_UNTIL(POS, LEN, BUF, DELC) \
  for (; POS < LEN && BUF[POS] != DELC; POS++)
#define WALK_UNTIL2(POS, LEN, BUF, DELI1, DELI2) \
  for (; POS < LEN && BUF[POS] != DELI1 && BUF[POS] != DELI2; POS++)
#define WALK_UNTIL3(POS, LEN, BUF, DELI1, DELI2, DELI3)         \
  for (; POS < LEN && BUF[POS] != DELI1 && BUF[POS] != DELI2 && \
         BUF[POS] != DELI3;                                     \
       POS++)
#define CHECK_REMAIN_END(POS, LEN, REQ_LEN) \
  if (LEN - POS < REQ_LEN) {                \
    _url_errorno = 100;                     \
    goto __PARSE_END;                       \
  }
#define WALK_CHAR(POS, BUF, DELI) \
  if (BUF[POS++] != DELI) goto __PARSE_END
void UrlParser::parse() {
  int _url_errorno = 0;
  const char *str = mRawUrl.c_str();

  int pos, len, scheme_pos, host_pos, port_pos, path_pos, param_pos, tag_pos;
  pos = 0;
  len = (int)mRawUrl.size();
  WALK_SP(pos, len, str);  // remove preceding spaces.
  if (str[pos] == '/') {
    goto __PARSE_HOST;
  }

  // start protocol scheme
  scheme_pos = pos;
  WALK_UNTIL(pos, len, str, ':');
  CHECK_LEN_END(pos, len);
  scheme = mRawUrl.substr(scheme_pos, pos - scheme_pos);
  CHECK_REMAIN_END(pos, len, 3);
  WALK_CHAR(pos, str, ':');
  WALK_CHAR(pos, str, '/');

// start host address
__PARSE_HOST:
  WALK_CHAR(pos, str, '/');
  host_pos = pos;
  WALK_UNTIL3(pos, len, str, ':', '/', '?');
  if (pos < len) {
    hostName = mRawUrl.substr(host_pos, pos - host_pos);
    if (str[pos] == ':') goto __PARSE_PORT;
    if (str[pos] == '/') goto __PARSE_PATH;
    if (str[pos] == '?') goto __PARSE_PARAM;
  } else {
    hostName = mRawUrl.substr(host_pos, pos - host_pos);
  }

__PARSE_PORT:
  WALK_CHAR(pos, str, ':');
  port_pos = pos;
  WALK_UNTIL2(pos, len, str, '/', '?');
  port = mRawUrl.substr(port_pos, pos - port_pos);
  CHECK_LEN_END(pos, len);
  if (str[pos] == '?') goto __PARSE_PARAM;
__PARSE_PATH:
  path_pos = pos;
  WALK_UNTIL(pos, len, str, '?');
  path = mRawUrl.substr(path_pos, pos - path_pos);
  CHECK_LEN_END(pos, len);
__PARSE_PARAM:
  WALK_CHAR(pos, str, '?');
  param_pos = pos;
  WALK_UNTIL(pos, len, str, '#');
  query = mRawUrl.substr(param_pos, pos - param_pos);

  CHECK_LEN_END(pos, len);
  // start parsing fragment
  WALK_CHAR(pos, str, '#');
  tag_pos = pos;
  fragment = mRawUrl.substr(tag_pos, len - tag_pos);
__PARSE_END:
  return;
}

UrlParser *UrlParser::parseUrl(string urlstr) {
  UrlParser *url = new UrlParser;
  url->mRawUrl = urlstr;
  url->parse();
  url->queryItems = ParserQueryItems(url->query);

  return url;
}

string UrlWithoutQuery(UrlParser *parser) {
  string result = parser->scheme + "://" + parser->hostName;
  if (parser->port.length() > 0) {
    result = result + ":" + parser->port;
  }
  if (parser->path.length() > 0) {
    result = result + parser->path;
  }
  return result;
}

string Splice(const vector<string> &array, const string &pattern) {
  string result;
  vector<string> list(array);
  if (array.size() < 1) {
    return "";
  }
  for (vector<string>::const_iterator iter = list.begin();
       iter != list.end() - 1; ++iter) {
    result += *iter;
    result += pattern;
  }
  string end = list.back();
  result += end;
  return result;
}

}
