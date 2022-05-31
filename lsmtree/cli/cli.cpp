#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#include "lsmtree/engine.hpp"

using namespace lsmtree;
using namespace std;

class CLI {
public:
    ~CLI() {
        delete engine_;
        engine_ = nullptr;
    }

    void Serve() {
        cout << "welcome" << endl;
        string line;
        while (cout << "lsmtree cli> " && getline(cin, line))  {
            vector<string> args = Parse(line);
            if (args.empty()) {
                continue;
            }

            string cmd = args[0];
            if (cmd == ".open") {
                if (args.size() != 2) {
                    Error("format error, usage: .open <datadir>");
                    continue;
                }
                if (not filesystem::exists(args[1])) {
                    Error(args[1] + " not exists");
                    continue;
                }
                engine_ = new Engine(args[1], 4096);
            } else if (cmd == ".close") {
                delete engine_;
                engine_ = nullptr;
            } else if (cmd == ".set") {
                if (args.size() != 3) {
                    Error("format error, usage: .set <key> <value>");
                    continue;
                }
                if (not engine_) {
                    Error("should open datadir first");
                    continue;
                }
                engine_->Set(args[1], args[2]);
            } else if (cmd == ".get") {
                if (args.size() != 2) {
                    Error("format error, usage: .get <key>");
                    continue;
                }
                if (not engine_) {
                    Error("should open datadir first");
                    continue;
                }
                auto value = engine_->Get(args[1]);
                cout << value << endl;
            } else if (cmd == ".keys") {
                if (args.size() != 1) {
                    Error("format error, usage: .keys");
                    continue;
                }
                if (not engine_) {
                    Error("should open datadir first");
                    continue;
                }
                vector<string> keys = engine_->Keys();
                // cout << DumpKeys(keys) << endl;
                for (auto &key : keys) {
                    cout << "\"" << key << "\"" << endl;
                }
            } else {
                cout << "unsupported commands" << endl;
            }
        };

        cout << "bye!" << endl;
        return;
    }

private:
    vector<string> Parse(string line) {
        if (line.size() == 1) return {line};

        vector<string> ret;
        size_t pos = 0;
        while ((pos = line.find(" ")) != string::npos) {
            ret.push_back(line.substr(0, pos));
            line.erase(0, pos + 1);
        }

        if (not line.empty()) {
            ret.push_back(line);
        }

        return ret;
    }

    void Error(string msg) {
        cout << "error: " << msg << endl;
    }

    void DumpArgs(vector<string> args) {
        for (auto &arg : args) {
            cout << "\"" << arg << "\"" << endl;
        }
    }

    string DumpKeys(vector<string> keys) {
        if (keys.empty()) {
            return "[]";
        }

        stringstream ss;
        ss << "[";
        ss << Wrap(keys[0]);
        for (size_t i = 1; i < keys.size(); ++i) {
            cout << "," << Wrap(keys[i]);
        }
        ss << "]";
        return ss.str();
    }

    inline string Wrap(string s) {
        return "\"" + s + "\"";
    }

    Engine *engine_ = nullptr;
};

int main(int argc, char **argv) {
    CLI cli;
    cli.Serve();

    return 0;
}
