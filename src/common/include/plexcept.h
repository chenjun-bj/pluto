/**
 *******************************************************************************
 * plexcept.h                                                                  *
 *                                                                             *
 * Exceptions :                                                                *
 *   parse_error                                                               *
 *******************************************************************************
 */

#ifndef _PL_EXCEPT_H_
#define _PL_EXCEPT_H_


/**
 *******************************************************************************
 * Headers                                                                     *
 *******************************************************************************
 */

#include <stdexcept>

/**
 *******************************************************************************
 * Constants                                                                   *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * Types                                                                       *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * Classes                                                                     *
 *******************************************************************************
 */

class parse_error : public std::runtime_error {
public:
    parse_error(const char* errmsg): 
       std::runtime_error(errmsg) {
    } 
};
/**
 *******************************************************************************
 * Function declaractions                                                      *
 *******************************************************************************
 */

#endif // _PL_EXCEPT_H__

