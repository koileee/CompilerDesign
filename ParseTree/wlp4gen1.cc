#include <vector>
#include <set>
#include <string>
#include <memory>
#include <iostream>
#include <list>
#include <sstream>
#include <stack>
#include <map>

using namespace std;

// Tree class
class Tree {
  public:
    string rules;
    string type="";
    vector< string > tokens;
    vector<shared_ptr<Tree>> children;
};

// Symbol class
class Symbol {
  public:
    vector<string> signature;
    map<string, string> vars;
};
//top symbol table
map<string, shared_ptr<Symbol>> topSymTbl;
//keep track of current function
string currentFunction;
//terms
set<string> terms = {"BOF", "BECOMES", "COMMA", "ELSE", "EOF", "EQ", "GE",
                        "GT", "ID", "IF", "INT", "LBRACE", "LE", "LPAREN", "LT",
                        "MINUS", "NE", "NUM", "PCT", "PLUS", "PRINTLN", "RBRACE",
                        "RETURN", "RPAREN", "SEMI", "SLASH", "STAR", "WAIN",
                        "WHILE", "AMP", "LBRACK", "RBRACK", "NEW", "DELETE",
                        "NULL"};


// make a tree based on input
shared_ptr<Tree> makeTree(){
  string s;
  getline(cin, s);
  istringstream ss(s);
  shared_ptr<Tree> t = make_shared<Tree>();
  ss >> t->rules;
  string rule;
  while (ss >> rule){
    t->tokens.push_back(rule);
  }

  if (terms.find(t->rules) == terms.end()) {

    for (int i=0; i<t->tokens.size(); i++){
      t->children.push_back(makeTree());
    }
  }
  return t;
}

string checkTypes(shared_ptr<Tree> t, int arg=0){
  if(t->rules == "ID"){
    t->type = topSymTbl[currentFunction]->vars[t->tokens[0]];
  }else if(t->rules == "NUM"){
    t->type = "int";
  }else if(t->rules == "NULL"){
    t->type = "int*";
  }else if(t->rules == "params"){
    t->type = "well";
  }else if(t->rules == "paramlist"){
    t->type = "well";
  }else if(t->rules == "type"){
    if(t->tokens[1] == "STAR"){
      t->type = "int*";
    }else{
      t->type = "int";
    }
  }else if(t->rules == "dcls"){
    if(t->tokens.size() == 0){
      t->type = "well";
    }else{
      string dclstype = checkTypes(t->children[0]);
      string dcltype = checkTypes(t->children[1]);
      if(dclstype == "well"){
        if(dcltype == "int" && t->children[3]->rules == "NUM"){
          t->type = "well";
        }else if(dcltype == "int*" && t->children[3]->rules == "NULL"){
          t->type = "well";
        }
      }
    }
  }else if(t->rules == "dcl"){
    t->type = checkTypes(t->children[1]);
  }else if(t->rules == "statements"){
    if(t->tokens.size() == 0){
      t->type = "well";
    }else{
      if(checkTypes(t->children[0]) == "well" && checkTypes(t->children[1]) == "well"){
        t->type = "well";
      }
    }
  }else if(t->rules == "statement"){
    if(t->tokens.size() == 4){
      if(checkTypes(t->children[0]) == checkTypes(t->children[2])){
        t->type = "well";
      }
    }else if(t->children[0]->rules == "IF"){
      if(checkTypes(t->children[2]) == "well"
         && checkTypes(t->children[5]) == "well"
         && checkTypes(t->children[9]) == "well"){
        t->type = "well";
      }
    }else if(t->children[0]->rules == "WHILE"){
      if(checkTypes(t->children[2]) == "well" && checkTypes(t->children[5]) == "well"){
        t->type = "well";
      }
    }else if(t->children[0]->rules == "PRINTLN"){
      if(checkTypes(t->children[2]) == "int"){
        t->type = "well";
      }
    }else if(t->children[0]->rules == "DELETE"){
      if(checkTypes(t->children[3]) == "int*"){
        t->type = "well";
      }
    }
  }else if(t->rules == "test"){
    if(checkTypes(t->children[0]) == checkTypes(t->children[2])){
      t->type = "well";
    }
  }else if(t->rules == "expr"){
    if(t->tokens.size() == 1){
      t->type = checkTypes(t->children[0]);
    }else{
      string ltype = checkTypes(t->children[0]);
      string rtype = checkTypes(t->children[2]);
      string op = t->children[1]->rules;
      if(ltype == "int" && rtype == "int"){
        t->type = "int";
      }else if(ltype == "int*" && rtype == "int"){
        t->type = "int*";
      }else if(ltype == "int" && rtype == "int*" && op == "PLUS"){
        t->type = "int*";
      }else if(ltype == "int*" && rtype == "int*" && op == "MINUS"){
        t->type = "int";
      }
    }
  }else if(t->rules == "term"){
    if(t->tokens.size() == 1){
      t->type = checkTypes(t->children[0]);
    }else if(checkTypes(t->children[0]) == "int" && checkTypes(t->children[2]) == "int"){
      t->type = "int";
    }
  }else if(t->rules == "factor"){
    if(t->tokens.size() == 1){
      t->type = checkTypes(t->children[0]);
    }else if(t->children[0]->rules == "LPAREN"){
      t->type = checkTypes(t->children[1]);
    }else if(t->children[0]->rules == "AMP"){
      if(checkTypes(t->children[1]) == "int"){
        t->type = "int*";
      }
    }else if(t->children[0]->rules == "STAR"){
      if(checkTypes(t->children[1]) == "int*"){
        t->type = "int";
      }
    }else if(t->children[0]->rules == "NEW"){
      if(checkTypes(t->children[3]) == "int"){
        t->type = "int*";
      }
    }else if(t->tokens.size() == 3){
      t->type = "int";
    }else if(t->tokens.size() == 4){
      if(checkTypes(t->children[2]) == "well"){
        t->type = "int";
      }
    }
  }else if(t->rules == "arglist"){
    if(t->tokens.size() == 1 && arg == topSymTbl[currentFunction]->signature.size() - 1){
      if(topSymTbl[currentFunction]->signature[arg] == checkTypes(t->children[0])){
        t->type = "well";
      }
    }else if(t->tokens.size() == 3){
      if(topSymTbl[currentFunction]->signature[arg] == checkTypes(t->children[0]) &&
      checkTypes(t->children[2], arg+1) == "well"){
        t->type = "well";
      }
    }
  }else if(t->rules == "lvalue"){
    if(t->tokens.size() == 1){
      t->type = checkTypes(t->children[0]);
    }else if(t->tokens.size() == 2){
      if(checkTypes(t->children[1]) == "int*"){
        t->type = "int";
      }
    }else{
      t->type = checkTypes(t->children[1]);
    }
  }else if(t->rules == "procedure"){
    string paramstype = checkTypes(t->children[3]);
    string dclstype = checkTypes(t->children[6]);
    string statetype = checkTypes(t->children[7]);
    string rettype = checkTypes(t->children[9]);
    if(paramstype == "well" && dclstype == "well" && statetype == "well" && rettype == "int"){
      t->type = "well";
    }
  }else if(t->rules == "main"){
    string arg1type = checkTypes(t->children[3]);
    string arg2type = checkTypes(t->children[5]);
    string dclstype = checkTypes(t->children[8]);
    string statetype = checkTypes(t->children[9]);
    string rettype = checkTypes(t->children[11]);
    if(arg2type == "int" && dclstype == "well" && statetype == "well" && rettype == "int"){
      t->type = "well";
    }
  }

  string err;
  if(t->type == ""){
    err="ERROR, type wrong";
    throw err;
  }
  return t->type;
}

// check symbol table for errors
void checktable(shared_ptr<Tree> t, shared_ptr<Symbol> s){
  if(t->children.size() && t->children[0]->rules == "ID"){
    if(t->children.size() == 1 && !s->vars.count(t->children[0]->tokens[0])){
      throw "ERROR variable " + t->children[0]->tokens[0] + " not declared";
    }else if(t->children.size() > 1){
      if(s->vars.count(t->children[0]->tokens[0])){
        throw "ERROR variable is not function";
      }else if(!topSymTbl.count(t->children[0]->tokens[0])){
        throw "ERROR function " + t->children[0]->tokens[0] + " not declared";
      }
    }
  }

  for(vector<shared_ptr<Tree>>::iterator it=(t->children).begin(); it != (t->children).end(); it++){
    checktable(*it, s);
  }
}


// adds to symbol table given a dcl node
void addDcl( shared_ptr<Tree> t, shared_ptr<Symbol> s, bool signature){
  string var;
  if(t->children[0]->children.size() == 1){
    var = "int";
  }else{
    var = "int*";
  }

  if(signature){
    s->signature.push_back(var);
  }

  if(!s->vars.count(t->children[1]->tokens[0])){
    s->vars[t->children[1]->tokens[0]] = var;
  }else{
    throw "ERROR name " + t->children[1]->tokens[0] + " already declared";
  }
}

// adds to symbol table given a dcls node
void addDcls(shared_ptr<Tree> t, shared_ptr<Symbol> s){
  if(t->children.size() > 0){
    addDcl(t->children[1], s, false);
    addDcls(t->children[0], s);
  }
}


// add params based on paramslist
void addparams(shared_ptr<Tree> t, shared_ptr<Symbol> s){
  if(t->children.size() == 3){
    addDcl(t->children[0], s, true);
    addparams(t->children[2], s);
  }else if(t->children.size() == 1){
    addDcl(t->children[0], s, true);
  }
}

// Make the symbolTable
void makeSymbolTable(shared_ptr<Tree> t){
  shared_ptr<Symbol> s = make_shared<Symbol>();
  string error;
  string type;
  if (t->tokens[0] == "main"){
    currentFunction = "wain";
    shared_ptr<Tree>  wain = t->children[0];
    try{

      addDcl(wain->children[3], s, true);
      addDcl(wain->children[5], s, true);
      addDcls(wain->children[8], s);
      checktable(wain, s);
      topSymTbl[currentFunction] = s;

      string type = checkTypes(wain);
      if(type != "well"){
        error = "ERROR: not well typed";
        throw error;
      }
    }catch(const string& msg){
      throw msg;
    }


  } else if (t->tokens[0] == "procedure"){
    shared_ptr<Tree>  proc = t->children[0];
    currentFunction = proc->children[1]->tokens[0];
    string error;

    try{


      if( topSymTbl.find( currentFunction ) != topSymTbl.end() ){
        error = "ERROR: repeating procedure";
        throw error;
      } else if (currentFunction == "wain") {
        error = "ERROR: can't name wain";
        throw error;
      }
      topSymTbl[currentFunction] = s;

      if(proc->children[3]->children.size() == 1){
        addparams(proc->children[3]->children[0], s);
      }


      addDcls(proc->children[6], s);
      checktable(proc, s);

      type = checkTypes(proc);
      if(type != "well"){
        error = "ERROR: not well typed";
        throw error;
      }
    }catch(const string& msg){
      topSymTbl.erase(currentFunction);
      throw msg;
    }
    makeSymbolTable(t->children[1]);
  }

}

int main(){
  shared_ptr<Tree> t = makeTree();
  try{
    makeSymbolTable(t->children[1]);
    for(auto table : topSymTbl){
      if(table.first != "wain"){
      cerr<<table.first;
      for(auto sign: table.second->signature){
        cerr << " " << sign;
      }
      cerr << endl;
      for(auto sign : table.second->vars){
        cerr << sign.first << " " << sign.second << endl;
      }
      cerr<<endl;
    }
  }
    cerr<<"wain";
    for(auto sign: topSymTbl["wain"]->signature){
      cerr << " " << sign;
    }
    cerr<<endl;
    for(auto const &variable : topSymTbl["wain"]->vars){
      cerr << variable.first << " " << variable.second << endl;
    }
  } catch (const string& errMsg) {
    cerr << errMsg << endl;
  }
}
