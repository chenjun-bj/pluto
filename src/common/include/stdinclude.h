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
#define _STD_INCLUDE_H_


/**
 *******************************************************************************
 * Headers                                                                     *
 *******************************************************************************
 */
#include <unistd.h>
#include <errno.h>

#include <string.h>
#include <stdlib.h>

#include "pltypes.h"
#include "plexcept.h"
#include "log.h"
#include "config.h"
#include "util.h"

/**
 *******************************************************************************
 * Constants                                                                   *
 *******************************************************************************
 */
/* Current version number */
#define PLUTO_CURRENT_VERSION   1

#define KV_RING_DEF_SIZE 512   // The ring size, that the maximum number of node 
                               // supported by store
#define KV_RING_MIN_SIZE 1       // Minimum size of the ring
#define KV_RING_MAX_SIZE 32768   // Maximum ring size that can be supported.

// Must be a odd number
#define PLUTO_NODE_REPLICAS_NUM    3

/* Membership protocol parameters */
#define MEM_PROT_DEF_TFAIL     5
#define MEM_PROT_DEF_TREMOVE   20
#define MEM_PROT_DEF_NAME      "GOSSIP" 

#define MEM_PROT_DEF_GOSSIP_B  3

/* Environment variable names */
#define ENV_NM_BASE_IPCKEY    "PLUTO_IPCKEY" 

#define    PLSUCCESS    0
#define    PLERROR      1

/**
 *******************************************************************************
 * Function declaractions                                                      *
 *******************************************************************************
 */

#endif // _STD_INCLUDE_H_

