/**
 *******************************************************************************
 * config.h                                                                    *
 *                                                                             *
 * Configuration portal                                                        *
 *******************************************************************************
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include <sys/ipc.h>

#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>

/*
 *******************************************************************************
 *  Macros                                                                     *
 *******************************************************************************
 */

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

class ConfigPortal {
private:
    ConfigPortal();
public:
    typedef std::pair<std::string, unsigned short> IPAddr;

    virtual ~ConfigPortal() ;

    bool load(const std::string & filename);

    key_t get_ipckey() const;

    long get_ringsize() const;

    /**/
    std::string get_logpath() const;

    std::vector<IPAddr> get_joinaddress() const;
    std::string get_bindip() const;
    unsigned short get_bindport() const;

    /**
     * Protocal parameters
     */
    // TFAIL : fail the node after TFAIL seconds since last heartbeat
    long  get_failtime() const;
    // TREMOVE : remove the node from memeber list after TREMOVE seconds since 
    //           node fail
    long  get_rmtime() const;

    std::string get_protocol() const;
    long get_gossipb() const;
        
    static ConfigPortal* get_config();
private:
    static ConfigPortal* s_inst;

    boost::property_tree::ptree m_ptree;    
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

ConfigPortal* GetConfigPortal();

#endif // _LOG_H_

