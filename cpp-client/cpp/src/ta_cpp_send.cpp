//
// Created by wwango on 2022/11/14.
//

#include "ta_cpp_send.h"
#include <string.h>
#include <curl/curl.h>
#include <zlib.h>
#include "ta_json_object.h"
#include "ta_cpp_utils.h"
#include "ta_cpp_helper.h"


namespace thinkingdata {

    using namespace std;

    HttpSender::HttpSender(const string &server_url,
                           const string &appid)
            : server_url_(server_url), appid_(appid) {

        if(!server_url_.empty()) {
            if (server_url_[strlen(server_url_.c_str()) - 1] == '/') {
                configUrl = string(server_url_ + "config?appid="+appid_);
                server_url_ = string(server_url_ + "sync");
            } else {
                configUrl = string(server_url_ + "/config?appid="+appid_);
                server_url_ = string(server_url_ + "/sync");
            }
        }
    }

    bool HttpSender::send(const string &data) {
        string request_body;
        if (!encodeToRequestBody(data, &request_body)) {
            return false;
        }

        Response response = Post(server_url_,
                                 request_body,
                                 kRequestTimeoutSecond);
        if (response.code_ != 200) {
            ta_cpp_helper::printSDKLog("ThinkingAnalytics SDK send failed: ");
            ta_cpp_helper::printSDKLog(response.body_);
            ta_cpp_helper::handleTECallback(1003,response.body_);
            return false;
        }else{
            ta_cpp_helper::printSDKLog("[ThinkingData] Debug: Send event, Request = "+data);
            ta_cpp_helper::printSDKLog("[ThinkingData] Debug: Send event, Response = "+response.body_);
        }
        return true;
    }

    Response HttpSender::fetchRemoteConfig() {
        Response  response = Get(configUrl,kRequestTimeoutSecond);
        if (response.code_ != 200) {
            ta_cpp_helper::printSDKLog("ThinkingAnalytics SDK fetch remote config: ");
            ta_cpp_helper::printSDKLog(response.body_);
            ta_cpp_helper::handleTECallback(1003,response.body_);
        }
        return response;
    }

    bool HttpSender::gzipString(const string &str,
                                string *out_string,
                                int compression_level = Z_BEST_COMPRESSION) {
        z_stream zs;
        memset(&zs, 0, sizeof(zs));

        if (deflateInit2(&zs, compression_level, Z_DEFLATED,
                         15 | 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
            ta_cpp_helper::printSDKLog("deflateInit2 failed while compressing.");
            return false;
        }

        zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(str.data()));
        zs.avail_in = static_cast<uInt>(str.size());

        int ret;
        char out_buffer[32768];

        do {
            zs.next_out = reinterpret_cast<Bytef *>(out_buffer);
            zs.avail_out = sizeof(out_buffer);

            ret = deflate(&zs, Z_FINISH);

            if (out_string->size() < zs.total_out) {
                out_string->append(out_buffer, zs.total_out - out_string->size());
            }
        } while (ret == Z_OK);

        deflateEnd(&zs);

        if (ret != Z_STREAM_END) {
            std::cerr << "Exception during zlib compression: (" << ret << ") " << zs.msg
                      << std::endl;
            return false;
        }

        return true;
    }

    static const char kBase64Chars[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string HttpSender::base64Encode(const string &data) {
        const unsigned char
                *bytes_to_encode = reinterpret_cast<const unsigned char *>(data.data());
        size_t in_len = data.length();
        std::string ret;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];

        while (in_len-- > 0) {
            char_array_3[i++] = *(bytes_to_encode++);
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] =
                        ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] =
                        ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (i = 0; (i < 4); i++)
                    ret += kBase64Chars[char_array_4[i]];
                i = 0;
            }
        }

        if (i != 0) {
            for (j = i; j < 3; j++)
                char_array_3[j] = '\0';
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] =
                    ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] =
                    ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            for (j = 0; (j < i + 1); j++)
                ret += kBase64Chars[char_array_4[j]];
            while ((i++ < 3))
                ret += '=';
        }
        return ret;
    }

    bool HttpSender::encodeToRequestBody(const string & data, string *request_body) {

        string compressed_data;
        if (!gzipString(data, &compressed_data)) {
            return false;
        }
        const string base64_encoded_data = base64Encode(compressed_data);
        *request_body = base64_encoded_data;
        return true;
    }

    TAHttpSend::TAHttpSend(const string &server_url,
                           const string &appid){
        if (!server_url.empty() && !appid.empty()) {
            sender_ = new (std::nothrow) HttpSender(server_url, appid);
        }
    }

//    TAHttpSend::TAHttpSend(const string &server_url,
//                                     const string &appid)
//            : sender_(new HttpSender(server_url, appid)) {}

    bool TAHttpSend::Send(const TDJSONObject &record) {
        const string json_record = TDJSONObject::ToJson(record);
        std::stringstream buffer;


        std::vector <std::pair<string, string>> http_headers;

        if(!sender_){
            return false;
        }

        bool send_result = sender_->send(json_record);

//        if (send_result) {
//            ta_cpp_helper::printSDKLog("[ThinkingData] Debug: Send event, Request = "+json_record);
//        }

        return send_result;
    }

    Response TAHttpSend::fetchRemoteConfig() {
        if(sender_){
            return sender_->fetchRemoteConfig();
        }
        Response  res;
        res.code_ = -1;
        res.body_ = "sender_ init fail";
        return res;
    }

    TAHttpSend::~TAHttpSend() {
        if (sender_)
            delete sender_;
    }

    void TAHttpSend::Init() {
        TD_MUTEX_INIT(&records_mutex_);
        TD_MUTEX_INIT(&sending_mutex_);
    }
};