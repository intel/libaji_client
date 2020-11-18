/****************************************************************************
 *   Copyright (c) 2001 by Intel Corporation                                *
 *   author: Whitaker, Alan and Draper, Andrew                              *
 *   SPDX-License-Identifier: MIT                                           *
 *                                                                          *
 *   Permission is hereby granted, free of charge, to any person obtaining  *
 *   a copy of this software and associated documentation files (the        *
 *   "Software"), to deal in the Software without restriction, including    *
 *   without limitation the rights to use, copy, modify, merge, publish,    *
 *   distribute, sublicense, and/or sell copies of the Software, and to     *
 *   permit persons to whom the Software is furnished to do so, subject to  *
 *   the following conditions:                                              *
 *                                                                          *
 *   The above copyright notice and this permission notice (including the   *
 *   next paragraph) shall be included in all copies or substantial         *
 *   portions of the Software.                                              *
 *                                                                          *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 *   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. *
 *   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   *
 *   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   *
 *   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      *
 *   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 *
 ****************************************************************************/

//# START_MODULE_HEADER/////////////////////////////////////////////////////////
//#
//#
//#
//# Description:
//#
//# Authors:     Alan Whitaker, Andrew Draper
//#
//#              Copyright (c) Altera Corporation 2000 - 2001
//#              All rights reserved.
//#
//# END_MODULE_HEADER///////////////////////////////////////////////////////////
#ifndef INC_AJI_H
#define INC_AJI_H

//# INCLUDE FILES //////////////////////////////////////////////////////////
#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef INC_AJI_SYS_H
#include "aji_sys.h"
#endif

#ifndef INC_AJI_MACROS_SYS_H
#include "aji_macros_sys.h"
#endif

//# MACRO DEFINITIONS //////////////////////////////////////////////////////
/* \file
 * Terminology guide :
 *| JTAG spec | In this guide |
 *| ------------ | -------------- - |
 *| scan chain | device chain, hardwareand represented by #AJI_HARDWARE |
 *| TAP | TAP, TAP Controller, deviceand represented by #AJI_DEVICE |
 * 
 */

/**
 * \def AJI_API
 * This macro is a decorator and it decides 
 * whether the  function it is decorating is is imported from a DLL file 
 * for use in this compilataion unit  or this compilation unit
 * is compiling a DLL that will export this function. This macro is
 * define to be an alias of #JTAG_DLLEXPORT. Please modify #JTAG_DLLEXPORT
 * value instead of this macro.
 *
 * By default, the function it decorates will be imported from a DLL.
 *
 * When you are compiling the DLL that export this function
 * set your compiler to define JTAG_DLLEXPORT to #AJI_DLLEXPORT.
 * 
 * Example: On Windows, the declaration
 * \code
 *    AJI_ERROR AJI_API aji_get_hardware(...);
 * \endcode
 * will be preprocessed into
 * \code
 *    AJI_ERROR __delspec(import) aji_get_hardware(...);
 * \endcode
 * because #AJI_API will resolve to #JTAG_DLLEXPORT which in
 * turn solve to #DLLIMPORT which is <tt>__delspec(import)</tt>.
 * However, if you define #JTAG_DLLEXPORT to #AJI_DLLEXPORT
 * when you compile the code, i.e.
 * \verbatim gcc -c -DJTAG_DLLEXPORT=AJI_DLLEXPORT \endverbatim
 * you will get
 * \code
 *    AJI_ERROR __delspec(export) aji_get_hardware(...);
 * \endcode
 * as #JTAG_DLLEXPORT now takes the value of $AJI_DLLEXPORT
 * which itself is defined to <tt>__delspec(export)</tt>
 * 
 * Note that the recommendation here is to use #AJI_DLLEXPORT, not
 * #DLLEXPORT. The reason is not very clear to me (cinlyooi). However
 * there are signs that there are some special handling of #AJI_API
 * needed during PDB_PARSING_PHASE of the compilation. Precise reason
 * why the corresponding #AJI_DLLIMPORT is not used in place of
 * #DLLIMPORT is also not clear.
 *
 */

/**
 * Decorator that controls whether a symbol (function) is 
 * imported or exported. \see AJI_API.
 */
#ifndef JTAG_DLLEXPORT
#define JTAG_DLLEXPORT DLLIMPORT
#endif

#ifndef AJI_API
#ifdef WIN32
#define AJI_API JTAG_DLLEXPORT
#else
#define AJI_API JTAG_DLLEXPORT
#endif
#endif

#define AJI_MAX_HIERARCHICAL_HUB_DEPTH 8

//# ENUMERATIONS ///////////////////////////////////////////////////////////

/**
 * Status Code
 *
 * \note These are transmitted from client to server so must not change.
 */
enum AJI_ERROR 
{
    AJI_NO_ERROR               = 0,
    AJI_FAILURE                = 1,
    AJI_TIMEOUT                = 2,

    AJI_UNKNOWN_HARDWARE      = 32,
    AJI_INVALID_CHAIN_ID      = 33,
    AJI_LOCKED                = 34,
    AJI_NOT_LOCKED            = 35,

    AJI_CHAIN_IN_USE          = 36,
    AJI_NO_DEVICES            = 37,
    AJI_CHAIN_NOT_CONFIGURED  = 38,
    AJI_BAD_TAP_POSITION      = 39,
    AJI_DEVICE_DOESNT_MATCH   = 40,
    AJI_IR_LENGTH_ERROR       = 41,
    AJI_DEVICE_NOT_CONFIGURED = 42,
    AJI_CHAINS_CLAIMED        = 43,

    AJI_INVALID_OPEN_ID       = 44,
    AJI_INVALID_PARAMETER     = 45,
    AJI_BAD_TAP_STATE         = 46,
    AJI_TOO_MANY_DEVICES      = 47,
    AJI_IR_MULTIPLE           = 48,
    AJI_BAD_SEQUENCE          = 49,
    AJI_INSTRUCTION_CLAIMED   = 50,
    AJI_MODE_NOT_AVAILABLE    = 51, ///< The mode requested is not supported by this hardware
    AJI_INVALID_DUMMY_BITS    = 52, ///< The number of dummmy bits is out of range

    AJI_FILE_ERROR            = 80,
    AJI_NET_DOWN              = 81,
    AJI_SERVER_ERROR          = 82,
    AJI_NO_MEMORY             = 83, ///< Out of memory when configuring 
    AJI_BAD_PORT              = 84, ///< Port number (eg LPT1) does not exist
    AJI_PORT_IN_USE           = 85,
    AJI_BAD_HARDWARE          = 86, ///< Hardware (eg byteblaster cable) not connected to port
    AJI_BAD_JTAG_CHAIN        = 87, ///< JTAG chain connected to hardware is broken
    AJI_SERVER_ACTIVE         = 88, ///< Another thread in this process is using the JTAG Server
    AJI_NOT_PERMITTED         = 89,
    AJI_HARDWARE_DISABLED     = 90, ///< Disable input to On-board cable is driven active

    AJI_HIERARCHICAL_HUB_NOT_SUPPORTED = 125,
    AJI_UNIMPLEMENTED        = 126,
    AJI_INTERNAL_ERROR       = 127,

    ///@{
    ///These errors are generated on the client side only
    AJI_NO_HUBS              = 256,
    AJI_TOO_MANY_HUBS        = 257,
    AJI_NO_MATCHING_NODES    = 258,
    AJI_TOO_MANY_MATCHING_NODES = 259,
    AJI_TOO_MANY_HIERARCHIES = 260
    ///@}
};

/**
 * \note These are transmitted from client to server so must not change.
 */
enum AJI_CHAIN_TYPE
{
    AJI_CHAIN_UNKNOWN = 0,
    AJI_CHAIN_JTAG    = 1,
};

/**
 * \note These are transmitted from client to server so must not change.
 */
enum AJI_FEATURE
{
    AJI_FEATURE_DUMMY    = 0x0001, ///< This is a dummy chain which does not have hardware
    AJI_FEATURE_DYNAMIC  = 0x0002, ///< This chain was auto-detected and can't be removed
    AJI_FEATURE_SPECIAL  = 0x0004, ///< This chain needs special setup before use
    AJI_FEATURE_LED      = 0x0100, ///< This chain has a controllable status LED
    AJI_FEATURE_BUTTON   = 0x0200, ///< This chain has a "Start" button
    AJI_FEATURE_INSOCKET = 0x0400, ///< This chain supports in-socket programming
    AJI_FEATURE_JTAG     = 0x0800, ///< This chain supports JTAG access
    AJI_FEATURE_SECONDARY= 0x4000, ///< This chain was created by a secondary server
    AJI_FEATURE_IDENTIFY = 0x8000  ///< This chain supports an LED identify feature
};

/**
 * \note  // These are used in the client API so must not change.
 */
enum AJI_PACK_STYLE
{
    AJI_PACK_NEVER  = 0, ///< The client will always send commands to the server 
                         ///<    and will block until it receives a response.
    AJI_PACK_AUTO   = 1, ///< The client will send commands which return a result
                         ///<     to the server and will delay other commands.
    AJI_PACK_MANUAL = 2, ///M The client will not normally send commands to
                         ///<     the server unless instructed to using #aji_flush.
    AJI_PACK_STREAM = 3
};

/**
 * Indicates how the JTAG server may optimise the scan
 * \note These are transmitted from client to server so must not change.
 */
enum AJI_DR_FLAGS
{
    AJI_DR_UNUSED_0       = 1,  ///< Allow zeros to be written to unspecified bits.
                                ///<   The JTAG server may, if it is faster,
                                ///<   clock zeros into the chain 
                                ///<   instead of clocking the value read.
    AJI_DR_UNUSED_0_OMIT  = 3,  ///< Allow zeros at the TDI end, allow any value at TDO end. 
                                ///<   The JTAG server may, if it is faster, 
                                ///<   clock zeros into the chain instead of 
                                ///<   clocking the value read.
                                ///< It may also omit clocks for efficiency.
    AJI_DR_UNUSED_X       = 15, ///<   Allow any value to be written to unspecified bits.
                                ///<   The JTAG server may, if it is faster, 
                                ///<   clock any value into bits not specifically 
                                ///<   written by this call.  
                                ///< It may also omit clocks for efficiency.
    AJI_DR_NO_SHORT       = 16, ///<   Must clock all bits through (disable optimisations).
                                ///<   The JTAG server must not omit any clocks 
                                ///<   for efficiency even though it would be 
                                ///<   permitted by the values above (it might 
                                ///<   otherwise do this if the updated value 
                                ///<   would otherwise be the same).
    AJI_DR_END_PAUSE_DR   = 32, ///< End the dr scan in the PAUSE_DR state
    AJI_DR_START_PAUSE_DR = 64, ///< Allow the dr scan to start in the PAUSE_DR state
    AJI_DR_NO_RESTORE     = 128 ///< Do not reload previous instruction and overlay after relocking
};

/**
 * \note These are transmitted from client to server so must not change.
 */
enum AJI_IR_FLAGS
{
    AJI_IR_COULD_BREAK   = 2  ///< IR chain could break at this device on next scan
};

/**
 * \note These are transmitted from client to server so must not change.
 */
enum AJI_RTI_FLAGS
{
    AJI_ACCURATE_CLOCK = 1, ///< Clock must run at exactly the clock rate set for this scan
    AJI_EXIT_RTI_STATE = 2  ///< Exit from RUN-TEST-IDLE state after scan
};

/**
 * Device feature bitfield
 * \note These are transmitted from client to server so must not change.
 */
enum AJI_DEVFEAT 
{
    AJI_DEVFEAT_BAD_IR_CAPTURE          = 1, ///< Captured IR value does not always end 'b01
    AJI_DEVFEAT_USER01_BREAK_IR         = 2, ///< USER01 instructions can break the IR chain
    AJI_DEVFEAT_POSSIBLE_HUB            = 4, ///< Potentially contains a SLD HUB
    AJI_DEVFEAT_COMMAND_RESPONSE_CHAIN  = 8  ///< Support COMMAND and RESPONSE scan chains
};

/**
 * Direct pin control bitfield
 * 
 * Look at the source code to see map to the actual pin
 * and to see the pin names in JTAG, Passive and Active mode
 * 
 * \note These are transmitted from client to server so must not change.
 */
enum AJI_PINS 
{                  // Mode:  JTAG   Passive   Active
    AJI_PIN_TDI = 0x01,  //   TDI     DATA      ASDI
    AJI_PIN_TMS = 0x02,  //   TMS   nCONFIG  nCONFIG
    AJI_PIN_TCK = 0x04,  //   TCK     DCLK      DCLK
    AJI_PIN_NCS = 0x08,  //     -        -       nCS
    AJI_PIN_NCE = 0x10,  //     -        -       nCE

    AJI_PIN_TDO = 0x01,  //   TDO  CONFDONE CONFDONE
    AJI_PIN_NST = 0x02   //  RTCK   nSTATUS  DATAOUT
};

//# FORWARD REFERENCES FOR CLASSES /////////////////////////////////////////

//# TYPEDEFS ///////////////////////////////////////////////////////////////

typedef class AJI_CHAIN * AJI_CHAIN_ID;
typedef class AJI_OPEN  * AJI_OPEN_ID;

//# CLASS AND STRUCTURE DECLARATIONS ///////////////////////////////////////

/**
 * The AJI_HARDWARE class represents one chain attached to one hardware driver.
 * 
 * This chain can either be a jtag chain or a passive serial chain (as
 * indicated by chain_type).
 */
typedef struct AJI_HARDWARE AJI_HARDWARE;
struct AJI_HARDWARE
{
    AJI_CHAIN_ID   chain_id;   ///< The ID for the chain this hardware belongs to
    DWORD          persistent_id; ///< This is the only member guaratnteed to remain 
                                  ///< across invocations and survive system reboot.
    const char   * hw_name;     ///< Name of this type of hardware
    const char   * port;
    const char   * device_name; ///< Name given to hardware by user (or NULL)
    AJI_CHAIN_TYPE chain_type;
    const char   * server;      ///< Name of server this is attached to (NULL if local)
    DWORD          features;    ///< Bitwise OR of 
                                ///< #AJI_FEATURE_DUMMY, #AJI_FEATURE_DYNAMIC and #AJI_FEATURE_SPECIAL
};

/**
 * The AJI_DEVICE class represents the information which the server needs to
 * know about one JTAG TAP controller on a JTAG chain.
 */
typedef struct AJI_DEVICE AJI_DEVICE;
struct AJI_DEVICE
{
    DWORD         device_id;
    DWORD         mask;                 ///< 1 bit in mask indicates X in device_id
    BYTE          instruction_length;
    DWORD         features;             ///< Bitwise or of AJI_DEVFEAT
    const char  * device_name;          ///< May be NULL
};

typedef struct AJI_HUB_INFO AJI_HUB_INFO;
/**
 * SLD HUB
 */
struct AJI_HUB_INFO
{
    DWORD bridge_idcode[AJI_MAX_HIERARCHICAL_HUB_DEPTH]; 
                ///< Parent bridge's IDCODE 
                ///< element at index 0 always have  a don't care value because 
                ///< top level hub does not have a  parent bridge
    DWORD hub_idcode[AJI_MAX_HIERARCHICAL_HUB_DEPTH]; ///< Parent Hub
};

typedef struct AJI_HIER_ID AJI_HIER_ID;
/**
 * SLD Node 
 */
struct AJI_HIER_ID
{
    DWORD idcode;                                   ///< 32-bit node info
    BYTE position_n;                                ///< Hierarchy level, 0 indicates top-level node
    BYTE positions[AJI_MAX_HIERARCHICAL_HUB_DEPTH]; ///< Position of node in each hierarchy
};


/**
 *
 * |bit   ||||||
 * |------|----------|--------|-------|-------|---------|
 * |15..12|11        |10      |9      |8      |7..0 |
 * |0000  |WEAK_CLAIM|OVERLAID|OVERLAY|SHARED|RESOURCE|
 *
 * The least significant bits (RESOURCE) hold the type of 
 * resource being claimed. Currently the following RESOURCE
 * are assigned:
 * - 0x0 for IR
 * - 0x1 for DR
 * 
 * The following bits are valid for all resource types:
 * - The SHARED bit allows more then one client to claim the resource at the
 *   same time (providing that the claim types are compatible).
 *
 * The following bits are valid for IR resources (those which represent
 * access to JTAG instructions):
 * - The OVERLAY bit tells the JTAG server that this is the overlay register
 *   for the device (a register whose value changes the meaning of the
 *   overlaid registers).  Exclusive access to an overlay register is not
 *   allowed.
 * - The OVERLAID bit tells the JTAG server that the meaning of this register
 *   is affected by the value in the overlay register.  If this value is
 *   present in the IR when the device is locked then the JTAG server will
 *   ensure that the overlay register holds the correct value.
 *
 * A device can only have one overlay register (which must be listed before
 * all overlaid registers in the list of claims).
 *
 * For exclusive and shared claims the first client to claim an instruction
 * is successful.  Subsequent claims for the same IR / OVERLAY are succeed
 * only if all the claims are SHARED.
 *
 * A weak claim is always successful at open time, however the availability
 * of the IR / OVERLAY value are checked each time it is used.  A subsequent
 * exclusive or shared claim will override a weak claim for a resource.
 * A weak claim of ~0 allows weak access to all values in that resource.
 * Because weak claims are checked at the time aji_access_ir/dr is called
 * there is a performance implication to using weak claims.
 *
 * This enum lists all valid combinations
 */
enum AJI_CLAIM_TYPE
{
    AJI_CLAIM_IR                 = 0x0000, ///< Exclusive access to this IR value
    AJI_CLAIM_IR_SHARED          = 0x0100, ///< Shared access to this IR value
    AJI_CLAIM_IR_SHARED_OVERLAY  = 0x0300, ///< Shared access to this OVERLAY IR value
    AJI_CLAIM_IR_OVERLAID        = 0x0400, ///< Exclusive access to this OVERLAID IR value
    AJI_CLAIM_IR_SHARED_OVERLAID = 0x0500, ///< Shared access to this OVERLAID IR value
    AJI_CLAIM_IR_WEAK            = 0x0800, ///< Allow access to this IR value if unclaimed
                                           ///< (value ~0 means all unclaimed IR values)

    AJI_CLAIM_OVERLAY            = 0x0001, ///< Exclusive access to this value in the OVERLAY DR
    AJI_CLAIM_OVERLAY_SHARED     = 0x0101, ///< Shared access to this value in the OVERLAY DR
    AJI_CLAIM_OVERLAY_WEAK       = 0x0801  ///< Allow access to this value in OVERLAY DR if unclaimed
                                           ///< (value ~0 means all unclaimed OVERLAY DR values)
};

/**
 * The AJI_CLAIM structure is used to pass in a list of the instructions,
 * overlay values etc which we want to use, either exclusively or shared
 * with other clients.  The type field describes what we are trying to claim.
 *
 * \deprecated AJI_CLAIM is deprecated and replaced by  AJI_CLAIM2. 
 * The deprecated AJI_CLAIM can only access non-hierarchical hub and top-level 
 * hub of a debug fabric with hierarchical hubs.
 */
typedef struct AJI_CLAIM AJI_CLAIM;
struct AJI_CLAIM
{
    AJI_CLAIM_TYPE type;
    DWORD value; ///< instruction, a.k.a. JTAG register
};

/**
 * AJI_CLAIM2 is the new claim that supports access to debug fabric with
 * hierarchical hubs. 
 * AJI_CLAIM2 with AJI_CLAIM2.length == 0 and upper 32 bits
 * AJI_CLAIM2.value == 0 is equivalent to a legacy AJI_CLAIM.
 */
typedef struct AJI_CLAIM2 AJI_CLAIM2;
struct AJI_CLAIM2
{
    AJI_CLAIM_TYPE type;
    DWORD length; ///< internal variable, set to zero
    QWORD value; ///< instruction, a.k.a. JTAG register
};

/**
 * Output Callback Function value classification 
 */
enum OUTPUT_CALLBACK_LEVEL { 
    VOICE_ERROR = 0,
    VOICE_WARNING = 1,
    VOICE_INFO = 2,
    VOICE_DEBUG = 3
};
//# TYPEDEFS which require class declarations //////////////////////////////

//# INLINE FUNCTIONS ///////////////////////////////////////////////////////

//# EXTERN VARIABLE DECLARATIONS ///////////////////////////////////////////

/**
 * \deprecated Obsolete, don't use
 */
enum { AJI_SERVER_VERSION = 0 };

//# FUNCTION PROTOTYPES ////////////////////////////////////////////////////

/**
 * \deprecated Obsolete, don't use
 */
inline AJI_ERROR aji_get_server_version       (DWORD              * version)
    { *version = AJI_SERVER_VERSION; return AJI_NO_ERROR; }

/**
 * Get the last error
 * \return A description of the last error. Empty string if no error had occured 
 */
AJI_API const char * aji_get_error_info       (void);

/*
 * This function returns a list of all the types of hardware which are recognised 
 * by the local server <em>but not currently attached</em>. Functions, such as
 * #aji_add_hardware, can then be used to attach the hardware.
 * 
 * \param hardware_count On input, the number of #AJI_HARDWARE allocated on \p hardware_list. 
 *          On return, the actual number of potential hardware available, this can be larger
 *          then the number of #AJI_HARDWARE allocated on \p hardware_list
 * \param hardware_list On input, a list capable of holding  \p hardware_count #AJI_HARDWARE. 
 *          On return, this list will be filled with the hardware information up to 
 *          \p hardware_count if there is sufficient space.
 *          On return, only AJI_HARDWARE.hw_name and AJI_HARDWARE.port are valid. If
 *          AJI_HARDWARE.port is NULL then user is permitted to pass any value 
 *          as a port string. AJI_HARDWARE.hw_name is suitable for use as identifier in
 *          aji_add_hardware
 * \return #AJI_NO_ERROR  Function was successful
 * \return #AJI_TOO_MANY_DEVICES Insufficient space on #hardware_list to accommodate all
 *                       hardwares. See #hardware_count for the actual number of devices
 * \note You can use QUARTUS_JTAG_CLIENT_CONFIG to configure your server(s) detail
 * 
 * \note Search did not discover any actual top level use of this function in
 * <tt> libaji_client</tt>. As such this function is provided as - is and may not work.
 */
//AJI_ERROR AJI_API aji_get_potential_hardware  (DWORD              * hardware_count,
//                                               AJI_HARDWARE       * hardware_list);

/**
 * This function returns a list of all the hardware devices present
 *  on the local JTAG server and the configured remote JTAG servers.
 *
 * \param hardware_count On calling, the number of #AJI_HARDWARE allocated on \p hardware_list.
 *          On return, the actual number of potential hardware available, this can be larger
 *          then the number of #AJI_HARDWARE allocated on \p hardware_list
 * \param hardware_list On calling, a list capable of holding  \p hardware_count #AJI_HARDWARE.
 *          On return, this list will be filled with the hardware information up to 
 *          \p hardware_count if there is sufficient space.
 *          For the #AJI_HARDWARE returned,only AJI_HARDWARE.hw_name and AJI_HARDWARE.port are valid. If
 *          AJI_HARDWARE.port is NULL then user is permitted to pass any value
 *          as a port string. AJI_HARDWARE.hw_name is suitable for use as identifier in
 *          aji_add_hardware
 * \param timeout Timeout in miliseconds
 * \return #AJI_NO_ERROR  Function was successful
 * \return #AJI_TOO_MANY_DEVICES Insufficient space on #hardware_list to accommodate all
 *                       hardwares. See #hardware_count for the actual number of devices
 * \return #AJI_TIMEOUT Timeout expired before client connected to all servers
 */
AJI_ERROR AJI_API aji_get_hardware            (DWORD              * hardware_count,
                                               AJI_HARDWARE       * hardware_list,
                                               DWORD                timeout = 0x7FFFFFFF);

/**
 * This function returns a list of all the hardware devices present
 *  on the local JTAG server and the configured remote JTAG servers.
 *
 * \param hardware_count On calling, the number of #AJI_HARDWARE allocated on \p hardware_list.
 *          On output, the actual number of potential hardware available, this can be larger
 *          then the number of #AJI_HARDWARE allocated on \p hardware_list
 * \param hardware_list On calling, a list capable of holding  \p hardware_count #AJI_HARDWARE.
 *          On return, this list will be filled with the hardware information up to 
 *          \p hardware_count if there is sufficient space.
 *          For the #AJI_HARDWARE returned, only AJI_HARDWARE.hw_name and AJI_HARDWARE.port are valid. If
 *          AJI_HARDWARE.port is NULL then user is permitted to pass any value
 *          as a port string. AJI_HARDWARE.hw_name is suitable for use as identifier in
 *          aji_add_hardware
 * \param server_version_info_list  On calling, an array of  char* pointers capable of 
 *          holding \p hardware_count server description string.
 *          On return, the list will be filled with the server version information up to 
 *          \p hardware_count if there is sufficient space. Caller to release this list and the
 *          array of pointers.
 * \param timeout Timeout in miliseconds
 * \return #AJI_NO_ERROR  Function was successful
 * \return #AJI_TOO_MANY_DEVICES Insufficient space on #hardware_list to accommodate all
 *                       hardwares. See #hardware_count for the actual number of devices
 * \return #AJI_TIMEOUT Timeout expired before client connected to all servers
 */
AJI_ERROR AJI_API aji_get_hardware2           (DWORD              * hardware_count,
                                               AJI_HARDWARE       * hardware_list,
                                               char             * * server_version_info_list,
                                               DWORD                timeout = 0x7FFFFFFF);
/**
 * This function returns the hardware information for the 
 * hardware with the \p persistent_id specified. 
 * 
 * \param persistent_id The \p persistent_id returned by \p aji_get_hardware -related
 *            functions
 * \param hardware A pointer to an #AJI_HARDWARE structure to be filled in.
 * \param timeout  Timeout in miliseconds
 * 
 * \return #AJI_NO_ERROR Success. \p hardware filled in
 * \return #AJI_FAILURE Failure. No hardware matches the \p persistent_id given
 * \return #AJI_TIMEOUT Failure. Was unable to connect to the correct server before timeout
 *                expired.
 */
AJI_ERROR AJI_API aji_find_hardware           (DWORD                persistent_id,
                                               AJI_HARDWARE       * hardware,
                                               DWORD                timeout);

/**
 *  This function returns the hardware information for the
 * hardware with the \p name specified.
 *
 * \param name user visible name. Example:
 *   -  <type> [ <port>]
 *   -  <type> on <server> [ <port> ]
 *   -  <server> :: <type> [ <port>]
 *   -  <type>
 *   -  <type> on <server>
 *   -  NULL, or an empty string
 * \param hardware A pointer to an #AJI_HARDWARE structure to be filled in.
 * \param timeout  Timeout in miliseconds
 * \return #AJI_NO_ERROR Success. \p hardware filled in
 * \return #AJI_FAILURE Failure. More than one hardware cable exists which matches the name
 * \return #AJI_NO_DEVICES Failure. No hardware matches the \p name given
 * \return #AJI_TIMEOUT Failure. Was unable to connect to the correct server before timeout
 *                expired.
 * \return #AJI_INVALID_PARAMETER Internal error, i.e. hw_name is too small.
 * \see aji_get_hardware
 * \see aji_print_hardware_name
 */
AJI_ERROR AJI_API aji_find_hardware           (const char         * hw_name,
                                               AJI_HARDWARE       * hardware,
                                               DWORD                timeout);
/**
 * Retrieve the hardware name for the hardware identified by \p chain_id
 * 
 * \param chain_id The hardware to retrieve hardware_name from
 * \param hw_name On calling, an char* of capable of holding \p hw_name_len 
 *             characters. On return, the hardware name will be filled in
 * \param hw_name_len length of \p hw_name
 * 
 * \return #AJI_NO_ERROR Success.
 * \return #AJI_INVALID_CHAIN_ID Failure. Hardware with \p chain_id cannot be found
 * \return #AJI_TOO_MANY_DEVICES Failure. Not enough space in \p hw_name, i.e. 
                 \p hw_name_len is too short.
 */
AJI_ERROR AJI_API aji_print_hardware_name     (AJI_CHAIN_ID         chain_id,
                                               char               * hw_name,
                                               DWORD                hw_name_len);

/**
 * Retrieve the hardware name for the hardware identified by \p chain_id
 *
 * \param chain_id The hardware to retrieve hardware_name from
 * \param hw_name On calling, an char* of capable of holding \p hw_name_len
 *             characters. On return, the hardware name will be filled in
 * \param hw_name_len length of \p hw_name
 * \param explicit_localhost If true, and \p hw_name is on \c localhost, 
 *             append "<tt> on localhost</tt>" to \p hw_name on output
 * \param needed_hw_name_len Can be \c NULL, if not \c NULL, 
 *             on return, the length of \hw_name required.
 * \return #AJI_NO_ERROR Success.
 * \return #AJI_INVALID_CHAIN_ID Failure. Hardware with \p chain_id cannot be found
 * \return #AJI_TOO_MANY_DEVICES Failure. Not enough space in \p hw_name, i.e.
                 \p hw_name_len is too short.
 */
AJI_ERROR AJI_API aji_print_hardware_name     (AJI_CHAIN_ID         chain_id,
                                               char               * hw_name,
                                               DWORD                hw_name_len,
                                               bool                 explicit_localhost ,
                                               DWORD              * needed_hw_name_len = NULL);

/**
 * Add a hardawre device.
 * \param hardware Pointer #AJI_HARDWARE device to add. Only #AJI_HARDWARE.hw_name,
 *             #AJI_HARDWARE.port and #AJI_HARDWARE.device_name are used. 
 *             #AJI_HARDWARE.device_name may be \c NULL
 * 
 * \return #AJI_NO_ERROR  Success
 * \return #AJI_UNKNOWN_HARDWARE Failure. The hardware type is not known by the server
 * \return #AJI_INVALID_PARAMETER Failure. There was an invalid parameter
 * \return #AJI_CHAIN_IN_USE Failure. Indicates that the hardware has already
 *                  been added and cannot be added again
 * \return #AJI_BAD_HARDWARE Failure. Indicates that the hardware has been added, but is not 
 *                  currently present so cannot be used.
 * 
 * \note Call #aji_get_hardware again to findout which \c chain_id has been assigned.
 */
AJI_ERROR AJI_API aji_add_hardware            (const AJI_HARDWARE * hardware);

/*
 * Change the \p hardware setting for \p chain_id
 * 
 * \param chain_id The chain_id to change the setting
 * \param hardware The new settings. Currently only permit change of
 *           #AJI_HARDWARE.port
 * 
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_CHAIN_ID Failure. Bad \p chain_id
 * \return #AJI_SERVER_ACTIVE Failure. Server is currently active and cannot be used
 *
 * \note Search did not discover any actual top level use of this function in
 * <tt> libaji_client</tt>. As such this function is provided as - is and may not work.
 */
//AJI_ERROR AJI_API aji_change_hardware_settings(AJI_CHAIN_ID         chain_id,
//                                               const AJI_HARDWARE * hardware);

/**
 * If the hardware device specified by \p chain_id is attached to the 
 * local JTAG server then this function removes it (auto-detected hardware, 
 * identified by #AJI_FEATURE_DYNAMIC, cannot be removed manually).  
 *
 * If the hardware device specified is attached to a remote JTAG server 
 * then this function removes the connection to that JTAG server.
 *
 * \param chain_id The #AJI_CHAIN_ID for the hardware to remove
 * 
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_CHAIN_ID Failure. The \p chain_id is invalid
 * \return #AJI_CHAIN_IN_USE Failure. One or more TAPs on the chain is being used, 
 *                    or the chain has been locked by another client.
*/
AJI_ERROR AJI_API aji_remove_hardware         (AJI_CHAIN_ID         chain_id);

/**
 * Add a connection to the remote JTAG server
 * Equivalent to #aji_add_remote_server(const char*,const char*,bool) 
 * equal to false
 * \param server DNS name or IP address of the server
 * \param password The password to use to authenticate with the server
 * 
 * \return #AJI_NO_ERROR Success.
 * \return #AJI_FILE_ERROR Failure. Information about this server could not be
 *              stored in the registry/configuration file and so will not be
 *              available the next time the program is run
 * \return #AJI_INVALID_PARAMETER Failure. Either
 *                   - \p server or \p password is \c NULL or both. 
 *                   - \p server is actually \c localhost 
 */
AJI_ERROR AJI_API aji_add_remote_server       (const char         * server,
                                               const char         * password);


/**
 * Add a connection to the remote JTAG server
 * Equivalent to #aji_add_remote_server(const char*,const char*,bool)
 * equal to false
 * \param server DNS name or IP address of the server
 * \param password The password to use to authenticate with the server
 * \param temporary Request for a temporary server.
 *
 * \return #AJI_NO_ERROR Success.
 * \return #AJI_FILE_ERROR Failure. Information about this server could not be
 *              stored in the registry/configuration file and so will not be
 *              available the next time the program is run
 * \return #AJI_INVALID_PARAMETER Failure. Either
 *                   - \p server or \p password is \c NULL or both.
 *                   - \p server is actually \c localhost
 */
AJI_ERROR AJI_API aji_add_remote_server       (const char         * server,
                                               const char         * password,
                                               bool                 temporary);
/**
 * This function enables or disables the ability of remote clients 
 * to connect to the local server.
 *
 * \warning These functions are not implemented on Linux/UNIX platforms as 
 *      remote clients aren't supported for servers running on these platforms.
 * 
 * \param enable Whether remote clients will be allowed to connect
 * \param password The password clients must provide to be allowed to
 *             connect to the local server.
 * 
 * \return #AJI_NO_ERROR Success
 * \return #AJI_FILE_ERROR Failure. The information about this server could not be
 *               stored in the registry/configuration file and so will 
 *               not be available next time the program is run
 */
AJI_ERROR AJI_API aji_enable_remote_clients   (bool                 enable,
                                               const char         * password);

/*
 * This function returns whether or not remote connections 
 * to this server are allowed.
 * 
 * \param enable On return, indicates whether remote connections
 *            are allowed
 * 
 * \return #AJI_NO_ERROR Success
 *
 * \note Search did not discover any actual top level use of this function in
 * <tt> libaji_client</tt>. As such this function is provided as - is and may not work.
 */
AJI_ERROR AJI_API aji_get_remote_clients_enabled(bool             * enable);

/**
 * Allows unknown devices with possibly unknown instruction 
 * register lengths to be defined. In contrast to
 * #aji_define_device(AJI_CHAIN_ID,DWORD,const AJI_DEVICE*), it does not
 * affect any claims which are currently in use as it just adds information
 * about the device to the JTAG server's user device database
 * 
 * \param device The device to be added. 
 *         #AJI_DEVICE.instruction_length must be valid.
 *         #AJI_DEVICE.device_name may be \p NULL
 *         If bit 0 of #AJI_DEVICE.device_id
 *         is set, than #AJI_DEVICE.device_id is a JTAG IDCODE.
 *         This function adds information about the JTAG device
 *         with the JTAG IDCODE to the database and will be used 
 *         by JTAG Server later. If bit 0 is zero
 *         it simply add a device to the device database but 
 *         it cannot be used by the JTAG server later.
 * 
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_PARAMETER One or more value in \p device
 *              is not valid
 *
 */
AJI_ERROR AJI_API aji_define_device           (const AJI_DEVICE   * device);

/**
 * Removes a device from the JTAG server's 
 * user device database. 
 * 
 * \param device The device to be removed. Its content must
 *            match a device already in the database for the 
 *            function to have any effect
 * 
 * \return #AJI_NO_ERROR Success
 */
AJI_ERROR AJI_API aji_undefine_device         (const AJI_DEVICE   * device);

/**
 * Gets a list of all devices from all connected JTAG servers'
 *  user device databases.
 * 
 * \param device_count On calling, the number of elements in \p device_list
 *           On return, the actual number of devices. The returned value can be
 *           greater than the input value
 * \param device_list A list of size \p device_count to hold the information about
 *            devices. On output, up to \p device_count will be filled if the
 *            returned \p device_count value is same as or smaller than the capacity
 *            of this list.
 * 
 * \return #AJI_NO_ERROR Success
 * \return #AJI_TOO_MANY_DEVICES Failure. \p device list is too small, see returned
 *                   \p device_count value to see how many elements are needed.
 */
AJI_ERROR AJI_API aji_get_defined_devices     (DWORD              * device_count,
                                               AJI_DEVICE         * device_list);

//AJI_ERROR AJI_API aji_get_local_quartus_devices(DWORD             * device_count,
//                                               AJI_DEVICE         * device_list);

/**
 * Return the list of #AJI_DEVICE in the device chain specified by
 * \p chain_id.
 * 
 * \pre \p chain_id must be locked.
 * 
 * \param chain_id the hardware to scan. This is returned by #aji_get_hardware
 * \param device_count On calling, the number of #AJI_DEVICE available in \p records
 *           On return, the number of records available. Note that this value
 *           can be greater than the allocated \p records size if the function
 *           returned #AJI_TOO_MANY_DEVICES
 * \param device_list On calling, has allocated \p device_count 
 *            #AJI_DEVICE for storing#AJI_DEVICE data. 
 *            On return, if returned #AJI_NO_ERROR,
 *            \p device_count records would had been filled;
 *            The device in \p device_count[0]
 *            is the device closest to \p TDI; 
 *           
 * \param auto_scan If true, requests that the chain should be scanned if need be.
 * 
 * \return #AJI_NO_ERROR Success. However, it is not guaranteed that the full
 *            chain of \p chain_id is known. If it is only partially known, the
 *            corresponding unknwon field in #AJI_DEVICE will not be filled. 
 *            For example, if the instruction register length is unknown for
 *            a device, then #AJI_DEVICE.instruction_length is zero.
 * \return #AJI_INVALID_CHAIN_ID Failure. Invalid \p chain_id
 * \return #AJI_NOT_LOCKED  Failure. \p chain_id has not been configured
 *            and \p auto_scan was false
 * \return #AJI_TOO_MANY_DEVICES. Insufficient space has been reserved for
 *            \p device_list. The required space is return in #device_count
 */
AJI_ERROR AJI_API aji_read_device_chain       (AJI_CHAIN_ID         chain_id,
                                               DWORD              * device_count,
                                               AJI_DEVICE         * device_list,
                                               bool                 auto_scan = true);

/**
 * Maximum size for get/set_parameter block
 */
enum { AJI_PARAMETER_MAX = 3072 };

/**
 * Set a chain-specific parameter. Not all chains support all parameter values so
 * some knowledge about the type of hardware in use will be required before this function
 * can be used
 * 
 * \pre \p chain_id must be locked.
 *
 * \param chain_id  The \p chain_id to set the parameter. See #aji_get_hardware
 * \param name The name of the parameter to set
 * \param value The value to set to
 * 
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_CHAIN_ID Failure. \p chain_id is not valid
 * \return #AJI_NOT_LOCKED Failure. \p chain_id is not locked
 * \return #AJI_UNIMPLEMENTED Failure. The parameter name is not 
                  implemented on this chain
 */
AJI_ERROR AJI_API aji_set_parameter           (AJI_CHAIN_ID         chain_id,
                                               const char         * name,
                                               DWORD                value);
/**
 * Get the current value for the parameter \p name from \p chain_id
 *
 * \pre \p chain_id must be locked.
 * 
 * \param chain_id The \p chain_id to get the parameter from
 * \param name The parameter name
 * \param value A location to store the parameter
 * 
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_CHAIN_ID Failure. \p chain_id is not valid
 * \return #AJI_NOT_LOCKED Failure. \p chain_id is not locked
 * \return #AJI_UNIMPLEMENTED Failure. The parameter name is not
 *                 implemented on this chain
 */
AJI_ERROR AJI_API aji_get_parameter           (AJI_CHAIN_ID         chain_id,
                                               const char         * name,
                                               DWORD              * value);

/**
 * Get the current value for the parameter \p name from \p chain_id
 *
 * \pre \p chain_id must be locked. 
 *
 * \param chain_id The \p chain_id to get the parameter from
 * \param name The parameter name
 * \param value On calling,  \p value itself is of size 
 *                 \p valuemax. It contains data of size 
 *                 \p valuetx to be transmitted to the
 *                 server. The value returned by the server 
 *                 is written on top of this value. This
 *                 has the effect of initializing the
 *                 return value buffer with \p value on
 *                 the server.
 *              On return, contain the value retrieved from
 *                  the server for \p chain_id, whose size is
 *                  stored in \p valuemax. If the returned
 *                  \p valuemax value is less than
 *                  \p valuetx, then value in location 
 *                  w
 * \param valuemax On calling, size of \p value allocated.
 *                 On return, the size of \p value returned.
 *                 Must not exceed #AJI_PARAMETER_MAX 
 * \param valuetx  The size of \p value to be transmitted to the
 *                  server. Can be zero. If not zero, must not
 *                  exceed \p valuemax
 *
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_CHAIN_ID Failure. \p chain_id is not valid
 * \return #AJI_NOT_LOCKED Failure. \p chain_id is not locked
 * \return #AJI_UNIMPLEMENTED Failure. The parameter name is not
 *                 implemented on this chain
 */
AJI_ERROR AJI_API aji_get_parameter           (AJI_CHAIN_ID         chain_id,
                                               const char         * name,
                                               BYTE               * value,
                                               DWORD              * valuemax,
                                               DWORD                valuetx = 0);

/**
 * Return the \p open_id for the #AJI_DEVICE, a.k.a. JTAG TAP, at
 * position \p tap_position of the JTAG Chain \p chain_id. With AJI,
 * you need to specify how which Instruction Registers you want to
 * access and do you want to share it with other AJI client via
 * \p claims. \p open_id is used to access the #AJI_DEVICE (TAP).
 * 
 * \pre \p chain_id must be locked using #aji_lock_chain and had been
 *      configured
 * 
 * \deprecated Use #AJI_CLAIM2 version instead, i.e.
 *    #aji_open_device(AJI_CHAIN_ID,DWORD,AJI_OPEN_ID*,const AJI_CLAIM2*,DWORD,const char*).
 *    See #AJI_CLAIM2 on how to convert a #AJI_CLAIM to #AJI_CLAIM2
 * 
 * \param chain_id  The JTAG Chain to use
 * \param tap_position The position of TAP in \p chain_id. Zero-indexed
 * \param open_id  On return, the #AJI_OPEN_ID for the TAP at  
 *           \p tap_position on JTAG chain \p chain_id. The value 0
 *           is never used.
 * \param claims An array of size \p claims_n, containing
 *            The instruction registers this client will be accessing,
 *            and whether they can be shared with other clients
 * \param claim_n The number of instructions claimed in \p claims
 * \param application_name A string to identify this client.
 * 
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_CHAIN_ID Failure. \p chain_id is not valid
 * \return #AJI_NOT_LOCKED Failure. \p chain_id is not locked
 * \return #AJI_CHAIN_NOT_CONFIGURED Failure. \p chain_id has not been configured
 * \return #AJI_BAD_TAP_POSITION Failure. There is no TAP at \p tap_position 
 * \return #AJI_DEVICE_NOT_CONFIGURED Failure. The TAP at \p tap_position 
 *                has not been defined
 * \return #AJI_CHAINS_CLAIMED One or more of the requested #AJI_CLAIM in
 *               \p claims are already taken.
 */
AJI_ERROR AJI_API aji_open_device             (AJI_CHAIN_ID         chain_id,
                                               DWORD                tap_position, 
                                               AJI_OPEN_ID        * open_id,
                                               const AJI_CLAIM    * claims,
                                               DWORD                claim_n,
                                               const char         * application_name);


/**
 * Return the \p open_id for the #AJI_DEVICE, a.k.a. JTAG TAP, at
 * position \p tap_position of the JTAG Chain \p chain_id. With AJI,
 * you need to specify how which Instruction Registers you want to
 * access and do you want to share it with other AJI client via
 * \p claims. \p open_id is used to access the #AJI_DEVICE (TAP).
 *
 * \pre \p chain_id must be locked using #aji_lock_chain and had been
 *      configured
 * 
 * \bug There is a sighting that this function will failed if the
 *        shared library (lib.so or DLL) was loaded via 
 *        \c dlopen()/dlsym() on Linux and the corresponding
 *        \c LoadLibrary()/GetProcAddr() in Windows. If you
 *        experience this, please use the #AJI_CLAIM version, 
 *        #aji_open_device(AJI_CHAIN_ID,DWORD,AJI_OPEN_ID*,const AJI_CLAIM*,DWORD,const char*)
 *        instead
 * 
 * \param chain_id  The JTAG Chain to use
 * \param tap_position The position of TAP in \p chain_id. Zero-indexed
 * \param open_id  On return, the #AJI_OPEN_ID for the TAP at
 *           \p tap_position on JTAG chain \p chain_id. The value 0
 *           is never used.
 * \param claims An array of size \p claims_n, containing
 *            The instruction registers this client will be accessing,
 *            and whether they can be shared with other clients
 * \param claim_n The number of instructions claimed in \p claims
 * \param application_name A string to identify this client.
 *
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_CHAIN_ID Failure. \p chain_id is not valid
 * \return #AJI_NOT_LOCKED Failure. \p chain_id is not locked
 * \return #AJI_CHAIN_NOT_CONFIGURED Failure. \p chain_id has not been configured
 * \return #AJI_BAD_TAP_POSITION Failure. There is no TAP at \p tap_position
 * \return #AJI_DEVICE_NOT_CONFIGURED Failure. The TAP at \p tap_position
 *                has not been defined
 * \return #AJI_CHAINS_CLAIMED One or more of the requested #AJI_CLAIM in
 *               \p claims are already taken.
 */
AJI_ERROR AJI_API aji_open_device             (AJI_CHAIN_ID         chain_id,
                                               DWORD                tap_position,
                                               AJI_OPEN_ID        * open_id,
                                               const AJI_CLAIM2   * claims,
                                               DWORD                claim_n,
                                               const char         * application_name);

/**
 * Release instruction and scan chains claimed by 
 * #aji_open_device, #aji_open_node or #aji_open_hub.
 * 
 * \pre \p open_id must not be locked
 * \param open_id The #AJI_OPEN_ID to be released.
 * 
 * \return #AJI_NO_ERROR Success
 * \return AJI_INVALID_OPEN_ID Failure. The \p open_id is not valid
 * \return AJI_LOCKED  Failure. The \p open_id is still locked
 */
AJI_ERROR AJI_API aji_close_device            (AJI_OPEN_ID          open_id);

/**
 * Return an #AJI_OPEN_ID, \p open_id, that claims the whole device chain 
 * so that a client can manipulate the whole chain. 
 * The server does not modify the data so the client 
 * must know which devices are present on the chain.
 *
 * If the chain is opened in passive serial mode then 
 * the only operation which is permitted is serial download.
 *
 * If the whole chain is opened in JTAG mode then most 
 * JTAG operations are permitted,, however other clients will 
 * not be able to open the chain therefore this function 
 * should only be used if the sharable JTAG operations are not suitable.
 * 
 * \pre \p chain_id is not locked and no other client chaimed any
 * #AJI_DEVICE (TAP) on this chain.
 * 
 * 
 * \param chain_id The JTAG Chain to be claimed
 * \param open_id On return, the #AJI_OPEN_ID that represents the
 *           whole chain in \p chain_id
 * \param style  The mode to open the chain with. Can be
 *           #AJI_CHAIN_JTAG, #AJI_CHAIN_PASSIVE or
 *           #AJI_CHAIN_ACTIVE
 * \param application_name A null-terminated string identifying 
 *           this client.
 * 
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_CHAIN_ID Failure. \p chain_id is invalid
 * \return #AJI_NOT_LOCKED Failure. \c chain_id is not locked
 * \return #AJI_CHAIN_IN_USE Failure. Another client has already
 *            opened the JTAG chain or at least one device in the 
 *            chain is used by another client
 * \return #AJI_UNIMPLEMENTED Failure. The JTAG server has 
 *            not implemented this function.
 */
AJI_ERROR AJI_API aji_open_entire_device_chain(AJI_CHAIN_ID         chain_id,
                                               AJI_OPEN_ID        * open_id,
                                               AJI_CHAIN_TYPE       style,
                                               const char         * application_name);

/**
 * Locks the device/ device chain represented by \p open_id.
 * This prevents other clients that has access to the chain. 
 * Therefore this should be used for as brief a period as possible, 
 * as it denies access to other clients. 
 * The calling application should call #aji_unlock as soon as it 
 * no longer requires locked access to the chain. 
 *
 * After successful completion of a lock function the chain 
 * will be in the \c Update-IR or \c Update-DR state and the values 
 * in the instruction and overlay (if present) registers will be 
 * the preserved from the last call to unlock (or will be undefined 
 * if this is the first call to lock since opening the device)
 *
 * If the chain has already been locked by another then 
 * the function will block for up to the specified timeout before 
 * returning #AJI_CHAIN_IN_USE.
 * 
 * \param open_id The chain to lock
 * \param timeout Timeout in miliseconds
 * \param pack_style Specifies when the client is allowed to 
 *         delay sending requests (including this one) to the server.
 *         One of #AJI_PACK_NEVER, #AJI_PACK_AUTO or #AJI_PACK_MANUAL
 *         expected.
 * 
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_CHAIN_ID Failure. \p chain_id is invalid
 * \return #AJI_NOT_LOCKED Failure. \c chain_id is not locked
 * \return #AJI_BAD_HARDWARE Failure. Hardware for the chained had failed
 * \return #AJI_CHAIN_IN_USE Failure. Another client has already
 *            opened the JTAG chain
 */
AJI_ERROR AJI_API aji_lock                    (AJI_OPEN_ID          open_id,
                                               DWORD                timeout,
                                               AJI_PACK_STYLE       pack_style);

/**
 * This function will unlock the device or device chain. 
 * This function should be called to cancel the matching lock function, 
 * to allow other applications to access the JTAG device chain again. T
 
 * \param open_id The device chain to unlock. Must matched the same value
 *          passed by #aji_lock and matching functions by the same client. 
 * 
 * \return #AJI_NO_ERROR Success
 * \return #AJI_NOT_LOCKED The device (TAP) was not locked by this client
 */
AJI_ERROR AJI_API aji_unlock                  (AJI_OPEN_ID          open_id);

/**
 * Atomically (without the possibility of other clients interposing 
 * commands) unlocks the device \p unlock_id from the controller which 
 *  has it locked and locks device \p lock_id
 * 
 * This function checks that the TAP controller is in a recognised state, 
 * i.e. \c Run-Test-Idle, \c Update-IR, \c Update-DR or \c Select-DR-Scan 
 * and if not moves the state machine to one of these states.
 * 
 * Its execution will be deferred if #AJI_PACK_AUTO or 
 * #AJI_PACK_MANUAL have been specified.
 * 
 * \param unlock_id The device to unlock. Must be locked with #aji_lock
 *                    It will not unlock device_chain locked with #aji_lock_chain
 * \param lock_id The device to lock
 *
 * \return #AJI_NO_ERROR Success
 * \return #AJI_NOT_LOCKED The device (TAP) was not locked by this client
 * 
 * \seealso #aji_unlock_chain_lock
 *
 */
AJI_ERROR AJI_API aji_unlock_lock             (AJI_OPEN_ID          unlock_id,
                                               AJI_OPEN_ID          lock_id);

/**
 * Atomically (without the possibility of other clients interposing
 * commands) unlocks the device \p unlock_id from the controller which
 *  has it locked and locks device chain \p lock_id
 *
 * This function checks that the TAP controller is in a recognised state,
 * i.e. \c Run-Test-Idle, \c Update-IR, \c Update-DR or \c Select-DR-Scan
 * and if not moves the state machine to one of these states.
 *
 * Its execution will be deferred if #AJI_PACK_AUTO or
 * #AJI_PACK_MANUAL have been specified.
 *
 * \param unlock_id The device chain to unlock. Must be locked with #aji_lock_chain.
 *            It will not unlock device_chain locked with #aji_lock
 * \param lock_id The device to lock
 * \param pack_style An indication of how this command, and subsequent commands
 *             should be packed together before being sent to the server.
 *
 * \return #AJI_NO_ERROR Success
 * \return #AJI_NOT_LOCKED The device (TAP) was not locked by this client
 * 
 * \seealso #aji_unlock_lock
 */
AJI_ERROR AJI_API aji_unlock_chain_lock       (AJI_CHAIN_ID         unlock_id,
                                               AJI_OPEN_ID          lock_id,
                                               AJI_PACK_STYLE       pack_style);


/**
 * Atomically (without the possibility of other clients interposing
 * commands) unlocks the device or device_chain \p unlock_id from the controller which
 *  has it locked and locks device chain \p lock_id
 *
 * This function checks that the TAP controller is in a recognised state,
 * i.e. \c Run-Test-Idle, \c Update-IR, \c Update-DR or \c Select-DR-Scan
 * and if not moves the state machine to one of these states.
 *
 * Its execution will be deferred if #AJI_PACK_AUTO or
 * #AJI_PACK_MANUAL have been specified.
 *
 * \param unlock_id The device/device_chain to unlock. It must be locked with #aji_lock.
 *            It will not unlock device_chain locked with #aji_lock
 * \param lock_id The device chain to lock
 * \param pack_style An indication of how this command, and subsequent commands
 *             should be packed together before being sent to the server.
 *
 * \return #AJI_NO_ERROR Success
 * \return #AJI_NOT_LOCKED The device (TAP) was not locked by this client
 *
 */
AJI_ERROR AJI_API aji_unlock_lock_chain       (AJI_OPEN_ID          unlock_id,
                                               AJI_CHAIN_ID         lock_id);

/**
 * Ensures that the server has executed all function calls 
 * and has made all results valid.  If there are no queued commands associated
 * with this TAP controller then the function does nothing.
 *
 * The TAP controller for open_id must be locked with #aji_lock otherwise 
 * this function will return #AJI_NOT_LOCKED.
 *
 * Its execution  is never deferred since its purpose is 
 * to cause deferred commands to be executed.
 * 
 * \param open_id The TAP (#AJI_DEVICE) to flush
 * \return #AJI_NO_ERROR Success
 * \return #AJI_NOT_LOCKED The device(TAP) was not locked by this client
 *
*/
AJI_ERROR AJI_API aji_flush                   (AJI_OPEN_ID          open_id);

/**
 * Obtain temporary exclusive access to a device chain before 
 * performing actions which will affect the whole chain 
 * (for example to open a connection to a device).  
 * 
 * \param chain_id The chain_id for the hardware (JTAG Chain)
 * \param timeout Timeout in miliseconds
 * 
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_CHAIN_ID Failure. \p chain_id is not valid
 * \return #AJI_CHAIN_LOCKED Failure. \p chain_id is already locked by this client
 * \return #AJI_CHAIN_IN_USE Failure. \p chain_id is locked by another client
 * 
 * \seealso #aji_unlock_chain
 */
AJI_ERROR AJI_API aji_lock_chain              (AJI_CHAIN_ID         chain_id,
                                               DWORD                timeout);

/**
 * Unlock the device chain specified by \p chain_id. 
 * This function should be called to cancel an #aji_lock_chain, 
 * to allow other applications to access the JTAG device chain again. 
 *
 * \param chain_id JTAG Chain to unlock. Must be locked using #aji_lock_chain 
 *            by the same client.
 * 
 * \return AJI_NO_ERROR Success
 * \return AJI_INVALID_CHAIN_ID Failure. \p chain_id is not valid
 * \return AJI_NOT_LOCKED Failure.  \p chain_id is not locked by this client
 */
AJI_ERROR AJI_API aji_unlock_chain            (AJI_CHAIN_ID         chain_id);

/**
 * This function writes an instruction to the IR register of the TAP controller 
 * specified by \p open_id. 
 * 
 * \pre The TAP controller for the scan chain must be in 
 * \c Run-Test-Idle, \c Update-IR, \c Update-DR 
 * or \c Select-DR-Scan before this function is called 
 * \pre \p open_id must be locked with #aji_lock
 * 
 * \param open_id The #AJI_OPEN_ID for the TAP. Cannot be used for #AJI_OPEN_ID
 *                that represents the whole chain, e.g. those returned 
 *                by #aji_open_entire_device_chain
 * \param instruction The IR register to access. The instruction is taken from 
 *                the least significant bits of instruction.
 * \param capture_ir A pointer to the location in which to store the value
 *                caputred into IR
 * \param flags Not used. Set to zero
 * 
 * \post The TAP controller for the scan chain will be left in \p Update-IR
 *
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_OPEN_ID Failure. \p open_id is not valid
 * \return #AJI_BAD_TAP_STATE Failure. The TAP controller is not in
 *              \c Run-Test-Idle, \c Update-IR, \c Update-DR 
 *              or \c Select-DR-Scan before this function is called 
 * \return #AJI_NOT_LOCKED Failure. TAP \p open_id is not locked by this client
 * \return #AJI_INVALID_PARAMETER Failure. \p instruction is not valid 
 *               for the select TAP \p open_id
 * \return #AJI_INSTRUCTION_CLAIMED Failure. \p Instruction has been claimed
 *               by another client, or was not claimed when you open this device
 * \return #AJI_BAD_HARDWARE Failure. The hardware for the chain has failed
 * 
 * \note The execution of aji_access_ir can be deferred if #AJI_PACK_AUTO has been
 *   specified and ( \p captured_ir \p == \p NULL) or if #AJI_PACK_MANUAL
 *   has been specified.  It will not be deferred if the IR value specified
 *   was not claimed when the device was opened.

 */
AJI_ERROR AJI_API aji_access_ir               (AJI_OPEN_ID          open_id,
                                               DWORD                instruction,
                                               DWORD              * captured_ir,
                                               DWORD                flags = 0);

/**
 * This function writes an instruction to the IR register of the TAP controller
 * specified by \p open_id.
 *
 * \pre The TAP controller for the scan chain must be in
 * \c Run-Test-Idle, \c Update-IR, \c Update-DR
 * or \c Select-DR-Scan before this function is called
 * \pre \p open_id must be locked with #aji_lock
 *
 * \param open_id The #AJI_OPEN_ID for an individual TAP or the whole scan chain
 * \param length The length of The IR register to access, in bits. It is
 *               validated against that  known by JTAG Server. 
 *               However, if \c open_id the whole chain, then no validation
 *               is done and \c length must be the correct length.
 * \param write_data The data to be written into the IR register. Must be of
 *          length \c length. The first bit wrriten is the least signiciant bit of
 *          <tt> write_data[0]</tt> followed by the next signicant bit of the same byte etc.
 *          After the first byte is written the least significant bit of <tt> write_data[1]</tt>
 *          is written etc
 * \param read_data The location where value captured from IR should be stored
 *          Can be \c NULL. Must be of length \c length if in use. Like 
 *          \c write_data, the data is filled starting with the least significant
 *          bit of \c read_data[0], and after it is filled, least significant
 *          bit of \c read_data[1] will be next etc.
 * \param flags Not used. Set to zero
 * 
 * \post The TAP controller for the scan chain will be left in \p Update-IR
 *
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_OPEN_ID Failure. \p open_id is not valid
 * \return #AJI_BAD_TAP_STATE Failure. The TAP controller is not in
 *              \c Run-Test-Idle, \c Update-IR, \c Update-DR
 *              or \c Select-DR-Scan before this function is called
 * \return #AJI_NOT_LOCKED Failure. TAP \p open_id is not locked by this client
 * \return #AJI_INVALID_PARAMETER Failure. \p instructoin is not valid
 *               for the select TAP \p open_id
 * \return #AJI_INSTRUCTION_CLAIMED Failure. \p Instruction has been claimed
 *               by another client, or was not claimed when you open this device
 * \return #AJI_BAD_HARDWARE Failure. The hardware for the chain is had failed
 * \return #AJI_UNIMPLEMENTED Failure. Not implemented by this version of server
 * 
 * \note The execution of aji_access_ir can be deferred if #AJI_PACK_AUTO has been
 *   specified and ( \p read+data \p == \p NULL) or if #AJI_PACK_MANUAL
 *   has been specified.  It will not be deferred if the IR value specified
 *   was not claimed when the device was opened.

 */
AJI_ERROR AJI_API aji_access_ir               (AJI_OPEN_ID          open_id,
                                               DWORD                length_ir,
                                               const BYTE         * write_bits,
                                               BYTE               * read_bits,
                                               DWORD                flags = 0);

/**
 * Reads and/or writes data to the TAP controller or the whole scan chain 
 * identified by \p open_id
 * 
 * \pre The TAP controller for the scan chain must be in
 * \c Run-Test-Idle, \c Update-IR, \c Update-DR
 * or \c Select-DR-Scan before this function is called
 * \pre \p open_id must be locked with #aji_lock
 *
 * \param open_id The #AJI_OPEN_ID for an individual TAP or the whole scan chain
 * \param length_dr The length of the selected data register, or the data
 *          registers' length for the whole scan chain.
 * \param flags Indicates how the server may optimise the scan.
 *          Permitted values are #AJI_DR_UNUSED_0, #AJI_DR_UNUSED_0_OMIT,
 *          #AJI_DR_UNUSED_X, #AJI_DR_NO_SHORT and #AJI_DR_NO_RESTORE
 * \param write_offset The offset of the scan chain to be changed. All
 *          other bits are recirculated so that they are left unchanged
 * \param write_length The number of bits to be written into the data register(s)
 * \param write_bits A pointer to the bits to be written to the selected data
 *          register(s). The bits are replaced by \p write_offset.
 *          The first bit writen is the least signiciant bit of
 *          <tt> write_bits[0]</tt> followed by the next signicant bit of the same byte etc.
 *          After the first byte is written the least significant bit of <tt> write_bits[1]</tt>
 *          is written etc. 
 *          Can be \p NULL if no bit is to be written
 * \param read_offset The offset of the scan chain to be read from
 * \param read_length The number of bits to be written into the data register(s)
 * \param read_bits The location where value captured from IR should be stored.
 *          The start of this output is offset by \read_offset, allowing
 *          part of a register to be read
 *          Like \c write_bits, the data is filled starting with the least significant
 *          bit of <tt>read_bits[0]</tt>, and after it is filled, least significant
 *          bit of <tt>read_bits[1]</tt> will be next etc.
 *          Can be \p NULL if no bit is to be read
 *
 * \post The TAP controller for the scan chain will be in the \p Update-DR state
 * 
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_OPEN_ID Failure. \p open_id is not valid
 * \return #AJI_NOT_LOCKED Failure. TAP \p open_id is not locked by this client
 * \return #AJI_BAD_TAP_STATE Failure. The TAP controller is not in
 *            \c Run-Test-Idle, \c Update-IR, \c Update-DR
 *            or \c Select-DR-Scan before this function is called
 * \return #AJI_IR_MULTIPLE Failure. The last call to #aji_access_ir loaded non-BYPASS
 *            instruction into more than one TAP
 * \return #AJI_INVALID_PARAMETER Failure. Failure. One or more parameters 
 *            are invalid
 * \return #AJI_BAD_HARDWARE Failure. The hardware for the chain is had failed
 *
 * \note The execution of aji_access_ir can be deferred if #AJI_PACK_AUTO has been
 * specified and ( \p read_length \p == \p NULL) or if #AJI_PACK_MANUAL
 * has been specified. In the latter case, caller must ensure \p read_bits
 * remains valid until command is completed. As \p write_data buffer will be
 * copied, this restriction does not apply to it.
 */
AJI_ERROR AJI_API aji_access_dr               (AJI_OPEN_ID          open_id,
                                               DWORD                length_dr,
                                               DWORD                flags,
                                               DWORD                write_offset,
                                               DWORD                write_length,
                                               const BYTE         * write_bits,
                                               DWORD                read_offset,
                                               DWORD                read_length,
                                               BYTE               * read_bits);

/**
 * Reads and/or writes data to the TAP controller or the whole scan chain
 * identified by \p open_id \p batch number of times.
 *
 * If the \p batch parameter is provided, and greater than one, 
 *  then batch scans will be performed - all scans use the same parameter 
 *  values except for \p write_bits and \p read_bits.  
 *  The second and subsequent scans increment the pointers by the minimum number of 
 *  whole bytes to move on to new data, so if \p write_length == 3 then \p write_bits will be 
 *  incremented by one byte between scans.  If \p write_length == 18 then 
 *  \p write_data will be incremented by 3 bytes.
 *
 * \pre The TAP controller for the scan chain must be in
 * \c Run-Test-Idle, \c Update-IR, \c Update-DR
 * or \c Select-DR-Scan before this function is called
 * \pre \p open_id must be locked with #aji_lock

 * \param open_id The #AJI_OPEN_ID for an individual TAP or the whole scan chain
 * \param length_dr The length of the selected data register, or the data
 *          registers' length for the whole scan chain.
 * \param flags Indicates how the server may optimise the scan.
 *          Permitted values are #AJI_DR_UNUSED_0, #AJI_DR_UNUSED_0_OMIT,
 *          #AJI_DR_UNUSED_X, #AJI_DR_NO_SHORT and #AJI_DR_NO_RESTORE
 * \param write_offset The offset of the scan chain to be changed. All
 *          other bits are recirculated so that they are left unchanged
 * \param write_length The number of bits to be written into the data register(s)
 * \param write_bits A pointer to the bits to be written to the selected data
 *          register(s). The bits are replaced by \p write_offset.
 *          See description on the actual size of \p write_bits required.
 *          The first bit writen is the least signiciant bit of
 *          \c write_bits[0] followed by the next signicant bit of the same byte etc.
 *          After the first byte is written the least significant bit of write_bits[1]
 *          is written etc. 
 *          Can be \p NULL if no bit is to be written
 * \param read_offset The offset of the scan chain to be read from
 * \param read_length The number of bits to be written into the data register(s)
 * \param read_bits The location where value captured from IR should be stored.
 *          The start of this output is offset by \p read_offset, allowing
 *          part of a register to be read. 
 *           See description on the actual size of \p read_bits required.
 *          Like \c write_bits, the data is filled starting with the least significant
 *          bit of <tt>read_bits[0]</t>, and after it is filled, least significant
 *          bit of <tt>read_bits[1]</t> will be next etc.
 *          Can be \p NULL if no bit is to be read
 * \batch  The number of times to repeat this function.
 *
 * \post The TAP controller for the scan chain will be in the \p Update-DR state
 *
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_OPEN_ID Failure. \p open_id is not valid
 * \return #AJI_NOT_LOCKED Failure. TAP \p open_id is not locked by this client
 * \return #AJI_BAD_TAP_STATE Failure. The TAP controller is not in
 *            \c Run-Test-Idle, \c Update-IR, \c Update-DR
 *            or \c Select-DR-Scan before this function is called
 * \return #AJI_IR_MULTIPLE Failure. The last call to #aji_access_ir loaded non-BYPASS
 *            instruction into more than one TAP
 * \return #AJI_INVALID_PARAMETER Failure. Failure. One or more parameters
 *            are invalid
 * \return #AJI_BAD_HARDWARE Failure. The hardware for the chain is had failed
 *
 * \note The execution of aji_access_ir can be deferred if #AJI_PACK_AUTO has been
 * specified and ( \p read_length \p == \p NULL) or if #AJI_PACK_MANUAL
 * has been specified. In the latter case, caller must ensure \p read_bits
 * remains valid until command is completed. As \p write_data buffer will be
 * copied, this restriction does not apply to it.
 */
AJI_ERROR AJI_API aji_access_dr               (AJI_OPEN_ID          open_id,
                                               DWORD                length_dr,
                                               DWORD                flags,
                                               DWORD                write_offset,
                                               DWORD                write_length,
                                               const BYTE         * write_bits,
                                               DWORD                read_offset,
                                               DWORD                read_length,
                                               BYTE               * read_bits,
                                               DWORD                batch);

/**
 * Puts the TAP controller into the \p Run-Test-Idle state and 
 * ensures that it runs through this state \p num_clocks times.
 * The JTAG server will often defer one of the clocks until 
 * the next instruction to reduce the number of extra state 
 * transitions required
 * 
 * \pre The TAP controller for the scan chain must be in
 *       \c Run-Test-Idle, \c Test-Logic-Reset,
 *       \c Update-IR, \c Update-DR
 * \pre \p open_id must be locked with #aji_lock
 *
 * \param open_id The TAP whose scan chain's TAP controller
 *                 is to be put into \p Run-Test-Idle state
 * \param num_clocks The number of times to run through 
 *                 the \p Run-Test-Idle state
 * 
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_OPEN_ID Failure. \p open_id is not valid
 * \return #AJI_NOT_LOCKED Failure. TAP \p open_id is not locked by this client
 * \return #AJI_BAD_TAP_STATE Failure. The TAP controller is not in
 *            \c Run-Test-Idle, \c Test-Logic-Reset, \c Update-IR, \c Update-DR
 *            or \c Select-DR-Scan before this function is called
 * \return #AJI_BAD_HARDWARE Failure. The hardware for the chain is had failed
 */
AJI_ERROR AJI_API aji_run_test_idle           (AJI_OPEN_ID          open_id,
                                               DWORD                num_clocks);

/** 
 * Put the TAP controller (and all other TAP controllers 
 * on the same chain) to go to \p Run-Test-Idle and
 * ensures that it runs through this state \p num_clocks times.
 * The JTAG server will often defer one of the clocks until
 * the next instruction to reduce the number of extra state
 * transitions required
 *
 * \pre The TAP controller for the scan chain must be in
 *       \c Run-Test-Idle, \c Test-Logic-Reset,
 *       \c Update-IR, \c Update-DR
 * \pre \p open_id must be locked with #aji_lock
 *
 * \param open_id The TAP whose scan chain's TAP controller
 *                 is to be put into \p Run-Test-Idle state
 * \param num_clocks The number of times to run through
 *                 the \p Run-Test-Idle state
 * \param flags #AJI_EXIT_RTI_STATE or #AJI_ACCURATE_CLOCK or 0
 *
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_OPEN_ID Failure. \p open_id is not valid
 * \return #AJI_NOT_LOCKED Failure. TAP \p open_id is not locked by this client
 * \return #AJI_BAD_TAP_STATE Failure. The TAP controller is not in
 *            \c Run-Test-Idle, \c Update-IR, \c Update-DR
 *            or \c Select-DR-Scan before this function is called
 * \return #AJI_BAD_HARDWARE Failure. The hardware for the chain is had failed
 */
    AJI_ERROR AJI_API aji_run_test_idle           (AJI_OPEN_ID          open_id,
                                               DWORD                num_clocks,
                                               DWORD                flags);

/**
 * Put the TAP controller (and all other TAP controllers 
 * on the same chain) to go to \p Test-Logic-Reset. 
 * This function is only useful for resetting the TAP 
 * \param open_id The TAP whose scan chain's TAP controller
 * is to be put into \p Test-Logic-Reset state
 *  
 * \pre \p open_id must be locked with #aji_lock
 *
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_OPEN_ID Failure. \p open_id is not valid
 * \return #AJI_NOT_LOCKED Failure. TAP \p open_id is not locked by this client
 * \return #AJI_BAD_HARDWARE Failure. The hardware for the chain is had failed
 * \return #AJI_UNIMPLEMENTED Failure. Not implemented by this version of
 *            JTAG server
 * 
 * \note The execution of this function will be deferred if 
 *        #AJI_PACK_AUTO or #AJI_PACK_MANUAL have been specified.
 */
AJI_ERROR AJI_API aji_test_logic_reset        (AJI_OPEN_ID          open_id);

/**
 * Pauses the clock on the chain containing this TAP controller for 
 * at least the number of microseconds specified.  
 * Since the JTAG server is allowed to insert delays 
 * at any time the delay may be longer than requested.
 * 
 * \pre \p open_id must be locked with #aji_lock
 * 
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_OPEN_ID Failure. \p open_id is not valid
 * \return #AJI_NOT_LOCKED Failure. TAP \p open_id is not locked by this client
 * \return #AJI_BAD_HARDWARE Failure. The hardware for the chain is had failed

 */
AJI_ERROR AJI_API aji_delay                   (AJI_OPEN_ID          open_id,
                                               DWORD                timeout_microseconds);

/**
 * Specifies a callback function to be called when the JTAG client 
 *  or server has output to be passed to the user.
 *
 * \param output_fn The callback function. 
 *            \p handle is a pointer to anything, most 
 *            likely a data structure, that was passed
 *            in by the caller when registering the call
 *            \p level One of #OUTPUT_CALLBACK_LEVEL
 * \param handle Pointer to anything that the caller wants 
 *            \p output_fn to hand over to caller when 
 *            it is called. Can be \p NULL
 * 
 * \note The output function might be called by a 
 * different thread to the one used by the client 
 * when calling AJI functions.
 */
void AJI_API aji_register_output_callback     (void              ( * output_fn)(void * handle, DWORD level, const char * line),
                                               void               * handle);

/**
 * Specifies a callback function to be called when the JTAG client
 * or server has progress to be passed to the user.
 *   
 * \param progress_fn The callback function.
 *             \p handle is pointer to anything, most
 *                   likely a data structure, that was passed
 *                   in by the caller when registering the call
 *             \p bits is the number of bits of user data which has been
 *                   written to the hardware since the last call to #aji_lock.
 *                   User data is defined as data which has been passed
 *                   to one of the #aji_access_dr functions.
 *            
 * \param handle Pointer to anything that the caller wants
 *            \p output_fn to hand over to caller when
 *            it is called. Can be \p NULL
 *
 * \note The progress function might be called by a
 * different thread to the one used by the client
 * when calling AJI functions.
 */
void AJI_API aji_register_progress_callback   (AJI_OPEN_ID          open_id,
                                               void              ( * progress_fn)(void * handle, DWORD bits),
                                               void               * handle);

/**
 * Reads the list of SLD Nodes for the SLD Hub represetned by the
 * TAP at position \p tap_position on \p scan chain \p chain_id.
 * 
 * This is the non-hierarchical version
 * 
 * \pre \p chain_id is locked by #aji_lock_chain
 * 
 * \param chain_id	#AJI_CHAIN_ID for the scan chain (hardware) containing the TAP
 *                  of interest. This is returned by #aji_get_hardware.
 * \param tap_position	The position of the TAP controller containing the
 *                  SLD Hub to be read in the scan chain \p chain_id. 
 *                  This can be determined from #aji_read_device_chain.
 * \param idcodes	A pointer to a buffer to be filled in with a list of the 
 *                  IDCODEs for the SLD nodes present within this device
 *                  On Calling, must be capable of holding at least 
 *                  \p idcode_n idcodes
 *                  On return, ifthe returned \p idcode_n is not larger
 *                  than its input value, \p idcodes will be filled 
 *                  with the appropriate IDCODEs for the  \p idcode_n 
 *                  elements. See Virtual JTAG/SLD documentation for
 *                  IDCODE format
 * \param idcode_n On calling, the size of the \p idcodes buffer.  
 *                  On return , the number of SLD Nodes found within this device.
 *                  Note that this value can be bigger than the size of \p idcodes.
 *                  Return 0 if there is no SLD HUB
 *
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_CHAIN_ID Failure. \p chain_id is invalid
 * \return #AJI_NOT_LOCKED. Failure. \p chain_id is not locked.
 * \return #AJI_CHAIN_NOT_CONFIGURED Failure. The chain has not be configured
 * \return #AJI_BAD_TAP_POSITION Failure. The TAP controller does not exists
 * \return #AJI_DEVICE_NOT_CONFIGURED Failure. The device at \p tap_position 
 *                 has not been defined
 * \return #AJI_FAILURE Failure. There is a SLDHUB, but it is not compatible with this
 *                 version of AJI API.
 * \return #AJI_BAD_HARDWARE Failure. The hardware for \p chain_id has failed
 * \return #AJI_CHAIN_IN_USE Failure. The TAP is already locked and is used by
 *                 by another AJI client
 * \return #AJI_TOO_MANY_DEVICES Failure. There are more SLD nodes, than the
 *                 space allocated on \p idcodes. Read \p idcode_n for
 *                 the number of space required.
 * \return #AJI_TOO_MANY_HIERARCHIES There are nodes in the hierarchy more than
 *                 #AJI_MAX_HIERARCHICAL_HUB_DEPTH deep.
 */
AJI_ERROR AJI_API aji_get_nodes               (AJI_CHAIN_ID         chain_id,
                                               DWORD                tap_position,
                                               DWORD              * idcodes,
                                               DWORD              * idcode_n);

/**
 * Reads the list of SLD Nodes for the SLD Hub represetned by the
 * TAP at position \p tap_position on \p scan chain \p chain_id.
 *
 * This is the non-hierarchical version
 *
 * \pre \p chain_id is locked by #aji_lock_chain
 *
 * \param chain_id	#AJI_CHAIN_ID for the scan chain (hardware) containing the TAP
 *                  of interest. This is returned by #aji_get_hardware.
 * \param tap_position	The position of the TAP controller containing the
 *                  SLD Hub to be read in the scan chain \p chain_id.
 *                  This can be determined from #aji_read_device_chain.
 * \param idcodes	A pointer to a buffer to be filled in with a list of the
 *                  IDCODEs for the SLD nodes present within this device
 *                  On Calling, must be capable of holding at least
 *                  \p idcode_n idcodes
 *                  On return, if the returned \p idcode_n is not larger
 *                  than its input value, \p idcodes will be filled
 *                  with the appropriate IDCODEs for the  \p idcode_n
 *                  elements. See Virtual JTAG/SLD documentation for
 *                  IDCODE format
 * \param idcode_n On calling, the size of the \p idcodes buffer.
 *                  On return , the number of SLD Nodes found within this device.
 *                  Note that this value can be bigger than the size of \p idcodes.
 *                  Return 0 if there is no SLD HUB
 * \param hub_info On return, The IDCODE for the SLD Hub at
 *                    \p chain_id[\p tap_position]
 *
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_CHAIN_ID Failure. \p chain_id is invalid
 * \return #AJI_NOT_LOCKED. Failure. \p chain_id is not locked.
 * \return #AJI_CHAIN_NOT_CONFIGURED Failure. The chain has not be configured
 * \return #AJI_BAD_TAP_POSITION Failure. The TAP controller does not exists
 * \return #AJI_DEVICE_NOT_CONFIGURED Failure. The device at \p tap_position
 *                 has not been defined
 * \return #AJI_FAILURE Failure. There is a SLDHUB, but it is not compatible with this
 *                 version of AJI API.
 * \return #AJI_BAD_HARDWARE Failure. The hardware for \p chain_id has failed
 * \return #AJI_CHAIN_IN_USE Failure. The TAP is already locked and is used by
 *                 by another AJI client
 * \return #AJI_TOO_MANY_DEVICES Failure. There are more SLD nodes, than the
 *                 space allocated on \p idcodes. Read \p idcode_n for
 *                 the number of space required.
 * \return #AJI_TOO_MANY_HIERARCHIES There are nodes in the hierarchy more than
 *                 #AJI_MAX_HIERARCHICAL_HUB_DEPTH deep.
 */
AJI_ERROR AJI_API aji_get_nodes               (AJI_CHAIN_ID         chain_id,
                                               DWORD                tap_position,
                                               DWORD              * idcodes,
                                               DWORD              * idcode_n,
                                               DWORD              * hub_info);

/**
 * Reads the list of SLD Nodes for the SLD Hub represetned by the
 * TAP at position \p tap_position on \p scan chain \p chain_id.
 *
 * \pre \p chain_id is locked by #aji_lock_chain
 *
 * \param chain_id	#AJI_CHAIN_ID for the scan chain (hardware) containing the TAP
 *              of interest. This is returned by #aji_get_hardware.
 * \param tap_position	The position of the TAP controller containing the
 *              SLD Hub to be read in the scan chain \p chain_id.
 *                  This can be determined from #aji_read_device_chain.
 * \param hier_ids A pointer to an array of #AJI_HIER_ID to hold output
 *                  On calling, must be capable of holding at least
 *                  \p hier_id_n elements.
 *                  On return, if the returned \p hier_id_n is not larger
 *                  than its input value, \p hier_ids will be filled
 *                  with the appropriate IDCODEs for the \p hier_id_n
 *                  elements
 * \param hier_id_n	On calling, the size of the \p hier_ids buffer.
 *                 On return, the number of nodes found within this device.
 *                 Note that the return value can be larger than the value
 *                 when calling it.
 *                 Return 0 if there is no SLD HUB
 * \param hub_infos Hub information for each level of hierarchies
 *                 where applicable. Optional, pass in \p NULL if not needed.
 *
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_CHAIN_ID Failure. \p chain_id is invalid
 * \return #AJI_NOT_LOCKED. Failure. \p chain_id is not locked.
 * \return #AJI_CHAIN_NOT_CONFIGURED Failure. The chain has not be configured
 * \return #AJI_BAD_TAP_POSITION Failure. The TAP controller does not exists
 * \return #AJI_DEVICE_NOT_CONFIGURED Failure. The device at \p tap_position
 *                 has not been defined
 * \return #AJI_FAILURE Failure. There is a SLDHUB, but it is not compatible with this
 *                 version of AJI API.
 * \return #AJI_BAD_HARDWARE Failure. The hardware for \p chain_id has failed
 * \return #AJI_CHAIN_IN_USE Failure. The TAP is already locked and is used by
 *                 by another AJI client
 * \return #AJI_TOO_MANY_DEVICES Failure. There are more SLD nodes, than the
 *                 space allocated on \p hier_ids. Read \p hier_id_n for
 *                 the number of space required.
 * \return #AJI_TOO_MANY_HIERARCHIES There are nodes in the hierarchy more than
 *                 #AJI_MAX_HIERARCHICAL_HUB_DEPTH deep.
 */
AJI_ERROR AJI_API aji_get_nodes               (AJI_CHAIN_ID         chain_id,
                                               DWORD                tap_position,
                                               AJI_HIER_ID        * hier_ids,
                                               DWORD              * hier_id_n,
                                               AJI_HUB_INFO       * hub_infos);

 
/**
 * Open a connection to a node on the chain, including hierarchical nodes.  
 * Once opened the node behaves in almost the same way 
 * as a device opened using #aji_open_device
 * 
 * \pre \p chain_id must be locked by #aji_lock_chain
 * 
 * \param chain_id The chain_id for the hardware. This is returned by #aji_get_hardware.
 * \param tap_position	The position of the TAP controller in the chain. 
 *                 This can be determined from #aji_read_device_chain.
 * \param idcode The SLD identity code which is desired.  
 *                 This should be one of the values returned from a previous call to #aji_get_nodes.
 * \param node_id The \p node_id returned by the function. This argument is passed to other 
 *                 functions to use the scan chains that are claimed.
 *                 Resources are handled in the same way as for #aji_open_device, 
 *                 except that any \c OVERLAY resources claimed have their claims modified 
 *                 to reflect the value of the node select bits in the overlay register.
 * \param claims An array of size \p claim_n containing the binary codes for the resources to claim.
 *                 At least one #AJI_CLAIM_OVERLAY or #AJI_CLAIM_OVERLAY_SHARED  or  #AJI_CLAIM_OVERLAY_WEAK 
 *                 and one of  #IR_SHARED_OVERLAID/#IR_OVERLAID register. They are
 *                  \c USER1 and \c USER0 of SLD architecture.They represents the VIR register for the SLD node.
 * \param claim_n The number of entries in the claims array.
 * \param application_name A null-terminated string giving the identity of the 
 *                 application claiming the chains.
 *
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_CHAIN_ID Failure. \p chain_id is not valid.
 * \return #AJI_INVALID_PARAMETER Failure. One or more parameters are invalid.
 * \return #AJI_NOT_LOCKED Failure. \p chain_id is not locked by this client.
 * \return #AJI_CHAIN_NOT_CONFIGURED Failure. \p chain_id  has not been configured.
 * \return #AJI_BAD_TAP_POSITION Failure. TAP controller does not exist at \p tap_position.
 * \return #AJI_DEVICE_NOT_CONFIGURED Failure. Tap at \p tap_position has not been defined.
 * \return #AJI_CHAINS_CLAIMED Failure. One or more or requested resources are already claimed.
 * \return #AJI_FAILURE Failure. There is no SLD hub on the device specified, 
 *                                 or that the device requested is not present on that hub.
 * \return #AJI_NO_MATCHING_NODES Failure. No matching node found
 * \return #AJI_HIERARCHICAL_HUB_NOT_SUPPORTED Failure. The desired node is down in the hierarchy and 
 *                                 the server version does not support the access
 * \return #AJI_IR_LENGTH_ERROR Failure. The desired node is too deep down in the hierarchy where 
 *                                 the number of select bits + IR bits is more than 64
 *
 * \note Search did not discover any actual top level use of this function in
 *     <tt> libaji_client</tt>. As such this function is provided as-is and may not work.
 */
AJI_ERROR AJI_API aji_open_node               (AJI_CHAIN_ID         chain_id,
                                               DWORD                tap_position,
                                               DWORD                idcode,
                                               AJI_OPEN_ID        * node_id,
                                               const AJI_CLAIM    * claims,
                                               DWORD                claim_n,
                                               const char         * application_name);

/**
 * Open a connection to a node on the chain, including hierarchical nodes.
 * Once opened the node behaves in almost the same way
 * as a device opened using #aji_open_device
 *
 * \pre \p chain_id must be locked by #aji_lock_chain
 *
 * \param chain_id The chain_id for the hardware. This is returned by #aji_get_hardware.
 * \param tap_position	The position of the TAP controller in the chain.
 *                 This can be determined from #aji_read_device_chain.
 * \param node_position	The position of the SLD node on the TAP controller
 * \param idcode The SLD identity code which is desired.
 *                 This should be one of the values returned from a previous call to #aji_get_nodes.
 * \param node_id The \p node_id returned by the function. This argument is passed to other
 *                 functions to use the scan chains that are claimed.
 *                 Resources are handled in the same way as for #aji_open_device,
 *                 except that any \c OVERLAY resources claimed have their claims modified
 *                 to reflect the value of the node select bits in the overlay register.
 * \param claims An array of size \p claim_n containing the binary codes for the resources to claim.
 *                 At least one #AJI_CLAIM_OVERLAY or #AJI_CLAIM_OVERLAY_SHARED  or  #AJI_CLAIM_OVERLAY_WEAK
 *                 and one of  #IR_SHARED_OVERLAID/#IR_OVERLAID register. They are
 *                  \c USER1 and \c USER0 of SLD architecture.They represents the VIR register for the SLD node.
 * \param claim_n The number of entries in the claims array.
 * \param application_name A null-terminated string giving the identity of the
 *                 application claiming the chains.
 *
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_CHAIN_ID Failure. \p chain_id is not valid.
 * \return #AJI_INVALID_PARAMETER Failure. One or more parameters are invalid.
 * \return #AJI_NOT_LOCKED Failure. \p chain_id is not locked by this client.
 * \return #AJI_CHAIN_NOT_CONFIGURED Failure. \p chain_id  has not been configured.
 * \return #AJI_BAD_TAP_POSITION Failure. TAP controller does not exist at \p tap_position.
 * \return #AJI_DEVICE_NOT_CONFIGURED Failure. Tap at \p tap_position has not been defined.
 * \return #AJI_CHAINS_CLAIMED Failure. One or more or requested resources are already claimed.
 * \return #AJI_FAILURE Failure. There is no SLD hub on the device specified,
 *                                 or that the device requested is not present on that hub.
 * \return #AJI_NO_MATCHING_NODES Failure. No matching node found
 * \return #AJI_HIERARCHICAL_HUB_NOT_SUPPORTED Failure. The desired node is down in the hierarchy and
 *                                 the server version does not support the access
 * \return #AJI_IR_LENGTH_ERROR Failure. The desired node is too deep down in the hierarchy where
 *                                 the number of select bits + IR bits is more than 64
 *
 * \note Search did not discover any actual top level use of this function in
 *     <tt> libaji_client</tt>. As such this function is provided as-is and may not work.
 */
AJI_ERROR AJI_API aji_open_node               (AJI_CHAIN_ID         chain_id,
                                               DWORD                tap_position,
                                               DWORD                node_position,
                                               DWORD                idcode,
                                               AJI_OPEN_ID        * node_id,
                                               const AJI_CLAIM    * claims,
                                               DWORD                claim_n,
                                               const char         * application_name);

/**
 * Open a connection to a node on the chain, including hierarchical nodes.
 * Once opened the node behaves in almost the same way
 * as a device opened using #aji_open_device
 *
 * \pre \p chain_id must be locked by #aji_lock_chain
 *
 * \param chain_id The chain_id for the hardware. This is returned by #aji_get_hardware.
 * \param tap_position	The position of the TAP controller in the chain.
 *                 This can be determined from #aji_read_device_chain.
 * \param hier_id The hierarchical SLD identity of the node. This should be one of the
 *                 struct returned from a previous call to hierarchical
 *                 #aji_get_nodes(AJI_CHAIN_ID,DWORD,AJI_HIER_ID*,DWORD*,AJI_HUB_INFO*).
 * \param node_id The \p node_id returned by the function. This argument is passed to other
 *                 functions to use the scan chains that are claimed.
 *                 Resources are handled in the same way as for #aji_open_device,
 *                 except that any \c OVERLAY resources claimed have their claims modified
 *                 to reflect the value of the node select bits in the overlay register.
 * \param claims An array of size \p claim_n containing the binary codes for the resources to claim.
 *                 At least one #AJI_CLAIM_OVERLAY or #AJI_CLAIM_OVERLAY_SHARED  or  #AJI_CLAIM_OVERLAY_WEAK
 *                 and one of  #IR_SHARED_OVERLAID/#IR_OVERLAID register. They are
 *                  \c USER1 and \c USER0 of SLD architecture.They represents the VIR register for the SLD node.
 * \param claim_n The number of entries in the claims array.
 * \param application_name A null-terminated string giving the identity of the
 *                 application claiming the chains.
 *
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_CHAIN_ID Failure. \p chain_id is not valid.
 * \return #AJI_INVALID_PARAMETER Failure. One or more parameters are invalid.
 * \return #AJI_NOT_LOCKED Failure. \p chain_id is not locked by this client.
 * \return #AJI_CHAIN_NOT_CONFIGURED Failure. \p chain_id  has not been configured.
 * \return #AJI_BAD_TAP_POSITION Failure. TAP controller does not exist at \p tap_position.
 * \return #AJI_DEVICE_NOT_CONFIGURED Failure. Tap at \p tap_position has not been defined.
 * \return #AJI_CHAINS_CLAIMED Failure. One or more or requested resources are already claimed.
 * \return #AJI_FAILURE Failure. There is no SLD hub on the device specified,
 *                                 or that the device requested is not present on that hub.
 * \return #AJI_NO_MATCHING_NODES Failure. No matching node found
 * \return #AJI_HIERARCHICAL_HUB_NOT_SUPPORTED Failure. The desired node is down in the hierarchy and
 *                                 the server version does not support the access
 * \return #AJI_IR_LENGTH_ERROR Failure. The desired node is too deep down in the hierarchy where
 *                                 the number of select bits + IR bits is more than 64
 *
 * \note Search did not discover any actual top level use of this function in
 *     <tt> libaji_client</tt>. As such this function is provided as-is and may not work.
 */
AJI_ERROR AJI_API aji_open_node               (AJI_CHAIN_ID         chain_id,
                                               DWORD                tap_position,
                                               const AJI_HIER_ID  * hier_id,
                                               AJI_OPEN_ID        * node_id,
                                               const AJI_CLAIM2   * claims,
                                               DWORD                claim_n,
                                               const char         * application_name);

/**
 * This function writes the \c OVERLAY instruction (\p USER1 for virtual JTAG/SLD) 
 * to the IR register of the 
 * TAP controller specified by \p open_id, it then writes an \p overlay value 
 * to the DR register and finally writes the \c OVERLAID value (\p USER0) to the IR register.  
 * If requested, it returns the value captured during the DR scan. 
 * This function can access hierarchical node or hub.
 * 
 * \pre The TAP controller must be in \p Run-Test-Idle, \p Update-IR, 
 *          \p Update-DR or \p Select-DR-Scan before this function is called 
 * \pre \p node_id must be locked with #aji_lock
 * 
 * \param node_id The open_id of the TAP controller node. 
 *          This is returned by #aji_open_node, #aji_open_hub or #aji_find_node
 * \param overlay The binary code of the overlay value, i.e. the virtiual 
 *         IR register  for the SLD node
 * \param capture_overlay A pointer to the location in which to store 
 *          the status value captured from the overlay register.
 *          Can be \c NULL.
 * 
 * \post The TAP controller will be left in state \p UPDATE-IR
 *
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_OPEN_ID Failure. \p open_id is not valid
 *              One common cause is the #AJI_OPEN of the TAP is used instead of
 *              the SLD Node
 * \return #AJI_BAD_TAP_STATE Failure. The TAP controller is not in
 *              \c Run-Test-Idle, \c Update-IR, \c Update-DR 
 *              or \c Select-DR-Scan before this function is called 
 * \return #AJI_NOT_LOCKED Failure. TAP \p open_id is not locked by this client
 * \return #AJI_INVALID_PARAMETER Failure. The \p overlay virtual register is not valid
 * \return #AJI_INSTRUCTION_CLAIMED Failure. The \p overlay virtual register has been
 *              claimed by another client
 * \return #AJI_BAD_HARDWARE Failure. The hardware for \p node_id has failed
 *
 */
AJI_ERROR AJI_API aji_access_overlay          (AJI_OPEN_ID          node_id,
                                               DWORD                overlay,
                                               DWORD              * captured_overlay);

/**
 * Used to open a connection to a hierarchical hub that a node is associated to, 
 * given the node's \p hier_id and \p hub_level and the TAP (defined with \p chain_id
 * and \p tap_position) the node is on.  Once opened the hub behaves in almost 
 & the same way as a device opened using #aji_open_device or a node opened using #aji_open_node. 
 * 
 * \pre \p chain_id must be locked with #aji_lock_chain
 * 
 * \param chain_id The #AJI_CHAIN_ID for the hardware
 * \param tap_position The position of the TAP controller in \p chain_id
 * \param hub_level The hirarchical level of the SLD node \p hier_id.
 *                   It is one of the value returned by #aji_get_nodes
 * \param hier_id The hierarchical SLD identity of the hub returned by #aji_get_nodes
 * \param hub_id The id for the parent hub as defined by \p hier_id and \p hub_level. R
 *                   Remember: a hier_id can be several level down from the top and so
 *                   \p hier_level is needed to determine which parent you want.
 *                   \p hub_id never takes the value 0.
 * \param claims An array of size \p claim_n containing the binary code for 
 *                   the resrouces to claim. Must include a claim of the
 *                   #IR_SHARED_OVERLAY register and one of 
 *                   #IR_SHARED_OVERLAID/#IR_OVERLAID register. They are
 *                   \p USER1 and \p USER0 of SLD architecture.
 * \param claim_n Size of claim_n
 * \param application_name A null-terminated string giving the identity of the application
 *                   making the claim to \p hub_id
 * 
 * \return #AJI_NO_ERROR Success
 * \return #AJI_INVALID_OPEN_ID Failure. \p open_id is not valid
 *              One common cause is the #AJI_OPEN of the TAP is used instead of
 *              the SLD Node
 * \return #AJI_INVALID_PARAMETER Failure. One or more of the parameter is invalid
 * \return #AJI_NOT_LOCKED Failure. TAP \p open_id is not locked by this client
 * \return #AJI_BAD_TAP_POSITION Failure. The TAP controller does not exists
 * \return #AJI_DEVICE_NOT_CONFIGURED Failure.  The device (TAP) at \p tap_position is not defined
 * \return #AJI_CHAINS_CLAIMED Failure. Indicate that one or more requested resources are 
 *                      already claimed
 * \return #AJI_FAILURE Failure. There is no SLD hub on the device specified, or that the
 *                      the device requested is not present on the hub
 * \return #AJI_NO_MATCHING_NODES Failure. There is no matching hub found.
 * \return #AJI_HIERARCHICAL_HUB_NOT_SUPPORTED The desired hub is down in the hierarchy and 
 *                      the server version does not support access
 * \return #AJI_INSTRUCTION_CLAIMED Failure. The \p overlay virtual register has been
 *              claimed by another client
 * \return #AJI_IR_LENGTH_ERROR Failure. The desired hub is too deep down the heirarchy where
 *              the number of selected bits + IR bits is more than 64.
 */
AJI_ERROR AJI_API aji_open_hub                (AJI_CHAIN_ID         chain_id,
                                               DWORD                tap_position,
                                               DWORD                hub_level,
                                               const AJI_HIER_ID  * hier_id,
                                               AJI_OPEN_ID        * hub_id,
                                               const AJI_CLAIM2   * claims,
                                               DWORD                claim_n,
                                               const char         * application_name);

#endif // INC_AJI_H
