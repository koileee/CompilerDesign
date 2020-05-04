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
class Info{
public:
  string decType;
  int location;
};

// Symbol class
class Symbol {
  public:
    vector<string> signature;
    map<string, Info> vars;
};

int globalLocation = 0;

int globalWhile = 0;
int globalIf=0;

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

string getIDType(shared_ptr<Tree> tree){
  string name;
  if (tree->rules == "factor"){
    if (tree->tokens[0] == "NUM") {
      return "int";
    } else if (tree->tokens[0] == "NULL") {
      return "int*";
    } else if (tree->tokens[0] == "ID"){
      name = tree->children[0]->tokens[0];
      //cout<<"name: "<<name<<"type: "<<tree->type<<endl;
      if (topSymTbl[currentFunction]->vars.find(name) != topSymTbl[currentFunction]->vars.end()){
        return topSymTbl[currentFunction]->vars[name].decType;
      } else {
        return tree->type;
        //throw string("ERROR: ID not found 1");
      }
    } else if (tree->tokens[0] == "STAR"){
      name = tree->children[1]->children[0]->tokens[0];
      if (topSymTbl[currentFunction]->vars.find(name) != topSymTbl[currentFunction]->vars.end()){
        return topSymTbl[currentFunction]->vars[name].decType;
      } else {
        throw string("ERROR: ID not found 1");
      }
    }
  } else if (tree->rules == "lvalue" && tree->tokens[0] == "ID"){
    name = tree->children[0]->tokens[0];
    if (topSymTbl[currentFunction]->vars.find(name) != topSymTbl[currentFunction]->vars.end()){
      return topSymTbl[currentFunction]->vars[name].decType;
    } else {
      throw string("ERROR: ID not found 1");
    }
  } else if (tree->children.size() != 0){
    return getIDType(tree->children[0]);
  }

  return "";
}


void codeGen(shared_ptr<Tree> t){
  if(t->rules == "main"){
    //main -> INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE

    cout << "wain:" << endl;
    cout << "sw $31,-4($30)" << endl;
    cout << "sub $30,$30,$4" << endl;
    cout << "sub $29,$30,$4" << endl;
    cout << "lis $12" << endl;
    cout << ".word " << 4*topSymTbl[currentFunction]->vars.size() << endl;
    cout<< ";; "<<currentFunction<<endl;
    cout << "sub $30,$30,$12" << endl;
    cout << "sw $1," << topSymTbl[currentFunction]->vars[t->children[3]->children[1]->tokens[0]].location<< "($29)" << endl;
    cout << "sw $2," << topSymTbl[currentFunction]->vars[t->children[5]->children[1]->tokens[0]].location<< "($29)" << endl;
    if(topSymTbl[currentFunction]->vars[t->children[3]->children[1]->tokens[0]].decType == "int"){
      cout << "add $2,$0,$0" << endl;
    }
    cout << "lis $10" << endl;
    cout << ".word init" << endl;
    cout << "jalr $10" << endl;
    codeGen(t->children[8]);
    codeGen(t->children[9]);
    cout << ";;return" << endl;
    codeGen(t->children[11]);
    cout << "add $30, $29, $4" << endl;
    cout << "add $30,$30,$4" << endl;
    cout << "lw $31,-4($30)" << endl;
    cout << "jr $31" << endl;

  } else if(t->rules == "procedure"){
    //procedure -> INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
    cout << "func" << currentFunction << ":" << endl;
    cout << "sw $31,-4($30)" << endl;
    cout << "sub $30,$30,$4" << endl;
    cout << "sub $29,$30,$4" << endl;
    cout << "lis $12" << endl;
    cout << ".word " << 4 * topSymTbl[currentFunction]->vars.size() << endl;
    cout<< ";; "<<currentFunction<<endl;
    cout << "sub $30,$30,$12" << endl;
    codeGen(t->children[6]);
    //os << ";;statements" << endl;
    codeGen(t->children[7]);
    //os << ";;return" << endl;
    codeGen(t->children[9]);
    cout << "add $30,$29,$4" << endl;
    cout << "add $30,$30,$4" << endl;
    cout << "lw $31,-4($30)" << endl;
    cout << "jr $31" << endl;
  }else if(t->rules == "expr"){
    if (t->tokens[0] == "term"){
      t->type = t->children[0]->type;
      codeGen(t->children[0]);

    } else {
      codeGen(t->children[0]);
      cout << "sw $3, -4($30)" << endl;
      cout << "sub $30, $30, $4" << endl;
      codeGen(t->children[2]);
      cout << "add $30, $30, $4" << endl;
      cout << "lw $5, -4($30)" << endl;

      // pointer arithmetic
      // int* , int : + or -
      // int , int* : +
      // int*, int* :

      if (t->tokens[1] == "PLUS"){
        if(getIDType(t->children[0]) == "int*"){
          cout << "mult $3, $4" << endl;
          cout << "mflo $3" << endl;
          t->type = "int*";
        }else if(getIDType(t->children[0]) == "int*"){
          cout << "mult $5,$4" << endl;
          cout << "mflo $5" << endl;
          t->type = "int*";
        }else{
          t->type = "int";
        }
        cout << "add $3,$5,$3" << endl;;
      } else if (t->tokens[1] == "MINUS"){

        if(getIDType(t->children[0]) == "int*" && getIDType(t->children[0]) == "int"){
          cout << "mult $3,$4" << endl;
          cout << "mflo $3" << endl;
          cout << "sub $3,$5,$3" << endl;
          t->type = "int*";
        }else if(getIDType(t->children[0]) == "int" && getIDType(t->children[0]) == "int"){
          cout << "sub $3,$5,$3" << endl;
          t->type = "int";
        }else if(getIDType(t->children[0]) == "int*" && getIDType(t->children[0]) == "int*"){
          cout << "sub $3,$5,$3" << endl;
          cout << "div $3,$4" << endl;
          cout << "mflo $3" << endl;
          t->type = "int";
        }
      }
    }
  }else if(t->rules == "term" ){
    if (t->tokens[0]=="factor"){
      // term factor
      codeGen(t->children[0]);
      t->type = t->children[0]->type;
    } else {
      //term -> term x factor
      codeGen(t->children[0]);
      cout << "sw $3, -4($30)" << endl;
      cout << "sub $30, $30, $4" << endl;
      codeGen(t->children[2]);
      cout << "add $30, $30, $4" << endl;
      cout << "lw $5, -4($30)" << endl;

      if (t->tokens[1] == "STAR"){
        //term -> term STAR factor
        cout << "mult $5, $3" << endl;
        cout << "mflo $3" << endl;
      } else if (t->tokens[1] == "SLASH"){
        //term -> term SLASH factor
        cout << "div $5, $3" << endl;
        cout << "mflo $3" << endl;
      } else if (t->tokens[1] == "PCT"){
        //term -> term PCT factor
        cout << "div $5, $3" << endl;
        cout << "mfhi $3" << endl;
      }
      t->type = "int";
    }
  }else if(t->rules == "factor"){
    if(t->tokens[0] == "ID" && (t->tokens.size()==1)){
      t->type = topSymTbl[currentFunction]->vars[t->children[0]->tokens[0]].decType;
      //factor -> ID
      cout << "lw $3," << topSymTbl[currentFunction]->vars[t->children[0]->tokens[0]].location << "($29)" << endl;
    } else if (t->tokens[0] == "LPAREN") {
      //factor -> LPAREN expr RPAREN
      codeGen(t->children[1]);
      t->type = t->children[1] -> type;
    } else if (t->tokens[0] == "NUM") {
      //factor -> NUM
      cout << "lis $3" << endl;
      cout << ".word " << t->children[0]->tokens[0] << endl;
      t->type = "int";
    } else if (t->tokens[0] == "NULL"){
      //factor -> NULL
      cout << "add $3,$0,$11" << endl;
      t->type = "int*";

    } else if (t->tokens[0]=="AMP"){
      //factor → AMP lvalue
      codeGen(t->children[1]);
      t->type = "int*";
    } else if (t->tokens[0] == "STAR"){
      //factor → STAR factor
      codeGen(t->children[1]);
      t->type = "int";
      cout << "lw $3,0($3)" << endl;
    } else if(t->tokens[0] == "NEW"){
      //factor → NEW INT LBRACK expr RBRACK
      codeGen(t->children[3]);
      cout << "add $1,$3,$0" << endl;
      cout << "lis $3" << endl;
      cout << ".word new" << endl;
      cout << "jalr $3" << endl;
      cout << "bne $3,$0,1" << endl;
      cout << "add $3,$0,$11" << endl;
      t->type = "int*";
    }else if(t->tokens[0] == "ID"){
      //factor -> ID LPAREN (arglist) RPAREN
      cout << "add $26,$30,$0" << endl;

      if(t->children.size() == 4){
        //factor -> ID LPAREN arglist RPAREN
        codeGen(t->children[2]);
      }


      cout << "sw $29,-4($30)" << endl;
      cout << "sub $30,$30,$4" << endl;
      cout << "sw $26,-4($30)" << endl;
      cout << "sub $30,$30,$4" << endl;
      cout << "lis $12" << endl;
      cout << ".word func" << t->children[0]->tokens[0] << endl;
      cout << "jalr $12" << endl;
      cout << "add $30,$30,$4" << endl;
      cout << "lw $26,-4($30)" << endl;
      cout << "add $30,$30,$4" << endl;
      cout << "lw $29,-4($30)" << endl;
      cout << "add $30,$26,$0" << endl;
      t->type = "int";
    }
  }else if(t->rules == "arglist"){
    //arglist -> expr (COMMA aglist)
    codeGen(t->children[0]);
    cout << "sw $3,-4($30)" << endl;
    cout << "sub $30,$30,$4" << endl;
    if(t->tokens.size() == 3){
      codeGen(t->children[2]);
    }
  } else if (t->rules == "statements"){
    if (t->tokens.size() == 2){
      //statements → statements statement
      codeGen(t->children[0]);
      codeGen(t->children[1]);
    }
  } else if (t->rules == "statement" ){
    if (t->tokens[0]=="PRINTLN") {
      //  statement → PRINTLN LPAREN expr RPAREN SEMI
      codeGen(t->children[2]);
      // push $31
      /*
      cout << "sw $31, -4($30)" << endl;
      cout << "sub $30, $30, $4" << endl;
      */

      cout << "add $1, $3, $0" << endl;
      cout << "lis $10" << endl;
      cout << ".word print" << endl;
      cout << "jalr $10" << endl;

      // pop $31
      /*
      cout << "add $30, $30, $4" << endl;
      cout << "lw $31, -4($30)" << endl;
      */
    } else if (t->tokens.size()==4){
      // statement → lvalue BECOMES expr SEMI

      codeGen(t->children[2]);
      cout << "sw $3,-4($30)" << endl;
      cout << "sub $30,$30,$4" << endl;
      codeGen(t->children[0]);
      cout << "add $30,$30,$4" << endl;
      cout << "lw $5,-4($30)" << endl;
      cout << "sw $5,0($3)" << endl;
    } else if (t->tokens.size()==7){
      //statement → WHILE LPAREN test RPAREN LBRACE statements RBRACE
      globalWhile++;
      int localWhile = globalWhile;
      string startLabel = "sw" + to_string(localWhile);
      string end = startLabel+"e";
      cout << startLabel << ":" << endl;
      codeGen(t->children[2]);
      cout << "beq $3, $0,"<<end << endl;
      codeGen(t->children[5]);
      cout << "beq $0, $0," << startLabel << endl;
      cout << end << ":" << endl;
    } else if(t->tokens[0] == "IF"){
      //statement -> IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE
      globalIf++;
      int localIf = globalIf;
      string slabel = "si" + to_string(localIf);
      string elabel = slabel+"e";
      codeGen(t->children[2]);
      cout << "beq $3,$0," << slabel << endl;
      codeGen(t->children[5]);
      cout << "beq $0,$0," << elabel << endl;
      cout << slabel << ":" << endl;
      codeGen(t->children[9]);
      cout << elabel << ":" << endl;
    } else if(t->tokens[0] == "DELETE"){
      //statement -> DELETE LBRACK RBRACK expr SEMI
      codeGen(t->children[3]);
      cout << "beq $3,$11,4" << endl;
      cout << "add $1,$3,$0" << endl;
      cout << "lis $3" << endl;
      cout << ".word delete" << endl;
      cout << "jalr $3" << endl;
    }

  }else if(t->rules== "lvalue"){
    if(t->tokens[0] == "ID"){
      //lvalue -> ID
      string name = t->children[0]->tokens[0];
      cout << "lis $3" << endl;
      cout << ".word " << topSymTbl[currentFunction]->vars[name].location  << endl;
      cout << "add $3,$3,$29" << endl;
    }else if(t->tokens.size() == 3){
      //lvalue -> LPAREN lvalue RPAREN
      codeGen(t->children[1]);
    } else if (t->tokens[0] == "STAR"){
       //lvalue → STAR factor
       codeGen(t->children[1]);
    }

  } else if (t->rules == "dcls"){
    if (t->tokens.size() == 5) {
      //dcls dcls dcl BECOMES NUM/NULL SEMI
      codeGen(t->children[0]);
      string val = t->children[3]->tokens[0];
      cout << "lis $3" << endl;
      if(t->tokens[3] == "NUM")
        cout << ".word " << val << endl;
      else{
        cout << ".word 1" << endl;
      }
      string name = t->children[1]->children[1]->tokens[0];
      int offset;
      if (topSymTbl[currentFunction]->vars.find(name) != topSymTbl[currentFunction]->vars.end()){
        offset = topSymTbl[currentFunction]->vars[name].location;
      } else {
        throw string("ERROR: ID " + name  +" not found ");
      }
      cout << "sw $3, " << offset << "($29)" << endl;
    }
  } else if (t->rules == "test") {
    //test → expr LT expr
    if (t->tokens.size() != 0) {
      codeGen(t->children[0]);
      cout << "sw $3, -4($30)" << endl;
      cout << "sub $30, $30, $4" << endl;
      codeGen(t->children[2]);
      cout << "add $30, $30, $4" << endl;
      cout << "lw $5, -4($30)" << endl;
      if (t->tokens[1]=="LT"){
        //test → expr LT expr
        if(t->children[0]->type == "int*"){
          cout << "sltu $3,$5,$3" << endl;
        }else{
          cout << "slt $3,$5,$3" << endl;
        }
      } else if (t->tokens[1]=="EQ") {
        //test → expr EQ expr
        if(t->children[0]->type == "int*"){
          cout << "sltu $6,$3,$5" << endl;
          cout << "sltu $7,$5,$3" << endl;
        }else{
          cout << "slt $6,$3,$5" << endl;
          cout << "slt $7,$5,$3" << endl;
        }
        cout << "add $3,$6,$7" << endl;
        cout << "sub $3,$11,$3" << endl;
      } else if (t->tokens[1]=="NE"){
        //test → expr NE expr
        if(t->children[0]->type == "int*"){
          cout << "sltu $6,$3,$5" << endl;
          cout << "sltu $7,$5,$3" << endl;
        }else{
          cout << "slt $6,$3,$5" << endl;
          cout << "slt $7,$5,$3" << endl;
        }
        cout << "add $3,$6,$7" << endl;
      } else if (t->tokens[1]=="LE"){
        // test → expr LE expr
        if(t->children[0]->type == "int*"){
          cout << "sltu $3,$3,$5" << endl;
        }else{
          cout << "slt $3,$3,$5" << endl;
        }
        cout << "sub $3,$11,$3" << endl;
      }else if (t->tokens[1]=="GE"){
        // test → expr GE expr
        if(t->children[0]->type == "int*"){
          cout << "sltu $3,$5,$3" << endl;
        }else{
          cout << "slt $3,$5,$3" << endl;
        }
        cout << "sub $3,$11,$3" << endl;
      } else if (t->tokens[1]=="GT"){
        //  test → expr GT expr
        if(t->children[0]->type == "int*"){
          cout << "sltu $3,$3,$5" << endl;
        }else{
          cout << "slt $3,$3,$5" << endl;
        }
      }
    }

  }

}

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
    s->signature.push_back(t->children[1]->tokens[0]);
  }

  if(!s->vars.count(t->children[1]->tokens[0])){
    s->vars[t->children[1]->tokens[0]].decType = var;
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
      int i = 0;
      for (auto it = s->vars.begin(); it != s->vars.end(); it++){
        s->vars[it->first].location = i;
        i -= 4;
      }
      topSymTbl[currentFunction] = s;
      codeGen(wain);
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


      if(proc->children[3]->children.size() == 1){
        addparams(proc->children[3]->children[0], s);
      }


      addDcls(proc->children[6], s);

      int i = 0;
      for (auto it = s->vars.begin(); it != s->vars.end(); it++){
        string name = it->first;
        s->vars[name].location = i;
        i -= 4;
      }
      i  = 16;
      for(auto it = s->signature.rbegin(); it != s->signature.rend(); it++){
        string name = *it;
        //cout<<"name: "<<name<<endl;
        s->vars[name].location = i;
        i += 4;
      }
      topSymTbl[currentFunction] = s;

      codeGen(proc);
      makeSymbolTable(t->children[1]);
    }catch(const string& msg){
      topSymTbl.erase(currentFunction);
      throw msg;
    }

  }

}

int main(){
  shared_ptr<Tree> t = makeTree();


  try{
    cout << ".import print" << endl;
    cout << ".import init" << endl;
    cout << ".import delete" << endl;
    cout << ".import new" << endl;

    cout << "lis $4" << endl;
    cout << ".word 4" << endl;
    cout << "lis $11" << endl;
    cout << ".word 1" << endl;
    cout<<"beq $0,$0,wain" << endl;
    makeSymbolTable(t->children[1]);

  } catch (const string& errMsg) {
    cerr << errMsg << endl;
  }
}
