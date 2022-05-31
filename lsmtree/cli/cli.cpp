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

    void Open(string dir) {
        if (not filesystem::exists(dir)) {
            throw runtime_error("error: " + dir + " not exists");
        }
        engine_ = new Engine(dir, 4096);
    }

    void Close() {
        delete engine_;
        engine_ = nullptr;
    }

    void Set(vector<string> args) {
        if (not engine_) {
            throw runtime_error("error: should open datadir first");
        }
        engine_->Set(args[0], args[1]);
    }

    void Get(string key) {
        if (not engine_) {
            throw runtime_error("error: should open datadir first");
        }
        cout << engine_->Get(key) << endl;
    }

    void Keys() {
        if (not engine_) {
            throw runtime_error("error: should open datadir first");
        }
        vector<string> keys = engine_->Keys();
        cout << DumpKeys(keys) << endl;
    }

    void Serve() {
        cout << "welcome!" << endl;
        string line;
        while (cout << "lsmtree cli> " && getline(cin, line))  {
            vector<string> args = Parse(line);
            if (args.empty()) {
                continue;
            }

            string cmd = args[0];
            try {
                if (cmd == ".open") {
                    CheckNumArguments(args, 2,
                            "error: format error, usage: .open <datadir>");
                    Open(args[1]);
                } else if (cmd == ".close") {
                    CheckNumArguments(args, 1,
                            "error: format error, usage: .close");
                    Close();
                } else if (cmd == ".set") {
                    CheckNumArguments(args, 3,
                            "error: format error, usage: .set <key> <value>");
                    Set({args[1], args[2]});
                } else if (cmd == ".get") {
                    CheckNumArguments(args, 2,
                            "error: format error, usage: .get <key>");
                    Get(args[1]);
                } else if (cmd == ".keys") {
                    CheckNumArguments(args, 1,
                            "error: format error, usage: .keys");
                    Keys();
                } else {
                    cout << "unsupported commands" << endl;
                }
            } catch (runtime_error &e) {
                cout << e.what() << endl;
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

    void CheckNumArguments(vector<string> &args, 
            size_t numArgs, 
            string errmsg) {
        if (args.size() != numArgs) {
            throw runtime_error(errmsg);
        }
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
            ss << "," << Wrap(keys[i]);
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
