#ifndef BTREE_MAIN_HEADER_
#define BTREE_MAIN_HEADER_

template<class KeyType> class Node_btree
{
public:
    KeyType key_array[(BLOCK_SIZE/(sizeof(KeyType))) + 1]; /** <This array contains all the primary keys which are stored in a node.
    The +1 while defining the size of the array is to allow a buffer space during addition so that we can add a key to the array, check if array needs to be split,
    and then take actions accordingly. The last space in the array is always empty during all operations, and split is called whenever it is filled. */
    int NumberOfValidKeys; /** <NumberOfValidKeys keeps track of how many valid entries are there in a node, and serves as an upper bound for iteration in many loops. */
    Node_btree* children_array[(BLOCK_SIZE/sizeof(KeyType)) + 1 + 1]; // Note that initially it was size of Node_btree* in denominator. additional +1 to be only utilised during add. to be empty otherwise.
    Node_btree* parent; // parent node of given node. NULL for root.
    // But that would make size of children_array and key_array very different, whereas children array is supposed to be key_array + 1.
    // Remember that arrays aren't being initialized to specific values
    Node_btree();
}

template<class KeyType> class BTree
{
public:
    Node_btree<KeyType> *root; // making this public for now.
    BTree();
    void setRoot(Node_btree<KeyType>* node);
    void move_keys_right(Node_btree<KeyType>* node, int i, int j); // copy data in node->keys_array one unit to right starting from ith index till jth index, both inclusive.
    // movement from [i,j] to [i+1,j+1];
    void move_children_right(Node_btree<KeyType>* node, int i, int j); // copy data in children array rightwards from [i,j] to [i+1,j+1]
    void move_keys_left(Node_btree<KeyType>* node, int i, int j); // copy data leftwards from index [i,j] to [i-1,j-1]
    KeyType* split(Node_btree<KeyType>* toSplit, Node_btree<KeyType>* extra);
    int find_position_to_insert(Node_btree<KeyType>* current, KeyType* toInsert) ;//returns index value for insertion
    void add_key_in_nonleaf_Node(Node_btree<KeyType>* current, KeyType* toInsert, Node_btree<KeyType>* right_child); // helper function for add_key
    void add_key_in_leaf_Node(Node_btree<KeyType>* current, KeyType *toInsert); //This function adds key to current node. Helper function for add_key.
    void add_key(KeyType* toInsert); // function to add node to the b+ tree
    void print_node(Node_btree<KeyType>* to_print); // print all keys in a node
    void print_tree(); // print the entire tree
    KeyType* search_key(KeyType* target) // since search equates primary keys, make sure that their valid bit is also set to 1.
    KeyType* search_helper(KeyType* target, Node_btree<KeyType>* current) // helper function for the primary search function
    void delete_key(KeyType* toDelete); // to implement

}


#endif // BTREE_MAIN_HEADER_
