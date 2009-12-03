
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

using namespace std;

#include <basics.h>

#include "xbw.h"

map<string,uint> ids;
map<uint,string> rids;
uint maxTag = 0;
uint maxNode = 0;
uint * mapping;
XBW * index;

uint * parse(string s, uint * len) {
  vector<uint> res;
  s = s.substr(2);
  uint pos = s.find("/");
  while(pos!=string::npos) {
    res.push_back(ids[s.substr(0,pos)]);
    s = s.substr(pos+1);
    pos = s.find("/");
  }
  res.push_back(ids[s]);
  *len = res.size();
  uint * ret = new uint[*len];
  for(uint i=0;i<*len;i++) {
    ret[i] = res[i];
  }
  return ret;
}

/* Time meassuring */
double ticks= (double)sysconf(_SC_CLK_TCK);
struct tms t1,t2;

void start_clock() {
  times (&t1);
}

double stop_clock() {
  times (&t2);
  return (t2.tms_utime-t1.tms_utime)/ticks;
}
/* end Time meassuring */

#define REP 1

void answerQueries() {
  while(!cin.eof()) {
    string s;
    cout << "> ";
    cin >> s;
    if(s.length()==0) break;
    uint len, lres;
    uint * qry = parse(s,&len);
    start_clock();
    uint * res = NULL;
    for(uint k=0;k<REP;k++) {
      res = index->subPathSearch(qry,len,&lres);
      if(res!=NULL)
        delete [] res;
    }
    res = index->subPathSearch(qry,len,&lres);
    double time = 1000.*stop_clock()/(REP+1);
    cout << " Results for " << s << endl;
    //for(uint i=0;i<lres;i++) {
    //  cout << "  * " << res[i] << "  map=" << mapping[res[i]] << "  tag=" << rids[index->getLabel(res[i])] << endl;
    //}
    cout << " (results: " << lres << " | time: " << time << "ms | index size: " << index->size()/1024 << "Kb)" << endl;
    cout << endl;
    delete [] res;
    delete [] qry;
  }
  cout << endl;
}

int main(int argc, char ** argv) {
  if(argc!=2) {
    cout << "usage: " << argv[0] << " <base name>" << endl;
    return 0;
  }
  string basename = argv[1];

  index = new XBW(basename, "");

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

  ifstream input2((basename+".map").c_str(),ios::binary);
  input2.read((char*)&maxNode,sizeof(uint));
  mapping = new uint[maxNode];
  input2.read((char*)mapping,maxNode*sizeof(uint));
  input2.close();

  answerQueries();
}

