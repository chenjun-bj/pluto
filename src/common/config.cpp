/**
 *******************************************************************************
 * config.cpp                                                                  *
 *                                                                             *
 * Configuration protal                                                        *
 *******************************************************************************
 */

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include <string>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <stdlib.h>
#include "stdinclude.h"
#include "config.h"

using namespace std;

#define CFG_JSON_PATH_IPCKEY   "IPCKEY"
#define CFG_JSON_PATH_TFAIL    "PROTOCOL_PARAM.TFAIL"
#define CFG_JSON_PATH_TREMOVE  "PROTOCOL_PARAM.TREMOVE"
#define CFG_JSON_PATH_PORT_NM  "PROTOCOL_PARAM.NAME"
#define CFG_JSON_PATH_GOSSIP_B "PROTOCOL_PARAM.GOSSIP.B"
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
ConfigPortal* ConfigPortal::s_inst = NULL;

ConfigPortal* ConfigPortal::get_config()
{
    if (s_inst==NULL) {
        // TODO: use mutex to protect
        s_inst = new ConfigPortal();
    }
    return s_inst;
}

ConfigPortal::ConfigPortal() : m_ptree()
{
}

ConfigPortal::~ConfigPortal()
{
}

bool ConfigPortal::load(const string& filename)
{
    namespace pt = boost::property_tree;

    string suffix;
    string::size_type pos = filename.find_last_of('.');
    if (pos != string::npos) {
        suffix=filename.substr(pos+1);
    }

    try {
        if (strcasecmp(suffix.c_str(), "JSON") == 0) {
            pt::read_json(filename, m_ptree);
            return true;
        }
    }
    catch(pt::ptree_bad_path & e) {
        getlog()->sendlog(FATAL, "'%s': bad path : '%s'\n", filename.c_str(), e.what());
        return false;
    }
    catch(pt::ptree_bad_data & e) {
        getlog()->sendlog(FATAL, "'%s': bad data : '%s'\n", filename.c_str(), e.what());
        return false;
    }
    catch(...) {
        getlog()->sendlog(FATAL, "'%s': unknown error occur during parsing\n", filename.c_str());
        return false;
    }

    getlog()->sendlog(FATAL, "'%s': format not support\n", filename.c_str());
    return false;
}

key_t ConfigPortal::get_ipckey() const
{
    int defkey = -1;
    char* env = getenv(ENV_NM_BASE_IPCKEY);
    if (env != NULL) {
        defkey = atoi(env);
    }
    return m_ptree.get(CFG_JSON_PATH_IPCKEY, defkey);
}

long ConfigPortal::get_ringsize() const
{
    return KV_RING_DEF_SIZE;
}

long ConfigPortal::get_failtime() const
{
    return m_ptree.get(CFG_JSON_PATH_TFAIL, MEM_PROT_DEF_TFAIL);
}

long ConfigPortal::get_rmtime() const
{
    return m_ptree.get(CFG_JSON_PATH_TREMOVE, MEM_PROT_DEF_TFAIL);
}

string ConfigPortal::get_protocol() const
{
    return m_ptree.get(CFG_JSON_PATH_PORT_NM, MEM_PROT_DEF_NAME);
}

long ConfigPortal::get_gossipb() const
{
    return m_ptree.get(CFG_JSON_PATH_GOSSIP_B, MEM_PROT_DEF_GOSSIP_B);
}

ConfigPortal* GetConfigPortal()
{
    return ConfigPortal::get_config();
}

