#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#include "main.h"

class Symbol {
  private:
    string name;
    int level;
    int type;
    string value;
    Symbol *right;
    Symbol *left;
    Symbol *parent;

    // Overload operator
    bool operator==(Symbol);
    bool operator<(Symbol);
    bool operator>(Symbol);

  public:
    Symbol();
    Symbol(string, int, int);
    Symbol(string, int, int, Symbol *);

    friend class SymbolTable;
};

class SymbolTable {
  private:
    Symbol *root;
    int cur_level;

    void clear(Symbol *);
    void right_rotate(Symbol *);
    void left_rotate(Symbol *);
    void splay(Symbol *);
    void preorder(Symbol *);
    Symbol* searchHelper(string, int);
    bool search(string);

  public:
    SymbolTable();
    ~SymbolTable();
    void run(string filename);
    int getType(string);
    void insert(smatch);
    void assign(smatch);
    void begin();
    void end();
    void lookup(smatch);
    void print();
};
#endif