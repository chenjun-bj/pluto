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

    unsigned int get_membership_period() const {
        return 5;
    }

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

    int get_message_timeout() const {
        return 2000; // 2 seconds
    }

    int get_quorum_num() const ;

    const std::string& get_config_filename() const {
        return m_cfg_fn;
    }

    static ConfigPortal* get_config();
private:
    static ConfigPortal* s_inst;

    boost::property_tree::ptree m_ptree;    
    std::string  m_cfg_fn;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

ConfigPortal* GetConfigPortal();

#endif // _LOG_H_

