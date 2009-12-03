
#include <iostream>
#include <fstream>

#include <basics.h>
#include <static_sequence.h>
#include <static_bitsequence.h>

using namespace std;

/** Implementation of the XBW-Index proposed by Ferragina, Luccio, Manzini and Muthukrishnan
 * in FOCS 2005. The idea is patented, this code can't be redistributed nor used for
 * commercial purposes.
 *
 * @author Francisco Claude
 * @email fclaude@cs.uwaterloo.ca
 */
class XBW {
  public:
    /** Builds an XBW object reading it from a file, it uses ssb to create the \alpha sequence
     * and sbb to create the bit-sequences.
     * @param filename file where the index is stored
     * @param params parameters not implemented
     */
    XBW(const string filename, string params);

    /** Destroys the object */
    ~XBW();

    /** Searchs for path query in the tree. Computes the range of nodes that satisfy the
     * query up to the last item.
     * @param qry query, qry[0]=a and qry[1]=b means /a/b
     * @param ql length of the query
     * @param ini pointer for storing the initial position of the range answering the query
     * @param fin pointer for storing the final position of the range answering the query
     */
    void subPathSearch(const uint * query, const uint ql, uint * ini, uint * fin) const;

    /** Searchs for path query in the tree, produces an array with the node ids that
     * satisfy the query.
     * @param qry query, qry[0]=a and qry[1]=b means /a/b
     * @param ql length of the query
     * @param len pointer to store the length of the result
     * @return array with node ids descending from a path labeles qry, the length of the array is stored in len
     */
    uint * subPathSearch(const uint * qry, const uint ql, uint * len) const;

    /** Computes the parent of a node
     * @param n node id
     * @return parent of n
     */
    uint getParent(const uint n) const;

    /** Computes the k-th child of node n
     * @param n node id
     * @param k child number
     * @return commputes the k-th child of node n
     */
    uint getRankedChild(const uint n, const uint k) const;

    /** Selects the k-th children of node n with label l
     * @param n node id
     * @param l label
     * @param k occurrence
     * @return k-th occurrence of l among the children of n
     */
    uint getCharRankedChild(const uint n, const uint l, const uint k) const;

    /** Computes how many children does node n have
     * @param n node id
     * @return degree of n
     */
    uint getDegree(const uint n) const;

    /** Computes how many children of node n have label l
     * @param n node id
     * @param l label
     * @return number of children of n with label l
     */
    uint getCharDegree(const uint n, const uint l) const;

    /** checks whether node n is a leaf or not 
     * @param n node id
     * @return true if n is a leaf
     */
    bool isLeaf(const uint n) const;

    /** Returns the size of the XBW structure */
    uint size() const;

    /** gets the range where the children of node n are
     * @param n node id
     * @param ini pointer to where to store the initial position of the range
     * @param fin pointer to where to store the final position of the range
     */
    void getChildren(const uint n, uint * ini, uint * fin) const;

  protected:
    /** number of nodes in the tree */
    uint nodesCount;

    /** max label */
    uint maxLabel;

    /** labels of the nodes in the tree */
    static_sequence * alpha;

    /** marks the rightmost child of every node */
    static_bitsequence * last;

    /** marks which nodes are leaves */
    static_bitsequence * leaf;
    
    /** marks the beginning of each symbol run in \pi */
    static_bitsequence * A;

};

