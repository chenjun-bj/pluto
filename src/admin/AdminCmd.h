/**
 *******************************************************************************
 * AdminCmd.h                                                                  *
 *                                                                             *
 * Node administration commands                                                *
 *******************************************************************************
 */

#ifndef _ADMIN_CMDS_H_
#define _ADMIN_CMDS_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"
#include "memberlist.h"
#include <string>
#include <map>
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

class admincmd {
public:
    virtual ~admincmd() {
    }
    // return 0 for success, others error
    virtual int operator()(const std::string& in) = 0;
    virtual const std::string usage() = 0;
};

class cmdproc {
public:
    ~cmdproc();
    void register_cmd(const std::string& cmd, admincmd * pcmd);
    void handle_input(const std::string& line);
private:
    std::map< std::string, admincmd* > m_cmds;
};

class lst_member : public admincmd {
public:
    lst_member(MemberList * pmlst) : m_pmlst(pmlst) {
    }

    int operator()(const std::string& in);
    const std::string usage() {
        return "";
    }
private:
    const MemberList * m_pmlst;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _ADMIN_CMDS_H_

