#include <string>
#include <vector>
#include "lsmtree/engine.hpp"

using namespace std;
using namespace lsmtree;

class CLI {
public:
    ~CLI();
    void Serve();
private:
    void Open(string dir);
    void Close();
    void Set(vector<string> args);
    void Get(string key);
    void Keys();
    vector<string> Parse(string line);
    void CheckNumArguments(vector<string> &args, size_t n, string errmsg);
    void DumpArgs(vector<string> args);
    string DumpKeys(vector<string> keys);
    inline string Wrap(string s) {
        return "\"" + s + "\"";
    }

    Engine *engine_ = nullptr;
};
