/**
 *******************************************************************************
 * KVStore.cpp                                                            *
 *                                                                             *
 * Key value store:                                                            *
 *   - Provide key-value store                                                 *
 *******************************************************************************
 */


/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"

#include "KVStore.h"

using namespace std;
/*
 *******************************************************************************
 *  Forward declaraction                                                       *
 *******************************************************************************
 */

/*
 *******************************************************************************
 *  Class declaraction                                                         *
 *******************************************************************************
 */

KVStore::KVStore() : m_storage()
{
    for (int i=0; i<PLUTO_NODE_REPLICAS_NUM; i++) {
        STORAGE_MAP m;
        m_storage.emplace_back(m);
    }
}

KVStore::~KVStore()
{
}

int KVStore::do_read(const string& key, int replica_type, 
                     vector<unsigned char> & v)
{
    if (replica_type<0 || replica_type>PLUTO_NODE_REPLICAS_NUM) {
        getlog()->sendlog(LogLevel::ERROR, "Store: invalid replica type '%d'\n", replica_type);
        return -1;
    }

    STORAGE_MAP m = m_storage[replica_type];
    if (m.find(key) != m.end()) {
        v = m[key];
        return 0;
    }
    return -1;
}

int KVStore::do_write(const string& key, int replica_type,
                      const vector<unsigned char> & v)
{
    if (replica_type<0 || replica_type>PLUTO_NODE_REPLICAS_NUM) {
        getlog()->sendlog(LogLevel::ERROR, "Store: invalid replica type '%d'\n", replica_type);
        return -1;
    }

    STORAGE_MAP m = m_storage[replica_type];
    if (m.find(key) != m.end()) {
        // duplicate
        return -1;
    }

    m.insert(make_pair(key, v));

    return 0;
}

int KVStore::do_update(const string& key, int replica_type,
                       const vector<unsigned char> & v)
{
    if (replica_type<0 || replica_type>PLUTO_NODE_REPLICAS_NUM) {
        getlog()->sendlog(LogLevel::ERROR, "Store: invalid replica type '%d'\n", replica_type);
        return -1;
    }

    STORAGE_MAP m = m_storage[replica_type];
    if (m.find(key) == m.end()) {
        return -1;
    }

    m[key] = v;

    return 0;
}

int KVStore::do_delete(const string& key, int replica_type)
{
    if (replica_type<0 || replica_type>PLUTO_NODE_REPLICAS_NUM) {
        getlog()->sendlog(LogLevel::ERROR, "Store: invalid replica type '%d'\n", replica_type);
        return -1;
    }

    STORAGE_MAP m = m_storage[replica_type];
    m.erase(key);

    return 0;
}

int KVStore::do_delete(int replica_type)
{
    if (replica_type<0 || replica_type>PLUTO_NODE_REPLICAS_NUM) {
        getlog()->sendlog(LogLevel::ERROR, "Store: invalid replica type '%d'\n", replica_type);
        return -1;
    }

    STORAGE_MAP m = m_storage[replica_type];
    m.clear();

    return 0;
}

int KVStore::do_get(int replica_type,
                    std::map<std::string, std::vector<unsigned char> > & v,
                    bool remove)
{
    if (replica_type<0 || replica_type>PLUTO_NODE_REPLICAS_NUM) {
        getlog()->sendlog(LogLevel::ERROR, "Store: invalid replica type '%d'\n", replica_type);
        return -1;
    }

    STORAGE_MAP m = m_storage[replica_type];
    v = m;
    if (remove) {
        m.clear();
    }

    return 0;
}

/* eof */

