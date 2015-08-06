/** @file btree.cc
    @brief Contains a modular implementation of a BTree.

    The BTree can use any sort of primary key, which allows us to use this btree for any data which has a total ordering defined on it. The data will be defined as a part of the struct
    primary_key. Node_Btree is the datastructure defined for every node of the Btree. We have tried keeping the size of each node close to the size of a page to ensure optimal utilisation
     of BTree.

    @author Utkarsh Sharma
    @bug No known bugs
*/

#include <iostream>
#include <cstring>
#include <vector>
#define N 5
#define BLOCK_SIZE 50 // CHANGE THIS LATER


using namespace std;


/// The structure of a primary key which constitutes an entry in a node of the BTree.
/** The primary key must contain a bool 'valid' and helper functions 'LT' (less than) and 'EQ' (equal to) to define the total ordering among the keys in the tree. Since we move
    a lot of the data around in a BTree, many times nearby memory locations will contain old values. Instead of always overwriting the entire primary key memory when it is removed,
    we just set the valid bit to 0, which results in it being treated as an empty space by the BTree.

*/
struct primary_key
{
    bool valid; /// The part of primary key which checks whether the data in key is valid. Set to false by default (through constructor).
    int cust_id;
    int w_id;
    int d_id;
    bool LT(primary_key a) // A LT function on this scheme will be present in every primary key. If cust_id of calling key < called key then true
    {
        if (a.valid == 0)
            cout << "COMPARISION WITH INVALID KEY. THIS SHOULDN'T BE PRINTED. GARBAGE KEY VALUE " << a.cust_id;
        if (cust_id < a.cust_id)
            return true;
        return false;
    }

    bool EQ(primary_key a) // A equal to function on this scheme will be present in every primary key. If cust_id of calling key = called key then true
    {
        if (a.valid == 0)
            cout << "COMPARISION WITH INVALID KEY. THIS SHOULDN'T BE PRINTED. GARBAGE KEY VALUE " << a.cust_id;
        if (cust_id == a.cust_id)
            return true;
        return false;
    }
  /*  void operator=(const primary_key &other) // an overload for = will have to be defined in primary key class.
    {
        this->cust_id = other.cust_id;
        this->w_id = other.w_id;
        this->d_id = other.d_id;
        return;
    }  */
    void equateTo(primary_key a)
    {
        cout << "equateTo is called  ";
        valid = a.valid;
        cust_id = a.cust_id;
        w_id = a.w_id;
        d_id = a.d_id;
        cout << "equateTo returns." << endl;
        return;
    }
    primary_key() // constructor for the primary key
    {
        valid = false;
    }
};


/// A template class defining a node of the BTree. Each node will contain multiple primary keys and be linked to other nodes to form the BTree.
/** The node of a BTree is a template class which is meant to be initialised with the primary key struct as the type. This allows the node to allocate memory in such a manner
    that the size of key_array (it stores all the primary keys) in each node are nearly equal to that of a page. This will ensure that the entire array is read in one disk access,
    and also that the maximum possible length of the array is used.
*/
template <class KeyType> class Node_btree
{
public:
    KeyType key_array[(BLOCK_SIZE/(sizeof(KeyType))) + 1]; /**< This array contains all the primary keys which are stored in a node.
    The +1 while defining the size of the array is to allow a buffer space during addition so that we can add a key to the array, check if array needs to be split,
    and then take actions accordingly. The last space in the array is always empty during all operations, and split is called whenever it is filled. */

    int NumberOfValidKeys; /**< NumberOfValidKeys keeps track of how many valid entries are there in a node, and serves as an upper bound for iteration in many loops. */

    Node_btree* children_array[(BLOCK_SIZE/sizeof(KeyType)) + 1 + 1]; /**< This array stores all the pointers to the children nodes of the current node. Note that the size of this array
    is +1 than that of the key array because there is one chiild more than the number of keys in a BTree. The other +1 is for a temporary space which will be utilised during the insert
    function if the corresponding node is fully filled. Note that the last node will always be empty before a function is called or after a function returns */

    Node_btree* parent; /**< Pointer to the parent node of the current node. It is set as NULL for the root node. */

    /** Constructor for the Node_btree class. It sets the valid bits in the key array to zero and initialises the children array to 0, sets NumberOfValidKeys to zero
    and also sets the parent of the node to NULL. */
    Node_btree()
    {
        for (int i = 0; i < BLOCK_SIZE/sizeof(KeyType) + 1; i++)
        {
            key_array[i].valid = 0;
            children_array[i] = 0;
        }
        children_array[(BLOCK_SIZE/sizeof(KeyType)) + 1] = 0;
        parent = NULL;
        NumberOfValidKeys = 0;
    }
};


/// A template class which implements the BTree. The template depends on the primary key being used.
/** This class implements all the functionalities of the BTree. The main functions of the BTree are insert, search and delete. A print function is also included to
see the BTree at any point of time for human verification of any aspect. Every node of the BTree is of the type Node_btree, with the keys in every node as the template type. */
template <class KeyType> class BTree
{
private:
    Node_btree<KeyType> *root; /**< Pointer to the root node of the BTree. */
public:

    /** Constructor for the BTree. It sets the root pointer of the tree as NULL */
    BTree()
    {
        cout << "TREE CONSTRUCTOR";
        root = NULL;
    }

    /// This function sets the value of the root equal to the input parameter
    /** This function was created to facilitate access to the root when it was made private. It isn't being used currently due to change in implementation ideology midway but is
    still kept here. */
    void setRoot(Node_btree<KeyType>* node)
    {
        *root = *node;
        return;
    } // check this function also.

    /// Function to shift all the keys left in the array from index i to j (both inclusive).
    /** This function is called whenever a key has to be inserted into an array. Since arrays are contigous in memory, you can't insert between two elements. So this function shifts
    all elements from point of insertion to the right so as to create space for one more element to be added. Works by copying elements one by one to the right.
    @param node The node whose key array elements are being shifted.
    @param i The left limit (inclusive) in the key array of the elements being shifted.
    @param j The right limit (inclusive) in the key array of the elements being shifted. */
    void move_keys_right(Node_btree<KeyType>* node, int i, int j)
    {
        if (node->NumberOfValidKeys == BLOCK_SIZE/sizeof(KeyType))
            cout << "Buffer space being utilised. This should only print under adding to node function when node is full." << endl;
        for (int counter = j; counter >= i; counter--)
        {
            node->key_array[counter+1] = node->key_array[counter];
        }
        cout << "right movement of keys successfully completed." << endl;
        return;
    }

    /// Function to shift the children pointers to the right from index i to j (both inclusive)
    /** This function shifts the children to the right to facilitate insertion into the child array. Works by copying data sequentially to the right.Called along with move_keys_right
    function as both of these operations are needed while inserting keys in the middle of the array.
    @param node The node whose children array keys have to be shifted.
    @param i The left limit (inclusive) of the keys being shifted
    @param j The right limit (inclusive) of the keys being shifted */
    void move_children_right(Node_btree<KeyType>* node, int i, int j)
    {
        if (node->NumberOfValidKeys == BLOCK_SIZE/sizeof(KeyType))
            cout << "Buffer space being utilised. This should only print under adding to node function when node is full." <<endl;
        for (int counter = j; counter >= i; counter--)
        {
            node->children_array[counter+1] = node->children_array[counter];
        }
        cout << "right movement of children array complete" << endl;
    }

/// Function to shift all the keys left in the array from index i to j (both inclusive).
    /** This function is used to shift all the elements stored in an array towards the left by one unit. It was important in the original implementation idea, but wasn't used when
    the plan was changed later on. Included as a legacy function. Works by copying elements one by one to the left.
    @param node The node whose key array elements are being shifted.
    @param i The left limit (inclusive) in the key array of the elements being shifted.
    @param j The right limit (inclusive) in the key array of the elements being shifted. */
    void move_keys_left(Node_btree<KeyType>* node, int i, int j)
    {
        if (i == 0)
            cout << "SEGMENTATION FAULT DURING MOVE LEFT AS LOWER LIMIT IS ZERO." << endl;
        for (int counter = i; counter <= j; counter++)
            node->key_array[counter-1] = node->key_array[counter];
        cout << "left movement of keys successfully completed" << endl;
        return;
    }

/// Function to split the given node into two nodes.
    /**  This function is called when a node has crossed maximum capacity after insertion. It splits at the ((BLOCK_SIZE/sizeof(KeyType)) + 1)/2 index. It makes a copy of the key
    at the breakpoint, and then moves all the keys on the right of the breakpoint to a different node and sets the valid bit of those keys as zero in the current node.
    It also sets the valid bit of the key at breakpoint to zero.
    @param toSplit A pointer to the node which needs to be split.
    @param extra A pointer to an additional node in which the right half to the toSplit node will be transferred.
    @return A pointer to the a key which has to be inserted into a level above the current node.
    */
    KeyType* split(Node_btree<KeyType>* toSplit, Node_btree<KeyType>* extra)
    {
        int break_point = (BLOCK_SIZE/sizeof(KeyType) + 1)/2;
        KeyType* toReturn = new KeyType;
        cout << "memcpy called";
        memcpy(toReturn, &toSplit->key_array[break_point - 1], sizeof(KeyType));
        cout << "memcpy successful." << endl;
        cout << "break point for split is " << break_point << ". Key at break point is " << toReturn->cust_id << endl;
        // toReturn cant be kept as pointer to that element of toSplit because the element at the pointer location itself is made zero later on.
        for (int i = break_point; i <= BLOCK_SIZE/sizeof(KeyType); i++)
        {
            extra->key_array[i - break_point] = toSplit->key_array[i];
            extra->children_array[i - break_point] = toSplit->children_array[i];
            if (extra->children_array[i - break_point] != NULL)
                extra->children_array[i - break_point]->parent = extra;
            // this is done because right now all children point to first key in node, and splitting this node gives children a wrong pointer of parent.
            // example:          15     70      77      121
            //               a        b     c       d       e
            // if top node gets split, without this command, the parent of e would still point to node containing 15.
            extra->NumberOfValidKeys++;
            toSplit->key_array[i].valid = 0;
            toSplit->children_array[i] = 0;
            toSplit->NumberOfValidKeys--;
        }
        extra->children_array[BLOCK_SIZE/sizeof(KeyType) - break_point + 1] = toSplit->children_array[BLOCK_SIZE/sizeof(KeyType) + 1]; // last element of child array is not covered in the loop
        if (extra->children_array[BLOCK_SIZE/sizeof(KeyType) - break_point + 1] != NULL)
            extra->children_array[BLOCK_SIZE/sizeof(KeyType) - break_point + 1]->parent = extra; // setting parent correct for last element of child array
        cout << "the index of child array being made zero is " << BLOCK_SIZE/sizeof(KeyType) - break_point + 1 << endl;
        toSplit->children_array[BLOCK_SIZE/sizeof(KeyType) - break_point + 1] = 0;
        toSplit->key_array[break_point - 1].valid = 0;
        toSplit->NumberOfValidKeys--;
        cout << "AFTER SPLIT: first key of toSplit is " << toSplit->key_array[0].cust_id << endl;
        cout << "AFTER SPLIT: last key of toSplit " << toSplit->key_array[BLOCK_SIZE/sizeof(KeyType) - break_point].cust_id << endl;
        cout << "AFTER SPLIT: last key of toSplit " << toSplit->key_array[BLOCK_SIZE/sizeof(KeyType) - break_point].valid << endl;
        cout << "AFTER SPLIT: first key to right created node " << extra->key_array[0].cust_id << endl;
        cout << "AFTER SPLIT: last key of right created node " << extra->key_array[BLOCK_SIZE/sizeof(KeyType) - break_point].cust_id << endl;
        return toReturn;
    }

/// Function to find the position at which a key should be inserted.
    /** It does so by searching through the key array of the current node and finding the position where the key to be
    inserted is lexicographically greater than the previous key but less than the current key. If the key to be inserted is larger than all keys, then the next available position in the
    key array is returned.
    @param current  The node of the btree in which the key position is being searched.
    @param toInsert The key which has to be inserted into the current node.
    @return The index at which the key should be inserted.
    */
    int find_position_to_insert(Node_btree<KeyType>* current, KeyType* toInsert) //returns index value for insertion
    {
        int position_to_insert;
        int larger_than_all = 1;
        for (position_to_insert = 0; position_to_insert < current->NumberOfValidKeys; position_to_insert++)
            {
                cout << "FPIN. value of toInsert is " << toInsert->cust_id << endl;
                cout << "current position to insert (checking at) is " << position_to_insert << endl;
                cout <<"Value of key being checked with is " << current->key_array[position_to_insert].cust_id;
                cout << " . Output of LT function is " << toInsert->LT(current->key_array[position_to_insert]) << endl << endl;
                if ((toInsert->LT(current->key_array[position_to_insert]) == true) && (current->key_array[position_to_insert].valid == 1))
                {
                    cout << "value of i where condition satisfied is " << position_to_insert << endl;
                   // move_keys_right(current, i, current->NumberOfValidKeys - 1); This line also makes position to insert. Not to be done as of now.
                    larger_than_all = 0;
                    break;
                }
            }
        if (larger_than_all == 1)
            position_to_insert = current->NumberOfValidKeys;
        return position_to_insert;
    }

/// Function to add a key in a node of the BTree.
/** It adds a key to a specified node in the BTree. It also adds the corresponding right child array if the insertion is being done in a nonleaf node.
    It does so by first finding the position to insert the key, and inserts it there. Also if the right_child parameter is not NULL, which implies there is a child node addition
    needed to be done, the required space is made in the child array by shifting the keys to the right and adding the corresponding child node pointer.
    After this it checks whether the node is completely filled or not. If not, the function returns, else the node in which the insertion has just been done has to be split.
    An additional node is created into which along with the current node is sent into the split function. This results in the additional node containing the right half of the node to be
    split, and the original node reduced to half its size. The split function also returns a pointer to the key which will be subsequently inserted into the parent node.
    After this it checks whether the current node is the root node or not. If not, the add function is called for the parent node with the pointer to the additionally created
    node being sent up as the the new child node pointer along with the key returned from the split function. If the current node is the root, then a new root is defined
    and the insertion of key returned from split is done in the newly defined root and the two nodes, i.e. the one which was split, and the one into which the data was moved to, are
    defined as the new children of the root.
    @param current  Pointer to the node in which the key has to be added.
    @param toInsert Pointer to the key to be added.
    @param right_child  Pointer to the node which has to be added as the right child after key in added. Its default value is NULL, which will be used when the insertion is being done
                        in a leaf node. */
    void add_key_in_node(Node_btree<KeyType>* current, KeyType* toInsert, Node_btree<KeyType>* right_child = NULL)
    {
        cout << "ADD KEY IN NODE" << endl;
        int position_to_insert = find_position_to_insert(current, toInsert);
        if (position_to_insert == BLOCK_SIZE/sizeof(KeyType))
        {
            cout << "Node is full. Adding to it using buffer.";
        }
        move_keys_right(current, position_to_insert, current->NumberOfValidKeys - 1);
        //if (current->children_array[position_to_insert+1] != NULL)
         //   cout << "The value of 1st key of 1st child being shifted right is " << current->children_array[position_to_insert+1]->key_array[0].cust_id << endl;
        if (right_child != NULL)
        {
            move_children_right(current, position_to_insert + 1, current->NumberOfValidKeys); // the children array will be shifted right from position to insert + 1 because the node being added contains keys larger than key being added.
            current->children_array[position_to_insert+1] = right_child;
        }
        current->key_array[position_to_insert] = *toInsert;
        current->NumberOfValidKeys++;
        cout << "Key that was added was " << toInsert->cust_id << "at position " << position_to_insert << endl;
        if (right_child != NULL)
        {
            cout << "value of 1st key of less than child " << current->children_array[position_to_insert]->key_array[0].cust_id << endl;
            cout << "value of 1st key of more than child " << current->children_array[position_to_insert+1]->key_array[0].cust_id << endl;
            if (current->children_array[position_to_insert+2] != NULL)
                cout << "value of 1st key of after more than child " << current->children_array[position_to_insert+2]->key_array[0].cust_id << endl;
        }
        if (current->NumberOfValidKeys == BLOCK_SIZE/sizeof(KeyType) + 1) // this checks whether the node has to be split after insertion.
        {
            cout << "Node buffer has been used. will have to split node." << endl;
            Node_btree<KeyType>* right_created_node = new Node_btree<KeyType>;
            KeyType* splitReturned = split (current, right_created_node);
            cout << "split just returned. key being sent up is " << splitReturned->cust_id << endl;
            cout << "first element of right created node is " << right_created_node->key_array[0].cust_id << endl;
            if (current->parent != NULL)
            {
                cout << "First of key of node whose parent is being assigned to cousin " << current->key_array[0].cust_id << endl;
                current = current->parent;
                right_created_node->parent = current;
                cout << "Parent being defined. value of first key of child is " << right_created_node->key_array[0].cust_id << endl;
                cout << "Parent being defined. value of first key of parent is " << current->key_array[0].cust_id << endl;
                cout << "adding in parent now. First key of parent is " << current->key_array[0].cust_id << endl;
                add_key_in_node(current, splitReturned, right_created_node);
                return;
            }
            else
            {
                Node_btree<KeyType>* fresh_node = new Node_btree<KeyType>;
                root = fresh_node;
                current->parent = fresh_node;
                right_created_node->parent = fresh_node;
                fresh_node->children_array[0] = current;
                cout << "NEW ROOT being defined!!!!" << endl;
                add_key_in_node(fresh_node, splitReturned, right_created_node);
                return;
                // If parent is null, define new node as parent and make that root.
            }
        }
        return;
    }


/// Function to add a key in the BTree.
/** This function is called when a key has to be inserted in the BTree. The function starts at the root, and traverses the tree finding a suitable place to insert. We know that an
    addition has to be done in the leaf node. So it traverses the tree to the corresponding leaf node by comparing values with the keys in a node, and moving to a child node
    when the value is larger than the one at the previous index but smaller than the one at the current index. If it is larger than all values in the node, then the function traverses
    to the last valid entry of the children array and continues the search from there. When it finds the leaf node in which the insertion has to be done, the add_key_in_node function
    is called with the required parameters.
    @param toInsert The key which has to be inserted into the BTree. */
    void add_key(KeyType* toInsert) // function to add node to the b+ tree
    {
        cout << "addddd key called for key with value with " << toInsert->cust_id << endl;
        Node_btree<KeyType>* current = root;
        cout << "eh";
        int counter = 0;
        if (root == NULL)
        {
            root = new Node_btree<KeyType>;
            root->key_array[0] = *toInsert;
            root->NumberOfValidKeys = 1;
            root->parent = NULL;
            return;
        }
        while (current->children_array[0] != 0) // keys are always added in leaf node. This condition checks if node is leaf node or not. Constructor of Node_BTree initialises the array to 0 instead of NULL.
        {
            cout << "f";
        //    for (int i = 0; i <= BLOCK_SIZE/sizeof(KeyType); i++) //less than equal to for 'i' so that we can insert in node's last children_array element.
            for (int i = 0; i <= current->NumberOfValidKeys; i++)
            {
          //      if (i == BLOCK_SIZE/sizeof(KeyType)) // This condition added if node has to be inserted in last element of children nodes.
                if (i == current->NumberOfValidKeys)
                {
                    current = current->children_array[i];
                    break;
                }
                if (toInsert->LT(current->key_array[i]))
                {
                    current = current->children_array[i];
                    break;
                }
            }
        }
        // at this point, current should be the btree leaf node wherein the key has to be inserted.
        cout << "!!!!!!!!!!!!!add_key in leaf node now calling for key value " << toInsert->cust_id << endl;
        add_key_in_node(current, toInsert);
        cout << "SAFE" << endl;
        return;
    }

/// Function to print the subtree with the node in the parameter as its root.
/** It prints all the elements contained in the node and then calls the function recursively to all children. This results in a preorder printing of the tree nodes.
    @param  The node whose subtree (node included) has to be printed. */
    void print_subtree(Node_btree<KeyType>* to_print)
    {
     //   cout << "first key of node being printed is " << to_print->key_array[0].cust_id << " with Number of valid keys as " << to_print->NumberOfValidKeys << endl;
        for (int i = 0; i < to_print->NumberOfValidKeys; i++)
        {
   //         cout << "printing node. Valuue of i is " << i << "   ";
            if (to_print->key_array[i].valid == 1)
                cout << to_print->key_array[i].cust_id << "  " ;
        }
        cout << endl;
        for (int i = 0; i <= to_print->NumberOfValidKeys; i++)
        {
            if (to_print->children_array[i] == NULL)
                break;
            print_subtree(to_print->children_array[i]);
        }
        return;
    }

/// Function to print the entire BTree.
/** It calls the print_subtree function with the BTree root as the parameter. */
    void print_tree()
    {
        Node_btree<KeyType>* start = root;
        print_subtree(start);
        cout << endl;
        return;
    }

/// Function to search for a particular key in the BTree.
/** It is a wrapper for the search_helper function. One important point is that search works by checking equality of keys. So, set the valid bit of the key being sent as the target
    in the function to 1.
    @param target   The key which is being searched for.
    @return Pointer to the key being searched for. */
    KeyType* search_key(KeyType* target)
    {
        Node_btree<KeyType>* current = root;
        return search_helper(target, root);
    }

/// Utility function to search for a key in the BTree.
/** This function searchs in the BTree by checking with the values in the current node and traversing to the child node whenever the value of the target lies between two values in
    the current node. If it is larger than all values in the node, it moves down through the last child pointer. It is essentially a BFS search for the target.
    @param target   Key which is being looked for in the BTree.
    @param current  Pointer to the node in the BTree which is currently being traversed.
    @return Pointer to the key being searched. */
    KeyType* search_helper(KeyType* target, Node_btree<KeyType>* current)
    {
        if (current == NULL)
            cout << "ERROR 404. Primary Key not found. Crash imminent." << endl;
        for (int i = 0; i < current->NumberOfValidKeys; i++)
        {
            if (target->LT(current->key_array[i]) == 1)
                return search_helper(target, current->children_array[i]);
            else if (target->EQ(current->key_array[i]))
                return &(current->key_array[i]);
        }
        if (current->children_array[current->NumberOfValidKeys] != NULL)
            return search_helper(target, current->children_array[current->NumberOfValidKeys]);
        else
            cout << "ERROR 404. Primary Key not found." << endl;
        return 0;
    }



/// Function to search for a all keys in the BTree which fulfill a certain criteria. The criteria is decided by the function pointer passed as argument.
/** This is a wrapper for the linear_search_helper function. It works by searching the entire tree, and adding the address of any key which returns a true in the function whose pointer
    is passed as a parameter. The second argument of the function keeps changing as we traverse the key, whereas the first argument, which will be KeyType target, is the fixed key
    relative to whom we want to evaluate all the other keys. The function is called linear_search because it is a O(n) search, as apposed to the O(mlogn) search you can do it a btree.
    @param target   The key relative to whom other keys are evaluated in the compare function.
    @param *compare Pointer to the function which will make the comparision between keys. It should return a bool value.
    @return vector<void*> which contains pointers to all the keys which were evaluated to true in the compare function. Proper care must be taken while dereferencing, as addresses
            will first have to be cast to KeyType pointers.*/
    vector<void*> linear_search(KeyType* target, bool (*compare)(KeyType* a, KeyType* b)) // NOTE: If output is 1, b is added to output list.
    {
        // Implement the linear search funciton. Add to array whenever compare function returns value one,
        vector<void*> ans;
//	memset(ans, 0, sizeof(&ans));
        Node_btree<KeyType> current = (*root);
        ans = *(linear_search_helper(&current, &ans, target, compare));
        return ans;
    }


/// Helper function to search for a all keys in the BTree which fulfill a certain criteria. The criteria is decided by the function pointer passed as argument.
/** The function works by traversing the entire tree by first evaluating all keys in a node and then recursively calling the function to all children node. It is essentially similar
    to the print tree function, except that instead of printing the output, it evaluates it using the compare function and adds address  of key to vector<void*> if the function is true.
    @param current  The node in the btree in which the function should be searching.
    @param ans      A vector<void*> which contains all the addresses of all keys that have evaluated to true till now.
    @param target   The key relative to whom other keys are evaluated in the compare function.
    @param *compare Pointer to the function which will make the comparision between keys. It should return a bool value.
    @return vector<void*> which contains pointers to all the keys which were evaluated to true in the compare function. Proper care must be taken while dereferencing later on,
            as addresses will first have to be cast to KeyType pointers.*/
    vector<void*>* linear_search_helper(Node_btree<KeyType>* current, vector<void*>* ans, KeyType* target, bool (*compare)(KeyType* a, KeyType* b))
    {
        cout << "running another loop." << endl;
        cout << "first key of current node is " << current->key_array[0].cust_id << endl;
        for (int i = 0; i < current->NumberOfValidKeys; i++)
        {
            cout << "current cust_id " << current->key_array[i].cust_id << endl;
            cout << "target " << target->cust_id << endl;
            if ((*compare)(target, &(current->key_array[i])) == 1)
            {
                cout << "pushing back to vector ADRESS OF !!!!!!!!!!!! " << current->key_array[i].cust_id <<  endl;
                ans->push_back(&current->key_array[i]);
            }
        }
        cout << "out of the for loop" << endl;
        for (int i = 0; i <= current->NumberOfValidKeys; i++)
        {
            if (current->children_array[i] == NULL)
                break;
            ans = linear_search_helper(current->children_array[i], ans, target, compare);
        }
        return ans;
    }



/// Function to remove a key from the BTree.
    void delete_key(KeyType *toDelete)
    {
        // Implement the function. TPCC works without it also. No delete just results in some memory leakage. In current scope of project, that much leakage is acceptable.
        return;
    }


};

bool EQdummy_for_ls(primary_key* a, primary_key* b)
    {
        cout << "cust id of input a is " << a->cust_id << endl;
        cout << "cust id of input b is " << b->cust_id << endl;
        if (b->cust_id == a->cust_id)
            {
                return true;
            }
        return false;
    }

int main()
{
    cout << "a";
    BTree<primary_key> tree;
    //Node_btree<primary_key> node;
    cout << "b";
    primary_key test_key;
    test_key.cust_id = 1331;
    test_key.valid = true;
    test_key.w_id = 2;
    test_key.d_id = 3;
//    node.key_array[0] = test_key;
    cout << "c";
    tree.add_key(&test_key);
    cout << "d";
    primary_key test2_key;
    test2_key.cust_id = 121;
    test2_key.valid = true;
    test2_key.w_id = 14;
    test2_key.d_id = 11;
    tree.add_key(&test2_key);
    primary_key test3_key;
    test3_key.cust_id = 14641;
    test3_key.valid = true;
    test3_key.w_id = 141;
    test3_key.d_id = 111;
    tree.add_key(&test3_key);
    cout << "Printing after inserting three nodes" << endl;
    tree.print_tree();
    cout << endl;
    primary_key test_key4;
    test_key4.cust_id = 84;
    test_key4.valid = true;
    test_key4.w_id = 2;
    test_key4.d_id = 3;
    tree.add_key(&test_key4);
    test_key4.cust_id = 12;
    tree.add_key(&test_key4);
    test_key4.cust_id = 65;
    tree.add_key(&test_key4);
    test_key4.cust_id = 15;
    tree.add_key(&test_key4);
    test_key4.cust_id = 11;
    tree.add_key(&test_key4);
    test_key4.cust_id = 4;
    tree.add_key(&test_key4);
    test_key4.cust_id = 70;
    tree.add_key(&test_key4);
    test_key4.cust_id = 75;
    tree.add_key(&test_key4);
    test_key4.cust_id = 77;
    tree.add_key(&test_key4);
    test_key4.cust_id = 79;
    tree.add_key(&test_key4);
    test_key4.cust_id = 71;
    tree.add_key(&test_key4);
    test_key4.cust_id = 79284;
    tree.add_key(&test_key4);
    test_key4.cust_id = 9284;
    tree.add_key(&test_key4);
    cout << endl;
    cout << endl;
    cout << endl;
    tree.print_tree();
    cout << BLOCK_SIZE/sizeof(primary_key);
    cout << endl;
    cout << endl;
    test_key4.cust_id = 284;
    tree.add_key(&test_key4);
    test_key4.cust_id = 84;
    tree.add_key(&test_key4);
    test_key4.cust_id = 81;
    tree.add_key(&test_key4);
    test_key4.cust_id = 81;
    tree.add_key(&test_key4);
    test_key4.cust_id = 83;
    tree.add_key(&test_key4);
    test_key4.cust_id = 84;
    tree.add_key(&test_key4);
    test_key4.cust_id = 82;
    tree.add_key(&test_key4);
    test_key4.cust_id = 8;
    tree.add_key(&test_key4);
    test_key4.cust_id = 84;
    tree.add_key(&test_key4);
    test_key4.cust_id = 1214;
    tree.add_key(&test_key4);
    test_key4.cust_id = 115;
    tree.add_key(&test_key4);
    test_key4.cust_id = 1;
    tree.add_key(&test_key4);
    test_key4.cust_id = 654;
    test_key4.w_id = 112;
    tree.add_key(&test_key4);
    // To see the values in print statements below, make root as a public variable instead of private
    // cout << "Tree root node 0th element " << tree.root->key_array[0].cust_id << endl;
    // cout << "Tree root child number 0. 0th element " << tree.root->children_array[0]->key_array[0].valid << endl;
    // cout << "Tree root child number 0. 0th element " << tree.root->children_array[0]->key_array[0].cust_id << endl;
    // cout << "Tree root child number 0. 1th element " << tree.root->children_array[0]->key_array[1].valid << endl;
    // cout << "Tree root child number 0. 1th element " << tree.root->children_array[0]->key_array[1].cust_id << endl;
    // cout << "Tree root child number 1. 0th element " << tree.root->children_array[1]->key_array[0].valid << endl;
    // cout << "Tree root child number 1. 0th element " << tree.root->children_array[1]->key_array[0].cust_id << endl;
    // cout << "Tree root child number 1. 1th element " << tree.root->children_array[1]->key_array[1].valid << endl;
    // cout << "Tree root child number 1. 1th element " << tree.root->children_array[1]->key_array[1].cust_id << endl;
    cout << endl;
    cout << endl;
    cout << "calling print tree again:" << endl;
    tree.print_tree();
    cout << endl;
    test3_key.cust_id = 14641;
    cout << "Time to see if it can search. Looking for " << test3_key.cust_id << endl;
    cout << endl;
    primary_key* dhoond = tree.search_key(&test3_key);
    test_key4.w_id = 999;
    cout << "Searched keys cust_id is " << dhoond->cust_id << endl;
    cout << "Searched keys w_id is " << dhoond->w_id << endl;
 //   cout << endl << "time to check linear_search for exact match with " << test3_key.cust_id << endl;
 //   vector<void*> return_of_ls;
 //   return_of_ls = tree.linear_search(&test3_key, &EQdummy_for_ls);
 //   cout << endl << "first element of void pointer vector returned is " << ((primary_key*)(((return_of_ls)[0])))->cust_id << endl;
    cout << "Yay, it still runs";
    return 0;
}
