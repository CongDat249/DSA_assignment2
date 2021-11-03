#include "SymbolTable.h"

// Constructor
Symbol::Symbol(string name, int level, int type, Symbol *parent = nullptr) {
    this->name = name;
    this->level = level;
    this->type = type;
    this->parent = parent;
    this->left = this->right = nullptr;
}

SymbolTable::SymbolTable() {
    this->root = nullptr;
    this->cur_level = 0;
}

SymbolTable::~SymbolTable() { clear(this->root); }

// Helper function
int Symbol::compare(Symbol *x) {
    int n_diff = this->name.compare(x->name);
    int l_diff = this->level - x->level;

    if (l_diff > 0 || (l_diff == 0 && n_diff > 0))
        return 1;
    if (l_diff < 0 || (l_diff == 0 && n_diff < 0))
        return -1;

    return 0;
}

void SymbolTable::clear(Symbol *root) {
    if (root == nullptr)
        return;

    clear(root->left);
    clear(root->right);
    delete root;
}

int SymbolTable::getType(string type) {
    regex string("string");
    regex number("number");
    regex function(
        "\\(((number|string)(,number|,string)*)?\\)->(number|string)");

    if (regex_match(type, number))
        return 0;
    if (regex_match(type, string))
        return 1;
    if (regex_match(type, function))
        return 2;

    return -1;
}

string SymbolTable::preorder(Symbol *root) {
    if (root == nullptr)
        return "";

    return root->name + "//" + to_string(root->level) + " " +
           preorder(root->left) + preorder(root->right);
}

void SymbolTable::right_rotate(Symbol *x) {
    if (x == nullptr || x->left == nullptr)
        return;

    Symbol *y = x->left;
    if (x->parent == nullptr)
        this->root = y;
    else if (x->parent->left == x)
        x->parent->left = y;
    else
        x->parent->right = y;

    y->parent = x->parent;
    x->left = y->right;
    if (y->right)
        y->right->parent = x;
    y->right = x;
    x->parent = y;
}

void SymbolTable::left_rotate(Symbol *x) {
    if (x == nullptr || x->right == nullptr)
        return;

    Symbol *y = x->right;
    x->right = y->left;
    if (y->left)
        y->left->parent = x;

    if (x->parent == nullptr)
        this->root = y;
    else if (x->parent->left == x)
        x->parent->left = y;
    else
        x->parent->right = y;

    y->parent = x->parent;
    y->left = x;
    x->parent = y;
}

int SymbolTable::splay(Symbol *x) {
    if (x == nullptr || x->parent == nullptr)
        return 0;

    while (x->parent != nullptr) {
        Symbol *p = x->parent;
        if (p->parent == nullptr) {
            if (p->left == x)
                right_rotate(p);
            else if (p->right == x)
                left_rotate(p);

            return 1;
        }
        Symbol *g = p->parent;
        if (g->left == p && p->left == x) {
            right_rotate(g);
            right_rotate(p);
        } else if (g->right == p && p->right == x) {
            left_rotate(g);
            left_rotate(p);
        } else if (g->left == p && p->right == x) {
            left_rotate(p);
            right_rotate(g);
        } else {
            right_rotate(p);
            left_rotate(g);
        }
    }

    return 1;
}

bool SymbolTable::h_lookup(string name, int level) {
    Symbol x(name, level, 0);
    Symbol *walker = this->root;
    while (walker != nullptr) {
        int order = x.compare(walker);
        if (order == 0) {
            splay(walker);
            return true;
        } else if (order < 0) {
            if (walker->left == nullptr) {
                return false;
            }
            walker = walker->left;
        } else {
            if (walker->right == nullptr) {
                return false;
            }
            walker = walker->right;
        }
    }
    return false;
}

Symbol *SymbolTable::search_level(string name, int level, int &num_comp) {
    Symbol x(name, level, 0);
    Symbol *walker = this->root;
    while (walker != nullptr) {
        num_comp++;
        int order = x.compare(walker);
        if (order == 0) {
            return walker;
        } else if (order < 0) {
            if (walker->left == nullptr) {
                return walker;
            }
            walker = walker->left;
        } else {
            if (walker->right == nullptr) {
                return walker;
            }
            walker = walker->right;
        }
    }
    return nullptr;
}

Symbol *SymbolTable::getMaxValueNode(Symbol *root) {
    if (!root)
        return nullptr;
    Symbol *w = root;
    while (w->right) {
        w = w->right;
    }
    return w;
}

void SymbolTable::remove(Symbol *res) {
    if (this->root == nullptr)
        return;
    splay(res);

    Symbol *lh = this->root->left;
    Symbol *rh = this->root->right;

    if (!lh && !rh) {
        delete this->root;
        this->root = nullptr;
        return;
    } else if (!lh) {
        rh->parent = nullptr;
        delete this->root;
        this->root = rh;
        return;
    } else if (!rh) {
        lh->parent = nullptr;
        delete this->root;
        this->root = lh;
    } else {
        lh->parent = nullptr;
        delete this->root;
        Symbol *x = getMaxValueNode(lh);
        splay(x);
        this->root = x;
        x->right = rh;
        rh->parent = x;
    }
}

void SymbolTable::remove(int level) {
    while (root) {
        Symbol *res = getMaxValueNode(root);
        if (res->level != level) {
            break;
        }
        splay(res);
        remove(res);
    }
}

string SymbolTable::getParaType(string para, int &num_comp, int &num_splay) {
    regex number("\\d+");
    regex str("\'[A-Za-z0-9 ]*\'");
    regex var("[a-z][\\w]*");

    string res = "";
    string sub = "";
    for (unsigned int i = 0; i < para.length(); i++) {
        // cout << sub << endl;
        if (para[i] != ',') {
            sub += para[i];
        }
        if (para[i] == ',' || i == para.length() - 1) {
            if (regex_match(sub, number))
                res += "number,";
            else if (regex_match(sub, str))
                res += "string,";
            else if (regex_match(sub, var)) {
                Symbol *x = search(sub, num_comp, num_splay);
                if (!x)
                    return "undeclared";
                if (x->type == 0)
                    res += "number,";
                else if (x->type == 1)
                    res += "string,";
            } else
                return "error";

            sub = "";
        }
    }
    if (res != " ")
        return res.substr(0, res.size() - 1);
    return "";
}

Symbol *SymbolTable::bst_search(string name, int level) {
    Symbol x(name, level, 0);
    Symbol *walker = this->root;
    while (walker) {
        int order = x.compare(walker);
        if (order == 0) {
            return walker;
        } else if (order < 0) {
            if (walker->left == nullptr) {
                return walker;
            }
            walker = walker->left;
        } else {
            if (walker->right == nullptr) {
                return walker;
            }
            walker = walker->right;
        }
    }
    return nullptr;
}

Symbol *SymbolTable::search(string name, int &num_comp, int &num_splay) {
    if (this->root == nullptr)
        return nullptr;
    for (int level = this->cur_level; level >= 0; level--) {
        Symbol *res = bst_search(name, level);
        if (res->name.compare(name) == 0) {
            res = search_level(name, level, num_comp);
            num_splay += splay(res);
            return res;
        }
    }

    return NULL;
}

void SymbolTable::insert(smatch m) {
    // Get data from smatch m
    smatch m1;
    string line = m.str(0);
    string name = m.str(1);
    string type_str = m.str(2);
    string is_static = m.str(3);

    int level = is_static == "true" ? 0 : this->cur_level;

    // Handle type
    regex string("string");
    regex number("number");
    regex function(
        "\\(((number|string)(,number|,string)*)?\\)->(number|string)");

    int type = -1;
    if (regex_match(type_str, number))
        type = 0;
    if (regex_match(type_str, string))
        type = 1;
    if (regex_match(type_str, m1, function))
        type = 2;

    if (type == -1)
        throw InvalidInstruction(line);
    if (type == 2 && level != 0)
        throw InvalidDeclaration(line);

    int num_splay = 0;
    int num_comp = 0;

    Symbol *new_symbol = new Symbol(name, level, type);
    if (type == 2) {
        new_symbol->para = m1.str(0);
    }

    Symbol *walker = this->root;
    Symbol *p = nullptr;

    while (walker != nullptr) {
        p = walker;
        int order = new_symbol->compare(walker);
        if (order < 0) {
            walker = walker->left;
            num_comp++;
        } else if (order > 0) {
            walker = walker->right;
            num_comp++;
        } else {
            delete new_symbol;
            throw Redeclared(m.str(0));
        }
    }

    if (p == nullptr) {
        this->root = new_symbol;
        cout << num_comp << " " << num_splay << endl;
        return;
    }

    walker = new_symbol;
    new_symbol->parent = p;

    if (new_symbol->compare(p) < 0)
        p->left = walker;
    else
        p->right = walker;

    if (walker != nullptr || walker->parent != nullptr) {
        splay(walker);
        num_splay++;
    }

    cout << num_comp << " " << num_splay << endl;
}

void SymbolTable::assign(smatch m) {
    smatch m1;
    int num_comp = 0;
    int num_splay = 0;
    string line = m.str(0);
    string name = m.str(1);
    string value = m.str(2);

    // Regex
    regex number("\\d+");
    regex str("\'[A-Za-z0-9 ]*\'");
    regex var("[a-z][\\w]*");
    regex function_call("([^ ]*)\\((.*)\\)");

    // Get type of value
    // number, string
    if (regex_match(value, number) || regex_match(value, str)) {
        Symbol *res = search(name, num_comp, num_splay);
        int type = regex_match(value, number) ? 0 : 1;
        if (res == nullptr || res->name.compare(name) != 0)
            throw Undeclared(line);
        if (res->type != type)
            throw TypeMismatch(line);

        cout << num_comp << " " << num_splay << endl;
        return;
    }
    // variable
    if (regex_match(value, var)) {
        // Check value first
        Symbol *s = search(value, num_comp, num_splay);
        if (!s || s->name.compare(value) != 0)
            throw Undeclared(line);
        // Search for name
        Symbol *des = search(name, num_comp, num_splay);
        if (!des || des->name.compare(name) != 0)
            throw Undeclared(line);
        // Check type
        if (des->type != s->type)
            throw TypeMismatch(line);

        cout << num_comp << " " << num_splay << endl;
        return;
    }
    // Function call
    if (regex_match(value, m1, function_call)) {
        smatch m2;
        string f_name = m1.str(1);
        string para = m1.str(2);
        string para_pattern;
        string return_type;

        // Search for function name
        Symbol *s = search(f_name, num_comp, num_splay);
        if (!s || s->name.compare(f_name) != 0)
            throw Undeclared(line);
        if (s->type != 2)
            throw TypeMismatch(line);

        regex function_pattern(
            "\\(((number|string)(,number|,string)*)?\\)->(number|string)");
        if (regex_match(s->para, m2, function_pattern)) {
            para_pattern = m2.str(1);
            return_type = m2.str(m2.size() - 1);
        }

        para = getParaType(para, num_comp, num_splay);
        // Check para pass valid with function
        if (para == "error")
            throw TypeMismatch(line);
        if (para == "undeclared")
            throw Undeclared(line);
        if (para.compare(para_pattern) != 0) {
            throw TypeMismatch(line);
        }
        // Search for name
        Symbol *des = search(name, num_comp, num_splay);
        if (!des || des->name != name)
            throw Undeclared(line);
        // Check return type
        if (des->type != getType(return_type))
            throw TypeMismatch(line);

        cout << num_comp << " " << num_splay << endl;
        return;
    }

    throw InvalidInstruction(line);
}

void SymbolTable::begin() { this->cur_level++; }

void SymbolTable::end() {
    this->cur_level--;
    if (this->cur_level < 0)
        throw UnknownBlock();
    this->remove(cur_level + 1);
}

void SymbolTable::lookup(smatch m) {
    if (this->root == nullptr)
        throw Undeclared(m.str(0));

    string name = m.str(1);

    for (int level = cur_level; level >= 0; level--) {
        if (h_lookup(name, level))
            break;
    }

    if (this->root->name != name)
        throw Undeclared(m.str(0));

    cout << this->root->level << endl;
}

void SymbolTable::print() {
    string res = preorder(this->root);
    if (res != "") {
        cout << res.substr(0, res.size() - 1) << endl;
    }
}

void SymbolTable::run(string filename) {
    // Regex
    smatch m;
    regex insert_expr("INSERT ([a-z][\\w]*) ([^ ]*) (true|false)");
    regex assign_expr("ASSIGN ([a-z][\\w]*) (.*)");
    regex begin_expr("BEGIN");
    regex end_expr("END");
    regex lookup_expr("LOOKUP ([a-z][\\w]*)");
    regex print_expr("PRINT");
    regex remove_expr("REMOVE ([a-z][\\w]*)");

    // Read file
    string s;
    ifstream file(filename);
    while (getline(file, s)) {
        if (regex_match(s, m, insert_expr)) {
            this->insert(m);
        } else if (regex_match(s, m, assign_expr)) {
            this->assign(m);
        } else if (regex_match(s, m, lookup_expr)) {
            this->lookup(m);
        } else if (regex_match(s, m, begin_expr)) {
            this->begin();
        } else if (regex_match(s, m, end_expr)) {
            this->end();
        } else if (regex_match(s, m, print_expr)) {
            this->print();
        } else {
            throw InvalidInstruction(s);
        }
    }

    if (this->cur_level > 0) {
        throw UnclosedBlock(this->cur_level);
    }
}
