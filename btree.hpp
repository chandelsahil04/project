# pragma once
#include "file.hpp"
using namespace std;

class BTreeNode{
    File *keys; // An array of files
    int t;// Minimum degree(defines the range for number of keys)
    BTreeNode **C;//An array of child pointers
    int n; //Current number of keys
    bool leaf;//is true when node is leaf. Otherwise false
public:
    BTreeNode(int _t,bool leaf);
    
    void insertNonFull(File);
    
    int findKey(File file);
    
    void remove(File file);
    
    void removeFromLeaf(int idx);
    
    void removeFromNonLeaf(int idx);
    
    File getPred(int idx);
    
    File getSucc(int idx);
    
    void fill(int idx);
    
    void borrowFromPrev(int idx);
    
    void borrowFromNext(int idx);
    
    void merge(int idx);
    
    void splitChild(int i, BTreeNode *y);

    void traverse();

    BTreeNode *search(File);

    friend class BTree;
};


class BTree{
    BTreeNode *root;  //Pointer to root node
    int t; //Minimum degree

public:
    BTree(int _t){root=NULL; t=_t;}
    
    BTree(){root=NULL; t=0;}
    BTreeNode * search(File k){
        return (root==NULL)?NULL :root->search(k);
    }
    void insert(File k); 
    void traverse() 
    {  if (root != NULL) root->traverse(); } 
    
    void remove(File file);
};

BTreeNode :: BTreeNode(int t1, bool leaf1){

    //copy the given minimum degree and leaf property
    t=t1;
    leaf=leaf1;

    //Allocate memory for maximum number of possible keys
    //and child pointers
    keys=new File[2*t-1];
    C=new BTreeNode *[2*t];

    //Initialise the number of fileas as 0
    n=0;
}

//A utility function that returns the index of the first file that is
//greater than or equal to keys
int BTreeNode:: findKey(File file){
    int idx=0;
    while(idx<n && keys[idx].file_size<file.file_size)++idx;
    return idx;
}


//A function to remove the key from the sub-tree rooted with this nodes
void BTreeNode :: remove(File file){
    int idx=findKey(file);
    
    //the key to be removed is present in this node
    if(idx<n && keys[idx].file_size==file.file_size && keys[idx].file_name==file.file_name){
        if(leaf) removeFromLeaf(idx);
        else removeFromNonLeaf(idx);
    }
    else{
        
        //if this node is a leaf node, then the key is not present in the tree
        if(leaf){
            cout<<"The file "<<file.file_name<<" does not exist in the tree.\n";
            return;
        }
        
        //The key to be removed is present in the sub-tree rooted with this nodes
        //The flag indicates whether the key is present in the sub-tree rooted
        //with the last child of this node
        bool flag=((idx==n)? true :false);
        
        //If the child where the key is supposed to exist has less than t keys,
        //we fill that child
        if(C[idx]->n <t) fill(idx);
        
        //If the last child has been merged , it must have merged with the previous 
        //child and so we recurse on the (idx-1)th child. ELse, we recurse on the 
        //(idx)th child which now has atleast t keys
        if(flag && idx>n) C[idx-1]->remove(file);
        else C[idx]->remove(file);
    }
    return;
}

void BTreeNode :: removeFromLeaf(int idx){
    
    //Move all the keys after the idx-th pos one place backward
    for(int i=idx+1;i<n;++i) keys[i-1]=keys[i];
    
    //reduce the count of files
    n--;
    
    return;
}

// A function to remove the idx-th key from this node -which is a non-leafnode
void BTreeNode :: removeFromNonLeaf(int idx){
    
    int k=keys[idx].file_size;
    File file=keys[idx];
    //If the child that precedes k (C[idx]) has atleast t keys,
    //find the predecessor 'pred' of k in the subtree rooted at
    //C[idx]. Replace k by pred. Recursively delete pred
    //in C[idx]
    if(C[idx]->n >=t){
        File pred=getPred(idx);
        keys[idx]=pred;
        C[idx]->remove(pred);
    }
    
    //If the child C[idx] has less than t keys , examine C[idx+1].
    //If C[idx+1] has atleast t keys ,find the successor 'succ' of k in
    //the subtree rooted at C[idx+1]
    else if(C[idx+1]->n >=t){
        File succ=getSucc(idx);
        keys[idx]=succ;
        C[idx+1]->remove(succ);
    }
    
    // If both C[idx] and C[idx+1] has less than t keys , merge k and all of  C[idx+1]
    //into C[idx]
    //Now C[idx] contains 2t-1 files
    //Free C[idx+1] and recursively delete k from C[idx]
    else{
        merge(idx);
        C[idx]->remove(file);
    }
    return;
}

// A function to get predecessor of keys[idx]
File BTreeNode ::getPred(int idx){
    
    //Keep moving to the right most node untill we reach a leaf
    BTreeNode * cur=C[idx];
    while(!cur->leaf) cur=cur->C[cur->n];
    
    //return the last key of the leaf
    return cur->keys[cur->n-1];
}

File BTreeNode ::getSucc(int idx){
    
    //keep moving the left most node starting from C[idx+1] until we reach a leaf
    BTreeNode *cur=C[idx+1];
    while(!cur->leaf) cur=cur->C[0];
    
    //return the first key of the leaf
    return cur->keys[0];
}

//A function  to fill child C[idx] which has less than t-1 keys
void BTreeNode :: fill(int idx){
    
    //if the previous child (C[idx-1]) has more than t-1 keys , borrow a key 
    //from that child
    if(idx!=0 && C[idx-1]->n >=t) borrowFromPrev(idx);
    
    //if the next child (C[idx+1]) has more than t-1 keys, borrow a keys
    //from that child 
    else if(idx!=n && C[idx+1]->n>=t) borrowFromNext(idx);
    
    //Merge C[idx] with its sibling
    //If C[idx] is the last child,merge it with its previos sibling
    //otherwise merge it with next sibling
    else{
        if(idx!=n) merge(idx);
        else merge(idx-1);
    }
    return;
}

//A function to borrow a key from C[idx-1] and insert it
//into C[idx]
void BTreeNode :: borrowFromPrev(int idx){
    BTreeNode *child=C[idx];
    BTreeNode *sibling=C[idx-1];
    
    //The last key from C[idx-1] goes upto the parent and key[idx-1]
    //from parent is inserted as the first key in C[idx].Thus ,the loses
    //sibling one key and child gains one key
    //moving all key in C[idx] one step ahead
    for(int i=child->n-1; i>=0 ;i--) child->keys[i+1]=child->keys[i];
    
    //if C[idx] is not a leaf, move all its child pointers one step ahead
    if(!child->leaf){
        for(int i=child->n ;i>=0;i--) child->C[i+1]=child->C[i];
    }
    child->keys[0]=keys[idx-1];
    if(!child->leaf) child->C[0]=sibling->C[sibling->n];
    keys[idx-1]=sibling->keys[sibling->n-1];
    
    child->n+=1;
    sibling->n-=1;
    
    return ;
}

//A function to borrow a key from the C[idx+1] and place 
//it in C[idx]
void BTreeNode ::borrowFromNext(int idx){
    BTreeNode *child=C[idx];
    BTreeNode *sibling=C[idx+1];
    
    //keys[idx] is inserted as the last child
    //into C[idx]
    if(!(child->leaf)) child->C[(child->n)+1]=sibling->C[0];
    
    //The first key from sibling is inserted into keys[idx]
    keys[idx]=sibling->keys[0];
    
    //moving all keys in sibling one step behind
    for(int i=1;i<sibling->n; i++) sibling->keys[i-1]=sibling->keys[i];
    
    
    //moving the child pointers one step behind
    if(!sibling->leaf){
        for(int i=1;i<=sibling->n;i++) sibling->C[i-1]=sibling->C[i];
    }
    
    //Increasing an decreasing the key count of C[idx] and C[idx+1]
    //respectively
    child->n+=1;
    sibling->n-=1;
    
    return;
}

//A fucntion to merge C[idx] with C[idx+1]
//C[idx+1] is freed after merging
void BTreeNode :: merge(int idx){
    BTreeNode *child=C[idx];
    BTreeNode *sibling=C[idx+1];
    
    //Pulling a key from the current node and inerting it into the (t-1)th 
    //position of C[idx]
    child->keys[t-1]=keys[idx];
    
    //Copying the keys from C[idx+1] to C[idx] at the end
    for(int i=0;i<sibling->n;i++){
        child->keys[i+t]=sibling->keys[i];
    }
    
    //copying the child pointers from C[idx+1] to C[idx]
    if(!child->leaf){
        for(int i=0;i<=sibling->n;i++) child->C[i+t]=sibling->C[i];
    }
    
    for (int i=idx+1; i<n; ++i) 
        keys[i-1] = keys[i]; 
  
    for (int i=idx+2; i<=n; ++i) 
        C[i-1] = C[i]; 
  
    // Updating the key count of child and the current node 
    child->n += sibling->n+1; 
    n--; 
  
    // Freeing the memory occupied by sibling 
    delete(sibling); 
    return; 
}


//Function to traverse all nodes in a subtree rooted with this node
void BTreeNode :: traverse(){
    
    //There are n fileas and n+1 children,traverse through n files
    // and first n children
    int i;
    for(int i=0;i<n;i++){

        if(leaf==false){
            C[i]->traverse();
        }
        
        cout<<" " << keys[i].file_name<<" "<<keys[i].file_size<<endl;
    }
        //Print the subtree rooted with last child
        if(leaf==false)
            C[i]->traverse();
}

//Function to search key k in subtree rooted with this node
BTreeNode * BTreeNode :: search(File file){
    //find the first file having size greater than or equal to k
    int i=0;
    while(i<n && file.file_size > keys[i].file_size) i++;

    int k=i;
    //if found key is same as file ,return this node 
    if(keys[i].file_name==file.file_name) return this;

    /*
    //if key is not found here then travere till the file has same size
    while(i<n && file.file_size==keys[i].file_size){
        if(file.file_name==keys[i].file_name)return this;
        i++;
    }
    int kk=i;

    int j=k;
    while(j<kk){
        if(C[i]->search(file)!=NULL)return C[i]->search(file);
        j++;
    }

    //if it is leaf node and the value of file size is less than the next file sizes
    if(leaf==true && j>=k) return NULL;
    */
    // If key is not found here and this is a leaf node 
    if (leaf == true) 
        return NULL; 
  
    // Go to the appropriate child 
    return C[i]->search(file); 
}

//The main function that inserts a new key in this B-Tree
void BTree :: insert(File file){
    //if tree is empty
    if(root==NULL){
        //Allocate memory for new root
        root=new BTreeNode(t,true);
        root->keys[0]=file; //insert file
        root->n=1;//update number of files in root
    }
    else{ //if tree is not empty
        
        //if root is full, then tree grows in height
        if(root->n==2*t-1){
            //Allocate memory for new root
            BTreeNode *s= new BTreeNode(t,false);

            //Make old root as child of new root
            s->C[0]=root;

            s->splitChild(0,root);
            int i=0;
            if(s->keys[0].file_size<file.file_size) i++;

            s->C[i]->insertNonFull(file);

            //change root
            root=s;
        }
        else{ //if root is not full, call insertNonFull for root
            root->insertNonFull(file);
        }
    }
}


void BTreeNode :: insertNonFull(File file){
    
    //Initialise index as index of rightmost element
    int i=n-1;

    //if this is lead node
    if(leaf==true){
        //the following loop does two things
        // a. find the location of new key to be inserted
        // b. moves all greater keys to one place ahead
        while(i>=0 && keys[i].file_size > file.file_size){
            keys[i+1]=keys[i];
            i--;
        }

        keys[i+1]=file;
        n=n+1;
    } 
    else{ // if this is not leaf node
        
        //find the child which is going to have the new key
        while(i>=0 && keys[i].file_size>file.file_size)i--;

        // see if the found child is full
        if(C[i+1]->n == 2*t-1){
            // if the child is full the split it
            splitChild(i+1,C[i+1]);
            if(keys[i+1].file_size<file.file_size)i++;
        }
        C[i+1]->insertNonFull(file);
    }
}

//A utility function to split the child y of this node
// note that y must be full when this function is called
void BTreeNode :: splitChild(int i, BTreeNode *y){

    //Create a new node which is going ot store (t-1) files/keys
    //of y
    BTreeNode *z=new BTreeNode(y->t,y->leaf);
    z->n=t-1;

    //copy the last (t-1) keys of y to z
    if(y->leaf == false)
        for(int j=0;j<t;j++)
            z->C[j]=y->C[j+t];
    
    //Reduce the number of files/keys in y
    y->n=t-1;

    //Since this node is going to have a new child,
    //create a space of new child 
    for(int j=n; j>=i+1; j--)
        C[j+1]=C[j];

    //Link the new child to this node
    C[i+1]=z;

    //A key of y will move to this node. Find the location of
    // new keys and move all greater keys one space ahead
    for(int j=n-1;j>=i;j--) keys[j+1]=keys[j];

    //copy the middle key of y to this node
    keys[i]=y->keys[t-1];

    //increment count of keys in this node
    n+=1;
}

void BTree::remove(File file) 
{ 
    if (!root) 
    { 
        cout << "The tree is empty\n"; 
        return; 
    } 
  
    // Call the remove function for root 
    root->remove(file); 
  
    // If the root node has 0 keys, make its first child as the new root 
    //  if it has a child, otherwise set root as NULL 
    if (root->n==0) 
    { 
        BTreeNode *tmp = root; 
        if (root->leaf) 
            root = NULL; 
        else
            root = root->C[0]; 
  
        // Free the old root 
        delete tmp; 
    } 
    return; 
} 
