
#include <iostream>
#include <fstream>
#include <map>

using namespace std;

#include <basics.h>

#include "xbw.h"

map<string,uint> ids;
map<uint,string> rids;
uint maxTag = 0;
uint maxNode = 0;
uint * mapping;
xbw & index;

uint * parse(string s, int * len) {
  vector<uint> res;
  s = s.substr(2);
  uint pos = s.find("/");
  while(pos!=string::npos) {
    res.push_back(ids[s.substr(0,pos)]);
    s = s.substr(pos);
  }
  res.push_back(ids[s]);
  *len = res.size();
  uint * ret = new uint[*len];
  for(uint i=0;i<*len;i++)
    ret[i] = res[i];
  return ret;
}

void answerQueries() {
  string s;
  while(cin >> s) {
    uint len, lres;
    uint * qry = parse(s,&len);
    uint * res = index.subPathSearch(qry,len,&lres);
    cout << "Results for " << s << endl;
    for(uint i=0;i<lres;i++) {
      cout << "* " << res[i] << "  map=" << mapping[res[i]] << "  tag=" << rids[res[i]] << endl;
    }
    delete [] res;
    delete [] qry;
  }
}

int main(int argc, char ** argv) {
  if(argc!=2) {
    cout << "usage: " << argv[0] << " <base name>" << endl;
    return 0;
  }
  string basename = argv[1];

  index = new xbw(basename, "");

  ifstream input((basename+".dict").c_str());
  input >> maxTag;
  for(uint i=0;i<maxTag;i++) {
    string tag;
    uint id;
    input >> tag >> id;
    ids[tag] = id;
    rids[id] = tag;
  }
  input.close();

  input = ifstream((basename+".map").c_str(),ios::binary);
  input.read((char*)&maxNode,sizeof(uint));
  mapping = new uint[maxNode];
  input.read((char*)mapping,sizeof(uint));
  input.close();

  answerQueries();
}

