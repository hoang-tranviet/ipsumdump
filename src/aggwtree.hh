#ifndef AGGWTREE_HH
#define AGGWTREE_HH
#include "aggtree.hh"

class AggregateWTree { public:

    static const int COUNT_ADDRS = 0;
    static const int COUNT_PACKETS = 1;
    static const int LEAF = 16;
    static const int COUNT_ADDRS_LEAF = COUNT_ADDRS | LEAF;
    struct WNode;

    AggregateWTree(int count_what);
    AggregateWTree(const AggregateWTree &);
    AggregateWTree(const AggregateTree &, int count_what);
    ~AggregateWTree();

    bool ok(ErrorHandler * = 0) const;

    uint32_t num_active() const			{ return _num_nonzero; }
    uint32_t num_nonzero() const		{ return _num_nonzero; }
    uint32_t nnz() const			{ return _num_nonzero; }
    
    void add(uint32_t aggregate, int32_t count = 1);

    void cull_addresses(uint32_t nnz);
    void cull_addresses_by_packets(uint32_t nnz);
    void cull_packets(uint32_t np);

    void cut_smaller_prefix(int p, uint32_t size);

    void prefixize(int p);
    void make_prefix(int p, AggregateWTree &) const;

    void num_active_prefixes(Vector<uint32_t> &) const;
    void num_discriminated_by_prefix(Vector<uint32_t> &) const;

    void collect_active(Vector<WNode *> &) const;
    
    void fake_by_discriminating_prefix(int, const Vector<uint32_t> &);
    
    int read_file(FILE *, ErrorHandler *);
    int write_file(FILE *, bool binary, ErrorHandler *) const;
    int write_hex_file(FILE *, ErrorHandler *) const;

    AggregateWTree &operator=(const AggregateWTree &);

    typedef AggregateTree::Node Node;
    struct WNode : public Node {
	uint32_t full_count;
	int depth;
	inline WNode *child(int w) const;
	inline WNode *&child(int w);
    };

  public:
    
    WNode *_root;
    WNode *_free;
    Vector<WNode *> _blocks;

    uint32_t _num_nonzero;
    int _count_type;
    bool _topheavy;

    WNode *new_node();
    WNode *new_node_block();
    void free_node(WNode *);
    void initialize_root();
    void copy_nodes(const Node *, uint32_t = 0xFFFFFFFFU);
    void kill_all_nodes();
    void set_count_type(int);

    WNode *make_peer(uint32_t, WNode *);
    void finish_add(WNode *, int32_t, WNode *stack[], int);
    void free_subtree_x(WNode *, uint32_t &, uint32_t &);
    void collapse_subtree(WNode *, WNode *stack[], int);
    void delete_subtree(WNode *, WNode *stack[], int);
    void adjust_num_nonzero(int32_t, WNode *stack[], int);

    uint32_t node_ok(WNode *, int, uint32_t *, ErrorHandler *) const;
    WNode *pick_random_active_node(WNode *stack[], int *) const;

    uint32_t node_local_count(WNode *) const;
    uint32_t node_full_count(WNode *) const;

    void node_prefixize(WNode *, int, WNode *stack[], int);
    
    friend class AggregateTree;
    
};

inline AggregateWTree::WNode *
AggregateWTree::WNode::child(int w) const
{
    assert(w == 0 || w == 1);
    return (WNode *) Node::child[w];
}

inline AggregateWTree::WNode *&
AggregateWTree::WNode::child(int w)
{
    assert(w == 0 || w == 1);
    return (WNode *) Node::child[w];
}

inline AggregateWTree::WNode *
AggregateWTree::new_node()
{
    if (_free) {
	WNode *n = _free;
	_free = n->child(0);
	return n;
    } else
	return new_node_block();
}

inline void
AggregateWTree::free_node(WNode *n)
{
    n->child(0) = _free;
    _free = n;
}

inline uint32_t
AggregateWTree::node_local_count(WNode *n) const
{
    return (_count_type == COUNT_ADDRS ? n->count != 0 : n->count);
}

inline uint32_t
AggregateWTree::node_full_count(WNode *n) const
{
    return (n ? n->full_count : 0);
}

#endif
