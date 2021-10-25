#include <iostream>
#include <regex>

using namespace std;

string getParaType(string para) {
    regex number("\\d+");
    regex str("\'[A-Za-z0-9 ]*\'");

    string res = "";
    string sub = "";
    for (int i = 0; i < para.length(); i++) {
        // cout << sub << endl;
        if (para[i] != ',') {
            sub += para[i];
        }
        if (para[i] == ',' || i == para.length() - 1) {
            if (regex_match(sub, number))
                res += "number,";
            else if (regex_match(sub, str))
                res += "string,";
            else
                return "error";

            sub = "";
        } 
    }
    if (res != " ") return res.substr(0, res.size() - 1);
    return "";
}

int main() {
    string a = "1,'abc',3,4";
    cout << getParaType(a);
}