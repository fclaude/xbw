
/** Parser based on the libxml++ example */

#include <libxml++/libxml++.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <cassert>

#include <basics.h>

using namespace std;

class tnode {
  public:
    static int act_nr;
    int nr;
    int id;
    bool last;
    tnode * parent;
    int leaf;
    tnode(int _id, bool _last, tnode * _parent, int _leaf)
      :id(_id),last(_last),parent(_parent),leaf(_leaf) { 
        nr = ++act_nr; 
      }
    bool less(const tnode & n) const {
      if(id<n.id) return true;
      if(id>n.id) return false;
      if(parent==n.parent) {
        if(nr<n.nr) return true;
        else return false;
      }
      if(parent==NULL) return true;
      if(n.parent==NULL) return false;
      return (*parent).less(*n.parent);
    }
    bool cmp(const tnode & n) const {
      if(parent==n.parent) {
        if(nr<n.nr) return true;
        else return false;
      }
      if(parent==NULL) return true;
      if(n.parent==NULL) return false;
      if((*parent).less(*(n.parent))) return true;
      return false;
    }
};

bool compare(const tnode * n1, const tnode * n2) {
  return (*n1).cmp(*n2);
}

int maxV = (1<<30);
int tnode::act_nr = 0;
vector<tnode *> nodes;
map<string, int> ids;
int max_id = 1;
map<int,int> occ;

string tagstr(int id) {
  for(map<string,int>::iterator iter = ids.begin(); iter != ids.end(); ++iter) {
    if(id==iter->second) return iter->first;
  }
  return "";
}

void saveDictionary(string fname) {
  ofstream out(fname.c_str());
  assert(out.good());
  out << ids.size() << endl;
  for(map<string,int>::iterator iter = ids.begin(); iter != ids.end(); ++iter) 
    out << iter->first << " " << iter->second << endl;
  out.close();
}

void saveMapping(string fname) {
  ofstream out(fname.c_str(),ios::binary);
  assert(out.good());
  uint len = nodes.size();
  uint * mapping = new uint[len];
  for(uint i=0;i<nodes.size();i++)
    mapping[i] = nodes[i]->nr;
  out.write((char*)&len,sizeof(uint));
  out.write((char*)mapping,len*sizeof(uint));
  out.close();
  delete [] mapping;
}

void inc_occ(uint id) {
  //cout << id << " ";
  if(occ[id]==0)
    occ[id]=1;
  else 
    occ[id]=occ[id]+1;
}

tnode* process_node(const xmlpp::Node* node, tnode * parent, bool last) {
  const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
  const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(node);
  const xmlpp::CommentNode* nodeComment = dynamic_cast<const xmlpp::CommentNode*>(node);

  Glib::ustring nodename = node->get_name();

  //Treat the various node types differently: 
  if(nodeText) { // text
    return NULL;
  }
  else if(nodeComment) { // comment
    return NULL;
  }
  else if(nodeContent) { // content
    return NULL;
  }

  string snode = nodename;
  if(ids[snode]==0) {
    ids[snode] = max_id++;
  }
  tnode * act_node = new tnode(ids[snode],last,parent,0);
  if(parent!=NULL)
    inc_occ(parent->id); //occ[parent->id] = occ[parent->id]+1;
  nodes.push_back(act_node);
  
  bool hasAttr = false;
  bool hasChild = false;
  tnode * lastchild=NULL;
  if(const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node)) { //A normal Element node:
    //Print attributes:
    const xmlpp::Element::AttributeList& attributes = nodeElement->get_attributes();
    for(xmlpp::Element::AttributeList::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter) {
      const xmlpp::Attribute* attribute = *iter;
      string aname = string("@")+attribute->get_name();
      if(ids[aname]==0)
        ids[aname] = max_id++;
      lastchild = new tnode(ids[aname],false,act_node,0);
      inc_occ(act_node->id); //occ[act_node->id] = occ[act_node->id]+1;
      nodes.push_back(lastchild);
      nodes.push_back(new tnode(maxV,true,lastchild,1));
      inc_occ(lastchild->id);
      hasAttr = true;
      hasChild = true;
    }
  }
  
  if(!nodeContent) {
    //Recurse through child nodes:
    xmlpp::Node::NodeList list = node->get_children();
    xmlpp::Node::NodeList::iterator iter = list.begin();
    for(; iter != list.end(); ++iter) {
      tnode * ret;
      bool last = false;
      ++iter; 
      if(iter==list.end()) 
        last = true; 
      --iter;
      if((ret=process_node(*iter, act_node, last))) {
        lastchild = ret;
        hasChild = true;
      }
    }
  }
  
  if(lastchild!=NULL)
    lastchild->last=1;
  if(!hasChild) { 
    tnode * bottom = new tnode(maxV,true,act_node,1);
    nodes.push_back(bottom);
    inc_occ(act_node->id);
    act_node->leaf=0;
  }
  return act_node;
}


int main(int argc, char* argv[])
{
  string filepath, fileout;
  if(argc == 3) {
    filepath = argv[1]; 
    fileout = argv[2];
  } else {
    cout << "usage: " << argv[0] << " <input file> <output base name>" << endl;
    return 0;
  }
  
  try {
    xmlpp::DomParser parser;
    //parser.set_validate();
    //parser.set_substitute_entities(); 
    parser.parse_file(filepath);
    if(parser) {
      const xmlpp::Node* pNode = parser.get_document()->get_root_node(); 
      process_node(pNode, NULL, 0);
    }
    for(uint i=0;i<nodes.size();i++)
      if(nodes[i]->id==maxV)
        nodes[i]->id=max_id;
    max_id++;
    sort(nodes.begin(),nodes.end(),compare);
    /*for(unsigned int i=0;i<nodes.size();i++)
      cout << "alpha=" << nodes[i]->id 
        << "  last=" << nodes[i]->last 
        << "  tagstr=" << tagstr(nodes[i]->id) 
        << "  leaf=" << nodes[i]->leaf 
        << "  nr=" << nodes[i]->nr << endl;*/
    int len = nodes.size();
    int * alpha = new int[len];
    int * last  = new int[len/W+1];
    int * leaf = new int[len/W+1];
    int * A = new int[len/W+2];
    //cout << "len=" << len << endl;
    for(int i=0;i<len/W+1;i++)
      last[i] = leaf[i] = 0;
    for(int i=0;i<len/W+2;i++)
      A[i]=0;
    for(int i=0;i<len;i++) {
      alpha[i] = nodes[i]->id;
      if(nodes[i]->last)
        bitset(last,i);
      if(nodes[i]->leaf)
        bitset(leaf,i);
    }
    uint sum = 1;
    bitset(A,1);
    for(int i=0;i<max_id;i++) {
      sum += occ[i];
      bitset(A,sum);
      //cout << "occ[" << i << "]=" << occ[i] << endl;
    }
    //cout << "sum=" << sum << endl;
    ofstream out((fileout+".xbw").c_str(), ios::out | ios::binary);
    assert(out.good());
    out.write((char*)&len,sizeof(uint));
    out.write((char*)alpha,len*sizeof(uint));
    out.write((char*)last,(len/W+1)*sizeof(uint));
    out.write((char*)A,(len/W+2)*sizeof(uint));
    out.close();
    saveDictionary(fileout+".dict");
    saveMapping(fileout+".map");
    delete [] alpha;
    delete [] A;
    delete [] leaf;
    delete [] last;
    for(uint i=0;i<nodes.size();i++)
      delete nodes[i];
  } catch(const std::exception& ex) {
    cout.flush();
    cout << "Exception caught: " << ex.what() << std::endl;
  }

  return 0;
}
