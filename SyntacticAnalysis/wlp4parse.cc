#include <vector>
#include <set>
#include <string>
#include <memory>
#include <iostream>
#include <list>
#include <sstream>
#include <stack>
#include <map>
#include <fstream>
using namespace std;

vector<string> terminals;
vector<string> nonterminals;
vector<string> prodRules;
vector<string> transitions;
map<string, string> stateTrans;
string start;
string states;

string startRule;


class State {
  public:
    bool term;
    string rule;
    string val;
    vector<unique_ptr<State>> states;
};


void readRule(vector<string> &t, istream &f) {
    string l;
    int n;
    getline(f,l);
    n = stoi(l);
    //cout<<n<<endl;
    string temp;
    for(int i = 0; i < n; i++) {
        getline(f,temp);
        t.push_back(temp);
        //cout<<temp<<endl;
        if(temp.substr(0, temp.find(" ")) == start){
          startRule=temp;
        }
    }
}

void readTerms(vector<string> &t, istream &f) {
    string l;
    int n;
    getline(f,l);
    n = stoi(l);
    //cout<<n<<endl;
    string temp;
    for(int i = 0; i < n; i++) {
        getline(f, temp);
        t.push_back(temp);
        //cout<<temp<<endl;
    }
}

void readTerms(istream &f) {
    string l;
    int n;
    getline(f,l);
    n = stoi(l);
    //cout<<n<<endl;
    string temp;
    for(int i = 0; i < n; i++) {
        getline(f, temp);
    }
}



void transmap(){

  for (vector<string>::iterator it=transitions.begin(); it!=transitions.end(); ++it){
    istringstream iss(*it);
    string l;
    iss >> l;
    string sym;
    iss >> sym;
    string action;
    iss >> action;
    string r;
    iss >> r;
    //cout<<*it<<endl;
    int i = stoi(r);
    if(action == "reduce"){
      r = prodRules[i];
    }
    stateTrans[l + " " + sym] = action + " " + r;
  }
}


unique_ptr<State> parse(string seq){
  int index = -1;
  unique_ptr<State> next = NULL;

  string transition;
  string action;
  string temp;
  string trans;
  vector<string> input;
  vector<unique_ptr<State>> symStack;
  vector<string> stateStack;
  stateStack.push_back("0");

  istringstream iss(seq);
  //cout<<seq<<endl;
  string n;
  while (!iss.eof()){
    iss>>n;
    input.push_back(n);
  }

  for(unsigned int y=0; y<input.size(); y++){
    if(next == NULL){
      next = make_unique<State> ();
      next->term = true;
      temp = input[y];
      y++;
      next->rule = temp;
      temp = input[y];
      next->val = temp;
      index++;
    } else{
      y--;
    }

    if(stateTrans.count(stateStack.back() + " " + next->rule)){
      trans = stateTrans[stateStack.back() + " " + next->rule];
      istringstream t(trans);
      t >> action;
      if (action == "shift") {
        t>>temp;
        stateStack.push_back(temp);
        symStack.push_back(move(next));
        next = NULL;
      } else if(action == "reduce"){
        y-=2;
        index--;
        t>>next->rule;
        next->val = "";
        next->term = false;
        while(t >> temp){

          next->states.push_back(move(symStack.back()));
          next->val +=  " " + temp;
          symStack.pop_back();
          stateStack.pop_back();
        }
      }
    } else {
      //cout<<stateStack.back() + " " + next->rule<<endl;
        cerr << "ERROR at " << index << endl;
        return NULL;
    }
  }

  unique_ptr<State> result = make_unique<State> ();
  result->rule = start;
  result->val = " BOF procedures EOF";
  result->term = false;

  for(vector<unique_ptr<State>>::reverse_iterator it=(symStack).rbegin(); it != (symStack).rend(); it++){
      result->states.push_back(move(*it));
  }

  if(result->states[1]->rule == "procedures" && result->states.size() == 3){
    return result;
  }else{
    cerr << "ERROR at " << index - 1 << endl;
    return NULL;
  }
}

void print(unique_ptr<State> &s) {
  cout << s->rule;
  if(s->term){
    cout << " ";
  }
  cout <<s->val << endl;
    for(vector<unique_ptr<State>>::reverse_iterator it=(s->states).rbegin(); it != (s->states).rend(); it++){
        print(*it);
    }

}



int main(){
  ifstream rules;
  rules.open("rules.lr1");
  readTerms(rules); // read terminals
  readTerms(rules); // read nonterminals
  getline(rules,start); // read start symbol
  readRule(prodRules, rules); // read production rules
  getline(rules,states);
  readTerms(transitions, rules); // read transitions
  transmap(); // create transistion map
  string line;
  string line2 ="BOF BOF";
  while(getline(cin, line)){
    line2.append(" "+line);
  }
  line2.append(" EOF EOF");
  unique_ptr<State> t = parse(line2);
  if(t != NULL){
    print(t);
  }
}
