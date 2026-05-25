#pragma once
#include <string>
#include <map>
using namespace std;

class JsonParser {
public:
    static string serialize(const map<string,string>& data) {
        string json = "{";
        int i = 0;
        for (const auto& p : data) {
            json += "\"" + p.first + "\":\"" + p.second + "\"";
            if (++i < (int)data.size()) json += ",";
        }
        return json + "}";
    }

    static map<string,string> deserialize(const string& json) {
        map<string,string> result;
        string clean = json;
        if (!clean.empty() && clean.front()=='{') clean = clean.substr(1);
        if (!clean.empty() && clean.back() =='}') clean.pop_back();

        size_t pos = 0;
        while (pos < clean.size()) {
            size_t k1 = clean.find('"', pos);   if (k1==string::npos) break;
            size_t k2 = clean.find('"', k1+1);  if (k2==string::npos) break;
            size_t v1 = clean.find('"', k2+2);  if (v1==string::npos) break;
            size_t v2 = clean.find('"', v1+1);  if (v2==string::npos) break;
            result[clean.substr(k1+1, k2-k1-1)] = clean.substr(v1+1, v2-v1-1);
            pos = v2+1;
            if (pos < clean.size() && clean[pos]==',') pos++;
        }
        return result;
    }

    static string makeQuestion(int id, const string& type, const string& text,
                                const string& choices, const string& pts) {
        return serialize({{"type","question"}, {"id",to_string(id)},
                          {"qtype",type}, {"text",text},
                          {"choices",choices}, {"points",pts}});
    }

    static string makeAnswer(const string& name, const string& qid,
                              const string& answer) {
        return serialize({{"type","answer"}, {"name",name},
                          {"qid",qid}, {"answer",answer}});
    }

    static string makeResult(const string& correct, const string& pts,
                              const string& totalScore) {
        return serialize({{"type","result"}, {"correct",correct},
                          {"points",pts}, {"totalScore",totalScore}});
    }

    static string makeLeaderboard(const string& content) {
        return serialize({{"type","leaderboard"}, {"content",content}});
    }

    static string makeNotif(const string& msg) {
        return serialize({{"type","notif"}, {"message",msg}});
    }

    static string makeLogin(const string& name) {
        return serialize({{"type","login"}, {"name",name}});
    }

    static string makeRequestQuestion(const string& name) {
        return serialize({{"type","getQuestion"}, {"name",name}});
    }

    static string makeDone(const string& name) {
        return serialize({{"type","done"}, {"name",name}});
    }
};
