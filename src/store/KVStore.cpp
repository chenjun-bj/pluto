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
                     unsigned char* value, size_t & sz)
{
    if (replica_type<0 || replica_type>PLUTO_NODE_REPLICAS_NUM) {
        getlog()->sendlog(LogLevel::ERROR, "Store: invalid replica type '%d'\n", replica_type);
        return -1;
    }

    if (value == nullptr) {
        getlog()->sendlog(LogLevel::ERROR, "Store: read out buffer invalid\n");
        return -1;
    }

    STORAGE_MAP m = m_storage[replica_type];
    if (m.find(key) != m.end()) {
        vector<unsigned char> v = m[key];
        if (v.size()>sz) {
            sz = v.size();
            return -1;
        }
        sz = v.size();
        memcpy(value, v.data(), sz);
    }
    return 0;
}

int KVStore::do_write(const string& key, int replica_type,
                      const unsigned char* value, const size_t sz)
{
    if (replica_type<0 || replica_type>PLUTO_NODE_REPLICAS_NUM) {
        getlog()->sendlog(LogLevel::ERROR, "Store: invalid replica type '%d'\n", replica_type);
        return -1;
    }

    if (value == nullptr) {
        getlog()->sendlog(LogLevel::ERROR, "Store: write in buffer invalid\n");
        return -1;
    }

    STORAGE_MAP m = m_storage[replica_type];
    if (m.find(key) != m.end()) {
        // duplicate
        return -1;
    }

    vector<unsigned char> v(sz, '\0');
    memcpy(v.data(), value, sz);

    m.insert(make_pair(key, v));

    return 0;
}

int KVStore::do_update(const string& key, int replica_type,
                       const unsigned char* value, const size_t sz)
{
    if (replica_type<0 || replica_type>PLUTO_NODE_REPLICAS_NUM) {
        getlog()->sendlog(LogLevel::ERROR, "Store: invalid replica type '%d'\n", replica_type);
        return -1;
    }

    if (value == nullptr) {
        getlog()->sendlog(LogLevel::ERROR, "Store: update in buffer invalid\n");
        return -1;
    }

    STORAGE_MAP m = m_storage[replica_type];
    if (m.find(key) == m.end()) {
        return -1;
    }

    vector<unsigned char> v = m[key];
    v.resize(sz);
    memcpy(v.data(), value, sz);

    return 0;
}

int KVStore::do_delete(const string& key, int replica_type,
                       unsigned char* value, size_t & sz)
{
    if (replica_type<0 || replica_type>PLUTO_NODE_REPLICAS_NUM) {
        getlog()->sendlog(LogLevel::ERROR, "Store: invalid replica type '%d'\n", replica_type);
        return -1;
    }

    STORAGE_MAP m = m_storage[replica_type];
    if (value != nullptr) {
        if (m.find(key) != m.end()) {
            vector<unsigned char> v = m[key];
            if (sz > v.size()) {
                sz = v.size();
            }
            memcpy(value, v.data(), sz);
        }
        else {
            return -1;
        }
    }
    m.erase(key);

    return 0;
}
