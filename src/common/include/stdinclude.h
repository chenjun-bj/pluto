/**
 *******************************************************************************
 * stdinclude.h                                                                *
 *                                                                             *
 * Standard hearder file:                                                      *
 *   - defines constants for project                                           *
 *   - includes header files used by all source                                *
 *******************************************************************************
 */

#ifndef _STD_INCLUDE_H_
#define _STD_INLCUDE_H_


/**
 *******************************************************************************
 * Headers                                                                     *
 *******************************************************************************
 */
#include <unistd.h>
#include <errno.h>

#include <string.h>
#include <stdlib.h>

#include "../log.h"

/**
 *******************************************************************************
 * Constants                                                                   *
 *******************************************************************************
 */

#define KV_RING_SIZE 512   // The ring size, that the maximum number of node 
                           // supported by store

/**
 *******************************************************************************
 * Function declaractions                                                      *
 *******************************************************************************
 */
void daemonize(const char *cmd);

#endif // _STD_INCLUDE_H_

