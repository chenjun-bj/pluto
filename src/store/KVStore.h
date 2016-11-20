/**
 *******************************************************************************
 * KVStore.h                                                                   *
 *                                                                             *
 * Key value store:                                                            *
 *   - Provide key-value store                                                 *
 *******************************************************************************
 */

#ifndef _KEY_VALUE_STORE_H_
#define _KEY_VALUE_STORE_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"

#include <string>

#include <map>
#include <vector>

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

/**
 * NO lock for the store!!!
 */
class KVStore {
public:
    KVStore();
    ~KVStore();

    int do_read(const std::string& key, int replica_type, 
                unsigned char* value, size_t &sz);
    int do_write(const std::string& key, int replica_type, 
                 const unsigned char* value, const size_t sz);
    int do_update(const std::string& key, int replica_type, 
                  const unsigned char* value, const size_t sz);
    int do_delete(const std::string& key, int replica_type, 
                  unsigned char* value, size_t &sz);
private:
    typedef std::map<std::string, std::vector<unsigned char> > STORAGE_MAP;
    std::vector< STORAGE_MAP >  m_storage; 
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _KEY_VALUE_STORE_H_

