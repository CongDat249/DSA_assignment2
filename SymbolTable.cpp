#include "SymbolTable.h"

// Symbol implement
Symbol::Symbol(string name, int level, int type) {
    this->name = name;
    this->level = level;
    this->type = type;
    this->left = this->right = this->parent = nullptr;
}

Symbol::Symbol(string name, int level, int type, Symbol *parent) {
    this->name = name;
    this->level = level;
    this->type = type;
    this->parent = parent;
    this->left = this->right = nullptr;
}

bool Symbol::operator==(Symbol x) {
    return this->level == x.level && !(this->name.compare(x.name));
}

bool Symbol::operator<(Symbol x) {
    if (this->level < x.level)
        return true;
    else if (this->level == x.level && this->name.compare(x.name) < 0) {
        return true;
    }
    return false;
}

bool Symbol::operator>(Symbol x) {
    return !(this->operator<(x) || this->operator==(x));
}
// Symbol table implement
SymbolTable::SymbolTable() {
    this->root = nullptr;
    this->cur_level = 0;
}

SymbolTable::~SymbolTable() {
    clear(this->root);
}

void SymbolTable::clear(Symbol *root) {
    if (root == nullptr) return;

    clear(root->left);
    clear(root->right);
    delete root;
}

int SymbolTable::getType(string str_type) {
    regex function_type("\\(((number|string)(,number|,string)*)?\\)->(number|string)");
    if (str_type == "number") return 0;
    if (str_type == "string") return 1;
    if (regex_match(str_type, function_type)) return 2;
    return -1;
}

void SymbolTable::right_rotate(Symbol *x) {
    if (x == nullptr || x->left == nullptr) return;

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
    if (x == nullptr || x->right == nullptr) return;

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

void SymbolTable::splay(Symbol *x) {
    if (x == nullptr || x->parent == nullptr)
        return;

    while (x->parent != nullptr) {
        Symbol *p = x->parent;
        if (p->parent == nullptr) {
            if (p->left == x)
                right_rotate(p);
            else if (p->right == x)
                left_rotate(p);

            return;
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
}

Symbol *SymbolTable::searchHelper(string name, int level) {
    int num_comp = 0;
    Symbol x(name, level, 0);
    Symbol *walker = this->root;
    while (walker != nullptr) {
        if (*walker == x) {
            return walker;
        } else if (x < *walker) {
            num_comp++;
            walker = walker->left;
        } else {
            num_comp++;
            walker = walker->right;
        }
    }
    return nullptr;
}

bool SymbolTable::search(string name) {
    Symbol *res = nullptr;
    for (int level = cur_level; level >= 0; level--) {
        res = searchHelper(name, level);
        if (res) break;
    }
    if (!res) return false;
    splay(res);
    return true;
}

void SymbolTable::insert(smatch m) {
    int num_comp = 0;
    int num_splay = 0;
    string name = m.str(1);
    string is_static = m.str(m.size() - 1);
    int level = is_static == "true" ? 0 : this->cur_level;
    int type = getType(m.str(2));
    if (type == -1) throw InvalidInstruction(m.str(0));

    Symbol *new_symbol = new Symbol(name, level, type);

    Symbol *walker = this->root;
    Symbol *p = nullptr;

    while (walker != nullptr) {
        p = walker;
        if (*walker > *new_symbol) {
            walker = walker->left;
            num_comp++;
        } else if (*walker < *new_symbol) {
            walker = walker->right;
            num_comp++;
        } else {
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

    if (*new_symbol < *p)
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
    string name = m.str(1);
    string value = m.str(2);

    if (!this->search(name)) {
        throw Undeclared(m.str(0));
    }
    Symbol *des = this->root;
    // Regex
    regex number_expr("\\d+");
    regex string_expr("\'[A-Za-z0-9 ]*\'");
    regex var_expr("[a-z][\\w]*");
    regex function_call_expr("");

    if (regex_match(value, number_expr) && des->type == 0) {
        des->value = value;
    } else if (regex_match(value, string_expr) && des->type == 1) {
        des->value = value;
    } else if (regex_match(value, var_expr)) {
        if (!this->search(value)) throw Undeclared(m.str(0));
        Symbol *s = this->root;
        if (des->type == s->type) {
            des->value = s->value;
        } else {
            throw TypeMismatch(m.str(0));
        }
    } else if (regex_match(value, function_call_expr)) {
        // if (!this->search(value)) throw Undeclared(m.str(0));
    }

    cout << this->root->name;
}

void SymbolTable::begin() {
    this->cur_level++;
}

void SymbolTable::end() {
    this->cur_level--;
    if (this->cur_level < 0) throw UnknownBlock();
    // Delete symbol in this scope
}

void SymbolTable::lookup(smatch m) {
    string name = m.str(1);
    if (search(name)) {
        cout << this->root->level << endl;
        return;
    }
    throw Undeclared(m.str(0));
}

void SymbolTable::preorder(Symbol *root) {
    if (root == nullptr) return;

    cout << root->name << " " << root->level << " " << root->type << endl;
    preorder(root->left);
    preorder(root->right);
}

void SymbolTable::print() {
    preorder(this->root);
}

void SymbolTable::run(string filename) {
    // Regex
    smatch m;
    // regex insert_expr("(INSERT) ([a-z][\\w]*) ((\\(((number|string)(,number|,string)*)?\\)->)?(number|string)) (true|false)");
    regex insert_expr("INSERT ([a-z][\\w]*) ([^ ]*) (true|false)");
    regex assign_expr("ASSIGN ([a-z][\\w]*) ([^ ]*)");
    regex begin_expr("BEGIN");
    regex end_expr("END");
    regex lookup_expr("LOOKUP ([a-z][\\w]*)");
    regex print_expr("PRINT");

    // Read file
    string s;
    ifstream file(filename);
    while (getline(file, s)) {
        // Insert command
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