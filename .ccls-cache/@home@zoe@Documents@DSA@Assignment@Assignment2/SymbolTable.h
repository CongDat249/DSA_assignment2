#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#include "main.h"

class Symbol {
  private:
    string name, para;
    int type;
    int level;
    Symbol *right, *left, *parent;

    int compare(Symbol*);

  public:
    Symbol();
    Symbol(string, int, int, Symbol*);

    friend class SymbolTable;
};

class SymbolTable {
  private:
    Symbol* root;
    int cur_level;

    void clear(Symbol*);
    void right_rotate(Symbol*);
    void left_rotate(Symbol*);
    void remove(Symbol*);
    void remove(int);
    int splay(Symbol*);
    bool h_lookup(string, int);
    string preorder(Symbol*);
    Symbol* search(string, int&, int&);
    Symbol* search_level(string, int, int&);
    Symbol* getMaxValueNode(Symbol* root);
    Symbol* bst_search(string, int);

  public:
    SymbolTable();
    ~SymbolTable();
    void run(string filename);
    int getType(string);
    string getParaType(string, int&, int&);
    int getValueType(string);
    void insert(smatch);
    void assign(smatch);
    void begin();
    void end();
    void lookup(smatch);
    void print();
};
#endif
