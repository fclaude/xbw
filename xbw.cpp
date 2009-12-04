
#include "xbw.h"

XBW::XBW(const string & filename, const string & params) {
  string filename2 = filename+".xbw";
  ifstream input(filename2.c_str(),ios::binary);
  assert(input.good());
  
  // Read the number of nodes
  input.read((char*)&nodesCount,sizeof(uint));
  //cout << "nodesCount=" << nodesCount << endl;

  // Read the alpha sequence
  uint * alphaInt = new uint[nodesCount]; 
  input.read((char*)alphaInt,nodesCount*sizeof(uint));

  // Read bitmap last
  uint * lastInt = new uint[nodesCount/W+1];
  input.read((char*)lastInt,(nodesCount/W+1)*sizeof(uint));

  // Read bitmap leaf
  //uint * leafInt = new uint[nodesCount/W+1];
  //input.read((char*)leafInt,(nodesCount/W+1)*sizeof(uint));

  // Initialize and compute the max label
  maxLabel = 0;
  for(uint i=0;i<nodesCount;i++) {
    maxLabel = max(alphaInt[i],maxLabel);
  }

  uint * AInt = new uint[nodesCount/W+2];
  input.read((char*)AInt,(nodesCount/W+2)*sizeof(uint));

  // Create the data structures
  alphabet_mapper * am = new alphabet_mapper_none();
  //wt_coder * wcc = new wt_coder_huff(alphaInt, nodesCount, am);
  static_bitsequence_builder * sbb = new static_bitsequence_builder_brw32(20);
  static_bitsequence_builder * sbb2 = new static_bitsequence_builder_sdarray();
  //static_sequence_builder * ssb = new static_sequence_builder_wvtree(wcc, sbb, am);
  alpha = new static_sequence_bs(alphaInt, nodesCount, am, sbb2);
  //alpha = ssb->build(alphaInt, nodesCount);
  last = sbb->build(lastInt, nodesCount);
  //leaf = sbb->build(leafInt, nodesCount);
  A = sbb->build(AInt, nodesCount+1);
  /*for(uint i=0;i<A->length();i++)
    cout << A->access(i);
  cout << endl;*/

  //alphaTmp = alphaInt;
  balpha = bits(maxLabel);
  alphaSeq = new uint[uint_len(nodesCount,balpha)];
  for(uint i=0;i<uint_len(nodesCount,balpha);i++)
    alphaSeq[i] = 0;
  for(uint i=0;i<nodesCount;i++)
    set_field(alphaSeq,balpha,i,alphaInt[i]);

  input.close();
  delete sbb;
  delete sbb2;
  // Free the temporary arrays
  delete [] alphaInt;
  delete [] lastInt;
  //delete [] leafInt;
  delete [] AInt;
}

XBW::~XBW() {
  delete alpha;
  delete last;
  //delete leaf;
  delete A;
  delete [] alphaSeq;
}

uint XBW::size() const {
  uint s = sizeof(XBW)+alpha->size()+last->size();
  //s += sizeof(uint)*nodesCount;
  s += uint_len(nodesCount,balpha)*sizeof(uint);
  return s;
}

void XBW::getChildren(const uint n, uint * ini, uint * fin) const {
  if(isLeaf(n)) {
    *fin=0;
    *ini=1;
    return;
  }
  //uint c = alphaTmp[n]; 
  //uint c = alpha->access(n);
  //uint c = get_field(alphaSeq,balpha,n);
  uint c = getLabel(n);
  uint r = alpha->rank(c, n);
  uint y = A->select1(c);
  uint z = last->rank1(y-1);
  *ini = last->select1(z+r-1)+1;
  *fin = last->select1(z+r);
}

uint XBW::getRankedChild(const uint n, const uint k) const {
  uint ini, fin;
  getChildren(n, &ini, &fin);
  if(k>fin-ini+1)
    return (uint)-1;
  return ini+k-1;
}

uint XBW::getCharRankedChild(const uint n, const uint l, const uint k) const {
  uint ini,fin;
  getChildren(n, &ini, &fin);
  if(ini>fin) 
    return (uint)-1;
  uint y1 = alpha->rank(l, ini-1);
  uint y2 = alpha->rank(l, fin);
  if(k > y2-y1) 
    return (uint)-1;
  return alpha->select(l,y1+k);
}

uint XBW::getDegree(const uint n) const {
  uint ini,fin;
  getChildren(n, &ini, &fin);
  return fin-ini+1;
}

uint XBW::getCharDegree(const uint n, const uint l) const {
  uint ini,fin;
  getChildren(n, &ini, &fin);
  if(ini>fin) return 0;
  return alpha->rank(l, fin) - alpha->rank(l, ini-1);
}

uint XBW::getParent(const uint n) const {
  if(n==0) 
    return (uint)-1;
  uint c = A->rank1(n);
  uint y = A->select1(c);
  uint k = last->rank1(n-1)-last->rank1(y-1);
  uint p = alpha->select(c,k+1);
  return p;
}

bool XBW::isLeaf(const uint n) const {
  return maxLabel==getLabel(n);
}

void XBW::subPathSearch(const uint * qry, const uint ql, uint * ini, uint * fin) const {
  if(ql<=1) {
    *ini = 0;
    *fin = nodesCount-1;
    return;
  }
  uint first = A->select1(qry[0]);
  uint last  = A->select1(qry[0]+1)-1;
  //cout << "first=" << first << " last=" << last << endl;
  for(uint i=1;i<ql-1;i++) {
    uint k1 = alpha->rank(qry[i],first-1);
    uint z1 = alpha->select(qry[i], k1+1);
    uint k2 = alpha->rank(qry[i],last);
    uint z2 = alpha->select(qry[i], k2);
    if(k1==k2) { // No occurrence
      *ini = 1;
      *fin = 0;
      return;
    }
    first = getRankedChild(z1,1);
    last = getRankedChild(z2,getDegree(z2));
    //cout << "first=" << first << " last=" << last << endl;
  }
  *ini = first;
  *fin = last;
}

uint * XBW::subPathSearch(const uint * qry, const uint ql, uint * len) const {
  uint ini, fin;
  subPathSearch(qry,ql,&ini,&fin);
  if(ini>fin) {
    len = 0;
    return NULL;
  }
  uint elem = qry[ql-1];
  uint k1 = (ini==0?0:alpha->rank(elem, ini-1));
  uint k2 = alpha->rank(elem, fin);
  *len = k2-k1;
  uint * ret = new uint[*len];
  for(uint p=k1+1; p<=k2; p++)
    ret[p-k1-1] = alpha->select(elem, p);
  return ret;
}

uint XBW::getLabel(const uint n) const {
  //return alphaTmp[n]; 
  //return alpha->access(n);
  return get_field(alphaSeq,balpha,n);
}

