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

KVStore::KVStore()
{
}

KVStore::~KVStore()
{
}

int KVStore::do_read(const string& key, unsigned char* value, size_t & sz)
{
    return 0;
}

int KVStore::do_write(const string& key, const unsigned char* value, const size_t sz)
{
    return 0;
}

int KVStore::do_update(const string& key, const unsigned char* value, const size_t sz)
{
    return 0;
}

int KVStore::do_delete(const string& key, unsigned char* value, size_t & sz)
{
    return 0;
}
