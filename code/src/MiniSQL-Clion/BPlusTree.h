//
//  BPlustree.h
//  minisql_debug
//
//  Created by 张倬豪 on 2017/6/7.
//  Copyright © 2017年 Icarus. All rights reserved.
//

#ifndef BPlustree_h
#define BPlustree_h

#include <vector>
#include <stdio.h>
#include <string.h>
#include "BufferManager.h"
#include "DataStruct.h"
#include <string>

using namespace std;

//auto &bm = BufferManager::instance();

typedef uint32_t keySize;
typedef int offsetNumber;
const int BINARY_BOND = 20;

template <typename keyType>
class treeNode {
public:
    keySize key_number;
    treeNode* parent;
    vector <keyType> keys;
    vector <treeNode*> children;
    vector <offsetNumber> values;
    treeNode* next_leaf;
    bool is_leaf;
    
    treeNode(int degree, bool new_leaf = false);
    ~treeNode();
    
    bool isRoot();
    bool search(keyType key, keySize &index);
    treeNode* splite(keyType &key);
    keySize add(keyType &key);
    keySize add(keyType &key, offsetNumber value);
    bool removeFrom(keySize index);
    
private:
    int degree;
};

template <typename keyType>
class BPlusTree {
private:
    typedef treeNode<keyType>* Node;
    
    // a struct helping to find the node containing a specific key
    struct searchNodeParse {
        Node pNode; // a pointer pointering to the node containing the key
        uint32_t index; // the position of the key
        bool ifFound; // the flag that whether the key is found.
    };
private:
    string fileName;
    Node root;
    Node leafHead; // the head of the leaf node
    uint32_t keyCount;
    uint32_t level;
    uint32_t nodeCount;
    int key_size; // the size of key
    int degree;
    
public:
    BPlusTree(string m_name,int keySize,int degree);
    ~BPlusTree();
    
    offsetNumber search(keyType& key); // search the value of specific key
    bool insertKey(keyType &key,offsetNumber val);
    bool deleteKey(keyType &key);
    
    void dropTree(Node node);
    
    void readFromDiskAll();
    void writeBackToDiskAll();
    void readFromDisk(Block* btmp);
    
private:
    void init_tree();// init the tree
    bool adjustInsert(Node pNode);
    bool adjustAfterDelete(Node pNode);
    void findToLeaf(Node pNode, keyType key,searchNodeParse &snp);
};

template <class keyType>
treeNode<keyType>::treeNode(int m_degree,bool new_leaf):key_number(0),parent(NULL),next_leaf(NULL),is_leaf(new_leaf),degree(m_degree) {
    for(keySize i = 0; i < degree + 1;i ++) {
        children.push_back(NULL);
        keys.push_back(keyType());
        values.push_back(offsetNumber());
    }
    children.push_back(NULL);
}

template <class keyType>
bool treeNode<keyType>::isRoot() {
    return parent == NULL;
}

template <class keyType>
bool treeNode<keyType>::search(keyType key,keySize &index) {
    if(key_number == 0) {
        index = 0;
        return false;
    }
    else {
        if(keys[key_number - 1] < key) {
            index = key_number;
            return false;
        }
        else if(keys[0] > key) {
            index = 0;
            return false;
        } // end of test
        else if(key_number <= BINARY_BOND) {
            for(keySize i = 0;i < key_number;i ++) {
                if(keys[i] == key) {
                    index = i;
                    return true;
                }
                else if(keys[i] < key) {
                    continue;
                }
                else if(keys[i] > key) {
                    index = i;
                    return false;
                }
            }
        }
        else if(key_number > BINARY_BOND) {
            keySize left = 0, right = key_number - 1, mid = 0;
            while(right > left + 1) {
                mid = (right + left) / 2;
                if(keys[mid] == key) {
                    index = mid;
                    return true;
                }
                else if(keys[mid] < key) {
                    left = mid;
                }
                else if(keys[mid] > key) {
                    right = mid;
                }
            }
            
            if(keys[left] >= key) {
                index = left;
                return (keys[left] == key);
            }
            else if(keys[right] >= key) {
                index = right;
                return (keys[right] == key);
            }
            else if(keys[right] < key) {
                index = right ++;
                return false;
            }
        }
    }
    return false;
}

template <class keyType>
treeNode<keyType>* treeNode<keyType>::splite(keyType &key) {
    
    keySize minmumNode = (degree - 1) / 2;
    
    treeNode* newNode = new treeNode(degree, this->is_leaf);

    if(is_leaf) { // this is a leaf node
        key = keys[minmumNode + 1];
        for(keySize i = minmumNode + 1;i < degree;i ++) {// copy the right hand of the keys to the new node
            
            newNode->keys[i-minmumNode-1] = keys[i];
            keys[i] = keyType();
            newNode->values[i-minmumNode-1] = values[i];
            values[i] = offsetNumber();
        }
        newNode->next_leaf = this->next_leaf;
        this->next_leaf = newNode;
        
        newNode->parent = this->parent;
        newNode->key_number = minmumNode;
        this->key_number = minmumNode + 1;
    }
    else {
        key = keys[minmumNode];
        for(keySize i = minmumNode + 1;i < degree+1;i ++) {
            newNode->children[i-minmumNode-1] = this->children[i];
            newNode->children[i-minmumNode-1]->parent = newNode;
            this->children[i] = NULL;
        }
        for(keySize i = minmumNode + 1;i < degree;i ++) {
            newNode->keys[i-minmumNode-1] = this->keys[i];
            this->keys[i] = keyType();
        }
        this->keys[minmumNode] = keyType();
        newNode->parent = this->parent;
        newNode->key_number = minmumNode;
        this->key_number = minmumNode;
    }
    return newNode;
}

template <class keyType>
keySize treeNode<keyType>::add(keyType &key)
{
    if(key_number == 0) {
        keys[0] = key;
        key_number++;
        return 0;
    }
    else { //key_number > 0
        keySize index = 0; // record the index of the tree
        bool exist = search(key, index);
        if(exist) {
            cout << "Already has the key in the tree" << endl;
            return -1;
        }
        else { // add the key into the node
            for(keySize i = key_number;i > index;i --)
                keys[i] = keys[i-1];
            keys[index] = key;
            
            for(keySize i = key_number + 1;i > index+1;i --)
                children[i] = children[i-1];
            children[index+1] = NULL; // this child will link to another node
            key_number ++;
            
            return index;
        }
    }
}

template <class keyType>
keySize treeNode<keyType>::add(keyType &key,offsetNumber val) {
    if(!is_leaf) {
        cout << "Error:add(keyType &key,offsetNumber val) is a function for leaf nodes" << endl;
        return -1;
    }
    if(key_number == 0) {
        keys[0] = key;
        values[0] = val;
        key_number++;
        return 0;
    }
    else { //key_number > 0
        keySize index = 0; // record the index of the tree
        bool exist = search(key, index);
        if(exist) {
            cout << "Error:In add(keyType &key, offsetNumber val),key has already in the tree!" << endl;
            exit(3);
        }
        else { // add the key into the node
            
            for(keySize i = key_number;i > index;i --) {
                keys[i] = keys[i-1];
                values[i] = values[i-1];
            }
            keys[index] = key;
            values[index] = val;
            key_number ++;
            return index;
        }
    }
}

template <class keyType>
bool treeNode<keyType>::removeFrom(keySize index) {
    if(index > key_number) {
        cout << "Error:In removeFrom(keySize index), index is more than key_number!" << endl;
        return false;
    }
    else {
        if(is_leaf) {
            for(keySize i = index;i < key_number-1;i ++) {
                keys[i] = keys[i+1];
                values[i] = values[i+1];
            }
            keys[key_number-1] = keyType();
            values[key_number-1] = offsetNumber();
        }
        else {
            for(keySize i = index;i < key_number-1;i ++)
                keys[i] = keys[i+1];
            
            for(keySize i = index+1;i < key_number;i ++)
                children[i] = children[i+1];
            
            keys[key_number-1] = keyType();
            children[key_number] = NULL;
        }
        
        key_number --;
        return true;
    }
}

template <class KeyType>
treeNode<KeyType>::~treeNode()
{
    
}

template <class keyType>
BPlusTree<keyType>::BPlusTree(string m_name,int keysize,int m_degree):
fileName(m_name),
keyCount(0),
level(0),
nodeCount(0),
root(NULL),
leafHead(NULL),
key_size(keysize),
//file(NULL),
degree(m_degree) {
    init_tree();
    readFromDiskAll();
}

template <class keyType>
BPlusTree<keyType>:: ~BPlusTree() {
    dropTree(root);
    keyCount = 0;
    root = NULL;
    level = 0;
}

template <class keyType>
void BPlusTree<keyType>::init_tree() {
    root = new treeNode<keyType>(degree,true);
    keyCount = 0;
    level = 1;
    nodeCount = 1;
    leafHead = root;
}

template <class keyType>
void BPlusTree<keyType>::findToLeaf(Node pNode, keyType key, searchNodeParse & snp) {
    keySize index = 0;
    if(pNode->search(key,index)) // find the key in the node
    {
        if(pNode->is_leaf)
        {
            snp.pNode = pNode;
            snp.index = index;
            snp.ifFound = true;
        }
        else // the node is not a leaf, continue search until the leaf level
        {
            pNode = pNode -> children[index + 1];
            while(!pNode->is_leaf)
            {
                pNode = pNode->children[0];
            }
            snp.pNode = pNode;
            snp.index = 0;
            snp.ifFound = true;
        }
        
    }
    else // can not find the key in the node
    {
        if(pNode->is_leaf)
        {
            snp.pNode = pNode;
            snp.index = index;
            snp.ifFound = false;
        }
        else
        {
            findToLeaf(pNode->children[index],key,snp);
        }
    }
}

template <class keyType>
bool BPlusTree<keyType>::insertKey(keyType &key,offsetNumber val)
{
    searchNodeParse snp;
    if(!root) init_tree();
    findToLeaf(root,key,snp);
    if(snp.ifFound)
    {
        cout << "Error:in insert key to index: the duplicated key!" << endl;
        return false;
    }
    else
    {
        snp.pNode->add(key,val);
        if(snp.pNode->key_number == degree)
        {
            adjustInsert(snp.pNode);
        }
        keyCount ++;
        return true;
    }
}

template <class keyType>
bool BPlusTree<keyType>::adjustInsert(Node pNode)
{
    keyType key;
    Node newNode = pNode->splite(key);
    nodeCount ++;
    
    if(pNode->isRoot()) {
        Node root = new treeNode<keyType>(degree,false);
        level ++;
        nodeCount ++;
        this->root = root;
        pNode->parent = root;
        newNode->parent = root;
        root->add(key);
        root->children[0] = pNode;
        root->children[1] = newNode;
        return true;

    }
    else {
        Node parent = pNode->parent;
        keySize index = parent->add(key);
        
        parent->children[index+1] = newNode;
        newNode->parent = parent;
        if(parent->key_number == degree)
            return adjustInsert(parent);
        
        return true;
    }
}

template <class keyType>
offsetNumber BPlusTree<keyType>::search(keyType& key)
{
    if(!root) return -1;
    searchNodeParse snp;
    findToLeaf(root, key, snp);
    if(!snp.ifFound)
    {
        return -1; // Don't find the key in the tree;
    }
    else
    {
        return snp.pNode->values[snp.index];
    }
    
}

template <class keyType>
bool BPlusTree<keyType>::deleteKey(keyType &key)
{
    searchNodeParse snp;
    if(!root)
    {
        cout << "ERROR: In deleteKey, no nodes in the tree " << fileName << "!" << endl;
        return false;
    }
    else
    {
        findToLeaf(root, key, snp);
        if(!snp.ifFound)
        {
            cout << "ERROR: In deleteKey, no keys in the tree " << fileName << "!" << endl;
            return false;
        }
        else // find the key in the leaf node
        {
            if(snp.pNode->isRoot())
            {
                snp.pNode->removeFrom(snp.index);
                keyCount --;
                return adjustAfterDelete(snp.pNode);
            }
            else
            {
                if(snp.index == 0 && leafHead != snp.pNode) // the key exist in the branch.
                {
                    // go to upper level to update the branch level
                    keySize index = 0;
                    
                    Node now_parent = snp.pNode->parent;
                    bool if_found_inBranch = now_parent->search(key,index);
                    while(!if_found_inBranch)
                    {
                        if(now_parent->parent)
                            now_parent = now_parent->parent;
                        else
                        {
                            break;
                        }
                        if_found_inBranch = now_parent->search(key,index);
                    }// end of search in the branch
                    
                    now_parent -> keys[index] = snp.pNode->keys[1];
                    
                    snp.pNode->removeFrom(snp.index);
                    keyCount--;
                    return adjustAfterDelete(snp.pNode);
                    
                }
                else //this key must just exist in the leaf too.
                {
                    snp.pNode->removeFrom(snp.index);
                    keyCount--;
                    return adjustAfterDelete(snp.pNode);
                }
            }
        }
    }
}


template <class keyType>
bool BPlusTree<keyType>::adjustAfterDelete(Node pNode) {
    keySize minmumKey = (degree - 1) / 2;
    if(((pNode->is_leaf)&&(pNode->key_number >= minmumKey)) || ((degree != 3)&&(!pNode->is_leaf)&&(pNode->key_number >= minmumKey - 1)) || ((degree ==3)&&(!pNode->is_leaf)&&(pNode->key_number < 0))) {
        return  true;
    }
    if(pNode->isRoot()) {
        if(pNode->key_number > 0) {
            return true;
        }
        else {
            if(root->is_leaf) {
                delete pNode;
                root = NULL;
                leafHead = NULL;
                level --;
                nodeCount --;
            }
            else {
                root = pNode -> children[0];
                root -> parent = NULL;
                delete pNode;
                level --;
                nodeCount --;
            }
        }
    }// end root
    else {
        Node parent = pNode->parent,brother = NULL;
        if(pNode->is_leaf) {
            keySize index = 0;
            parent->search(pNode->keys[0],index);
            
            if((parent->children[0] != pNode) && (index + 1 == parent->key_number)) { //choose the left brother to merge or replace
                brother = parent->children[index];
                if(brother->key_number > minmumKey) { // choose the most right key of brother to add to the left hand of the pnode
                    for(keySize i = pNode->key_number;i > 0;i --) {
                        pNode->keys[i] = pNode->keys[i-1];
                        pNode->values[i] = pNode->values[i-1];
                    }
                    pNode->keys[0] = brother->keys[brother->key_number-1];
                    pNode->values[0] = brother->values[brother->key_number-1];
                    brother->removeFrom(brother->key_number-1);
                    
                    pNode->key_number ++;
                    parent->keys[index] = pNode->keys[0];
                    return true;
                    
                } // end add
                else { // merge the node with its brother
                    parent->removeFrom(index);
                    
                    for(int i = 0;i < pNode->key_number;i ++) {
                        brother->keys[i+brother->key_number] = pNode->keys[i];
                        brother->values[i+brother->key_number] = pNode->values[i];
                    }
                    brother->key_number += pNode->key_number;
                    brother->next_leaf = pNode->next_leaf;
                    
                    delete pNode;
                    nodeCount --;
                    
                    return adjustAfterDelete(parent);
                }// end merge
                
            }// end of the left brother
            else { // choose the right brother
                if(parent->children[0] == pNode)
                    brother = parent->children[1];
                else
                    brother = parent->children[index+2];
                if(brother->key_number > minmumKey) { // choose the most left key of brother to add to the right hand of the node
                    pNode->keys[pNode->key_number] = brother->keys[0];
                    pNode->values[pNode->key_number] = brother->values[0];
                    pNode->key_number ++;
                    brother->removeFrom(0);
                    if(parent->children[0] == pNode)
                        parent->keys[0] = brother->keys[0];
                    else
                        parent->keys[index+1] = brother->keys[0];
                    return true;
                    
                }// end add
                else { // merge the node with its brother
                    for(int i = 0;i < brother->key_number;i ++) {
                        pNode->keys[pNode->key_number+i] = brother->keys[i];
                        pNode->values[pNode->key_number+i] = brother->values[i];
                    }
                    if(pNode == parent->children[0])
                        parent->removeFrom(0);
                    else
                        parent->removeFrom(index+1);
                    pNode->key_number += brother->key_number;
                    pNode->next_leaf = brother->next_leaf;
                    delete brother;
                    nodeCount --;
                    
                    return adjustAfterDelete(parent);
                }//end merge
            }// end of the right brother
            
        }// end leaf
        else { // branch node
            keySize index = 0;
            parent->search(pNode->children[0]->keys[0],index);
            if((parent->children[0] != pNode) && (index + 1 == parent->key_number)) { // choose the left brother to merge or replace
                brother = parent->children[index];
                if(brother->key_number > minmumKey - 1) { // choose the most right key and child to add to the left hand of the pnode
                    //modify the pnode
                    pNode->children[pNode->key_number+1] = pNode->children[pNode->key_number];
                    for(keySize i = pNode->key_number;i > 0;i --) {
                        pNode->children[i] = pNode->children[i-1];
                        pNode->keys[i] = pNode->keys[i-1];
                    }
                    pNode->children[0] = brother->children[brother->key_number];
                    pNode->keys[0] = parent->keys[index];
                    pNode->key_number ++;
                    //modify the father
                    parent->keys[index]= brother->keys[brother->key_number-1];
                    //modify the brother and child
                    if(brother->children[brother->key_number]) {
                        brother->children[brother->key_number]->parent = pNode;
                    }
                    brother->removeFrom(brother->key_number-1);
                    
                    return true;
                    
                }// end add
                else { // merge the node with its brother
                    //modify the brother and child
                    brother->keys[brother->key_number] = parent->keys[index];
                    parent->removeFrom(index);
                    brother->key_number ++;
                    
                    for(int i = 0;i < pNode->key_number;i ++) {
                        brother->children[brother->key_number+i] = pNode->children[i];
                        brother->keys[brother->key_number+i] = pNode->keys[i];
                        brother->children[brother->key_number+i]-> parent= brother;
                    }
                    brother->children[brother->key_number+pNode->key_number] = pNode->children[pNode->key_number];
                    brother->children[brother->key_number+pNode->key_number]->parent = brother;
                    
                    brother->key_number += pNode->key_number;
                    
                    
                    delete pNode;
                    nodeCount --;
                    
                    return adjustAfterDelete(parent);
                }
                
            }// end of the left brother
            else { // choose the right brother
                if(parent->children[0] == pNode)
                    brother = parent->children[1];
                else
                    brother = parent->children[index+2];
                if(brother->key_number > minmumKey - 1) {// choose the most left key and child to add to the right hand of the pnode
                    //modifty the pnode and child
                    pNode->children[pNode->key_number+1] = brother->children[0];
                    pNode->keys[pNode->key_number] = brother->keys[0];
                    pNode->children[pNode->key_number+1]->parent = pNode;
                    pNode->key_number ++;
                    //modify the fater
                    if(pNode == parent->children[0])
                        parent->keys[0] = brother->keys[0];
                    else
                        parent->keys[index+1] = brother->keys[0];
                    //modify the brother
                    brother->children[0] = brother->children[1];
                    brother->removeFrom(0);
                    
                    return true;
                }
                else { // merge the node with its brother
                    //modify the pnode and child
                    pNode->keys[pNode->key_number] = parent->keys[index];
                    
                    if(pNode == parent->children[0])
                        parent->removeFrom(0);
                    else
                        parent->removeFrom(index+1);
                    
                    pNode->key_number ++;
                    
                    for(int i = 0;i < brother->key_number;i++) {
                        pNode->children[pNode->key_number+i] = brother->children[i];
                        pNode->keys[pNode->key_number+i] = brother->keys[i];
                        pNode->children[pNode->key_number+i]->parent = pNode;
                    }
                    pNode->children[pNode->key_number+brother->key_number] = brother->children[brother->key_number];
                    pNode->children[pNode->key_number+brother->key_number]->parent = pNode;
                    
                    pNode->key_number += brother->key_number;
                    
                    
                    delete brother;
                    nodeCount --;
                    
                    return adjustAfterDelete(parent);
                    
                }
            }
        }
    }
    return false;
}

template <class keyType>
void BPlusTree<keyType>::dropTree(Node node) {
    if(!node) return;
    if(!node->is_leaf) { //if it has child
        for(keySize i=0;i <= node->key_number;i++) {
            dropTree(node->children[i]);
            node->children[i] = NULL;
        }
    }
    delete node;
    nodeCount --;
    return;
}

template <class keyType>
void BPlusTree<keyType>::readFromDiskAll() {
    auto &bm = BufferManager::instance();
    uint32_t offset = 0;
    Block* blk;
    while (true) {
        blk = bm.readBlock(fileName, offset);
        byte *using_uint32_temp = new unsigned char;
        memcpy(using_uint32_temp, &blk->buf[0], sizeof(int));
        int* using_size = reinterpret_cast<int*>(using_uint32_temp);
        blk->using_size = *using_size;
        if (blk->using_size == 0) break;
        readFromDisk(blk);
        offset += BLOCK_SIZE;
    }
}

template <class keyType>
void BPlusTree<keyType>::readFromDisk(Block* btmp) {
    byte* content = btmp->buf;
    
    int valueSize = sizeof(offsetNumber);
    
    byte* indexBegin = content + sizeof(uint32_t);
    byte* valueBegin = indexBegin + key_size;
    byte* indexCurrent = indexBegin;
    byte* valueCurrent = valueBegin;
    
    keyType key;
    offsetNumber value;
    
    while (valueCurrent - indexBegin < btmp->using_size) {
        key = *(keyType*)indexCurrent;
        value = *(offsetNumber*)valueCurrent;
        insertKey(key, value);
        valueCurrent += key_size + valueSize;
        indexCurrent += key_size + valueSize;
    }
}

template <class keyType>
void BPlusTree<keyType>::writeBackToDiskAll() {
    auto &bm = BufferManager::instance();
    Node ntmp = leafHead;
    uint32_t offset = 0;
    Block* blk = bm.readBlock(this->fileName, offset);
    while (ntmp != NULL) {
        blk->using_size = 0;
        blk->file_name = this->fileName;
        bm.makeDirty(blk);
        int value_size = sizeof(offsetNumber);
        for (int i = 0; i < ntmp->key_number; i++) {
            byte *key = (byte *) &(ntmp->keys[i]);
            byte *value = (byte *) &(ntmp->values[i]);
            memcpy(&blk->buf[sizeof(int) + i * (key_size + value_size)], key, key_size);
            memcpy(&blk->buf[sizeof(int) + i * (key_size + value_size) + key_size], value, value_size);
            blk->using_size += key_size + value_size;
        }
        memcpy(&blk->buf[0], &blk->using_size, sizeof(int));
        offset += BLOCK_SIZE;
        blk = bm.readBlock(this->fileName, offset);
        ntmp = ntmp->next_leaf;
    }
}


#endif /* BPlustree_h */
