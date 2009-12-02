
/** Parser based on the libxml++ example */

#include <libxml++/libxml++.h>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;

class tnode {
  public:
    int id;
    bool last;
    tnode * parent;
    tnode(int _id, bool _last, tnode * _parent):id(_id),last(_last),parent(_parent) {}
    bool less(const tnode & n) const {
      if(parent==NULL) return true;
      if(n.parent==NULL) return false;
      if(id<n.id) return true;
      if(id>n.id) return false;
      return (*parent).less(*n.parent);
    }
    bool operator<(const tnode & n) const {
      if(parent==NULL) return true;
      if(n.parent==NULL) return false;
      if((*parent).less(*(n.parent))) return true;
      return false;
    }
};

vector<tnode *> nodes;
map<string, int> ids;
int max_id = 1;

void process_node(const xmlpp::Node* node, tnode * parent, bool last) {
  const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
  const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(node);
  const xmlpp::CommentNode* nodeComment = dynamic_cast<const xmlpp::CommentNode*>(node);

  Glib::ustring nodename = node->get_name();

  //Treat the various node types differently: 
  if(nodeText) { // text
    return;
  }
  else if(nodeComment) { // comment
    return;
  }
  else if(nodeContent) { // content
    return;
  }

  string snode = nodename;
  if(ids[snode]==0) {
    ids[snode] = max_id++;
  }
  tnode * act_node = new tnode(ids[snode],last,parent);
  
  bool hasAttr = false;
  if(const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node)) { //A normal Element node:
    //Print attributes:
    const xmlpp::Element::AttributeList& attributes = nodeElement->get_attributes();
    for(xmlpp::Element::AttributeList::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter) {
      const xmlpp::Attribute* attribute = *iter;
      string aname = attribute->get_name();
      if(ids[aname]==0)
        ids[aname] = max_id++;
      nodes.push_back(new tnode(ids[aname],false,act_node));
      hasAttr = true;
    }
  }
  
  if(!nodeContent) {
    //Recurse through child nodes:
    xmlpp::Node::NodeList list = node->get_children();
    xmlpp::Node::NodeList::iterator iter = list.begin();
    if(iter==list.end() && hasAttr) { // last attribute was a 1
      nodes[nodes.size()-1]->last=true;
    }
    for(; iter != list.end(); ++iter) {
      bool last = false;
      ++iter; if(iter==list.end()) last = true; --iter;
      process_node(*iter, act_node, last);
    }
  }
}


int main(int argc, char* argv[])
{
  string filepath, fileout;
  if(argc == 3 ) {
    filepath = argv[1]; 
    fileout = argv[2];
  } else {
    cout << "usage: %s <input> <output>\n" << endl;
    return 0;
  }
  
  try {
    xmlpp::DomParser parser;
    parser.set_validate();
    parser.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
    parser.parse_file(filepath);
    if(parser) {
      //Walk the tree:
      const xmlpp::Node* pNode = parser.get_document()->get_root_node(); //deleted by DomParser.
      process_node(pNode, NULL, 0);
    }
    sort(nodes.begin(),nodes.begin());
    for(int i=0;i<nodes.size();i++)
      cout << "alpha=" << nodes[i]->id << "  last=" << nodes[i]->last << endl;
  }
  catch(const std::exception& ex) {
    cout.flush();
    cout << "Exception caught: " << ex.what() << std::endl;
  }

  return 0;
}
