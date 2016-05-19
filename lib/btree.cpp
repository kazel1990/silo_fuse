#include "btree.h"

void btree::split(btree* &right, kv& center)
{
    int cidx = BTREE_MAX_SZ >> 1;
    center = node[cidx];
    std::vector<kv> rn(node.begin() + cidx + 1, node.end());
    if (leaf)
    {
        right = new btree(rn);
    }
    else
    {
        std::vector<btree*> rc(child.begin() + cidx + 1, child.end());
        right = new btree(rn, rc);
        child.resize(child.size()-rc.size());
    }
    node.resize(node.size()-rn.size()-1);
    return;
}
bool btree::insert_rec(std::array<char,16> key, size_t val)
{
    auto it = std::lower_bound(node.begin(), node.end(), key);
    if (it != node.end() && it->key == key) return false;
    if (leaf)
    {
        kv tmp;
        tmp.key = key;
        tmp.val = val;
        node.insert(it, tmp);
        return true;
    }
    else
    {
        size_t d = std::distance(node.begin(), it);
        while (child.size() <= d)
        {
            btree *ntree;
            ntree = new btree();
            child.push_back(ntree);
        }
        if (!child[d])
        {
            child[d] = new btree();
        }
        if (child[d]->size() >= BTREE_MAX_SZ)
        {
            btree *r;
            kv center;
            child[d]->split(r, center);
            node.insert(it, center);
            child.insert(child.begin() + d + 1, r);
            if (center.key == key) return false;
            if (center.key < key) d++;
        }
        return child[d]->insert_rec(key, val);
    }
}

bool btree::serialize(FILE* &f)
{
    int p=0;
    char buf[5120];
    bool ret = true;
    for(int j=24;j>=0;j-=8) buf[p++] = (node.size() >> j) % 256;
    for(int i=0;i<node.size();i++)
    {
        for(int j=0;j<16;j++) buf[p++] = node[i].key[j];
        for(int j=24;j>=0;j-=8) buf[p++] = (node[i].val >> j) % 256;
    }
    for(int j=24;j>=0;j-=8) buf[p++] = (child.size() >> j) % 256;
    fwrite(buf, sizeof(char), sizeof(buf), f);
    for(int i=0;i<child.size();i++)
    {
        ret = child[i]->serialize(f);
        if(!ret) return false;
    }
    return ret;
}

btree::btree()
{
    leaf = true;
}

btree::btree(std::vector<kv> init_node)
{
    leaf = true;
    node = init_node;
}

btree::btree(std::vector<kv> init_node, std::vector<btree*> init_child)
{
    leaf = false;
    node = init_node;
    child = init_child;
}

btree::~btree()
{
    node.clear();
    child.clear();
}

size_t btree::size()
{
    return node.size();
}

bool btree::insert(std::array<char,16> key, size_t val)
{
    if (node.empty())
    {
        kv tmp;
        tmp.key = key;
        tmp.val = val;
        node.push_back(tmp);
        return true;
    }
    auto it = std::lower_bound(node.begin(), node.end(), key);
    if (it != node.end() && it->key == key) return false;
    if (leaf)
    {
        kv tmp;
        tmp.key = key;
        tmp.val = val;
        node.insert(it, tmp);
        
        if (node.size() >= BTREE_MAX_SZ)
        {
            leaf = false;
            btree* l, *r;
            int cidx = BTREE_MAX_SZ >> 1;
            kv center = node[cidx];
            std::vector<kv> ln(node.begin(), node.begin() + cidx);
            std::vector<kv> rn(node.begin() + cidx + 1, node.end());
            l = new btree(ln);
            r = new btree(rn);
            node.clear();
            node.push_back(center);
            child.push_back(l);
            child.push_back(r);
        }
        return true;
    }
    else
    {
        if (node.size() >= BTREE_MAX_SZ)
        {
            leaf = false;
            btree* l, *r;
            int cidx = BTREE_MAX_SZ >> 1;
            kv center = node[cidx];
            std::vector<kv> ln(node.begin(), node.begin() + cidx);
            std::vector<btree*> lc(child.begin(), child.begin() + cidx + 1);
            std::vector<kv> rn(node.begin() + cidx + 1, node.end());
            std::vector<btree*> rc(child.begin() + cidx + 1, child.end());
            l = new btree(ln, lc);
            r = new btree(rn, rc);
            node.clear();
            node.push_back(center);
            child.clear();
            child.push_back(l);
            child.push_back(r);
        }
        it = std::lower_bound(node.begin(), node.end(), key);
        size_t d = std::distance(node.begin(), it);
        if (child[d]->size() >= BTREE_MAX_SZ)
        {
            btree *r;
            kv center;
            child[d]->split(r, center);
            node.insert(it, center);
            child.insert(child.begin() + d + 1, r);
            if (center.key == key) return false;
            if (center.key < key) d++;
        }
        return child[d]->insert_rec(key, val);
    }
}

size_t btree::find(std::array<char,16> key)
{
    auto it = std::lower_bound(node.begin(), node.end(), key);
    if (it != node.end() && it->key == key) return it->val;
    if (leaf) return 0;
    int d = std::distance(node.begin(), it);
    return child[d]->find(key);
}

bool btree::save(char* dir)
{
    FILE *f;
    try
    {
        f = fopen(dir,"w");
    } catch (int e) {
        return false;
    }
    if(f!=NULL) return serialize(f);
    return false;
}
