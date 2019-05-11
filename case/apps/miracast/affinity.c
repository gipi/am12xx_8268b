/**
 * @file affinity.c
 *
 * Implementation of WiDi affinity check code.
 *
 * INTEL CONFIDENTIAL
 * Copyright 2010-2011 Intel Corporation All Rights Reserved.
 *
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation or its
 * suppliers or licensors.  Title to the Material remains with Intel
 * Corporation or its suppliers and licensors.  The Material contains trade
 * secrets and proprietary and confidential information of Intel or its
 * suppliers and licensors.  The Material is protected by worldwide copyright
 * and trade secret laws and treaty provisions. No part of the Material may
 * be used, copied, reproduced, modified, published, uploaded, posted,
 * transmitted, distributed, or disclosed in any way without Intel's prior
 * express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or
 * delivery of the Materials,  either expressly, by implication, inducement,
 * estoppel or otherwise.  Any license under such intellectual property
 * rights must be express and approved by Intel in writing.
 */

// {{{ Includes
#include <stdio.h>    // snprintf
#include <stdint.h>   // for uint8_t
#include <fcntl.h>    // for O_RDNLY
#include <syslog.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>   // strtol
#include <stdarg.h>   // strtol

#include "affinity.h"
#include "sha2.h"
// }}}

// {{{ Debug helpers
#define DEBUG(...)   logger( __func__, __LINE__, LOG_DEBUG, __VA_ARGS__ )  ///< Messages used for development; they aren't shown for production builds
#define LOGNAME       __FILE__ ": "                             ///< Label prefixed to all DEBUG, NOTICE, ERROR, and CRITICAL calls
#define NOTICE(...)   syslog( LOG_NOTICE, LOGNAME __VA_ARGS__ ) ///< Informational messages that do not indicate an abnormal situation
#define ERROR(...)    syslog( LOG_ERR, LOGNAME __VA_ARGS__ )    ///< Messages that indicate an abnormal situation, but program operation can continue
#define CRITICAL(...) logger( __func__, __LINE__, LOG_CRIT, __VA_ARGS__ )  ///< Messages that indicate a severe situation where normal program operation cannot continue

/**
 * Internal helper function to print debug messages. Function name, file name,
 * and line number are included automatically. Outputs to syslog.
 *
 * @returns None
 */
static void logger( const char* func, int line, int priority, char* format, ... )
    __attribute__ ((format (printf, 4, 5)));
static void logger( const char* func, int line, int priority, char* format, ... )
{
    char new_format[255];
    assert( strlen(format) < sizeof(new_format) );
    snprintf( new_format, sizeof(new_format), "%s:%d: %s: %s", __FILE__, line, func, format );

    va_list args;
    va_start( args, format );
    vsyslog( priority, new_format, args );
    va_end( args );
}
// }}}

/**
 * Fills specified buffer with random data obtained from the operating system's
 * hardware random number generator.
 *
 * @returns 0 on success
 * @returns 1 on failure
 */
static int rtsp_affinity_get_random_number
    ( uint8_t *buf ///< buffer to store the random data in
    , int bufsize  ///< length of the buffer; this many bytes of random data will be obtained
    )
{
    assert( buf );

    int devrandom = -1;

    devrandom = open( "/dev/urandom", O_RDONLY );
    if ( devrandom == -1 ) {
        ERROR( "error opening /dev/urandom -- cannot continue" );
        return 1;
    }

    int rc = 0;
    int bytes_read = read( devrandom, buf, bufsize );
    if ( bytes_read != bufsize ) {
        ERROR( "expecting %d bytes from /dev/urandom but only read %d", bufsize, bytes_read );
        rc = 1;
    }

    close( devrandom );

    return rc;
}

/**
 * Reads the shared secret from disk, copies it into a static buffer, and
 * returns a pointer to the buffer. Not thread safe; intended to be used by a
 * single caller only.
 *
 * @returns pointer to null-terminated string containing ASCII representation
 *          of affinity shared secret (in hex)
 */
char* rtsp_affinity_get_shared_secret( void )
{
    static char affinity_shared_secret[33];
    static int file_loaded = 0;

    const char* filepath = "affinity_shared_secret.txt";
    FILE *fp = NULL;
    int bytesread = 0;

    if ( file_loaded ) {
        return affinity_shared_secret;
    }

    fp = fopen( filepath, "rt" );
    if ( NULL == fp ) {
        ERROR( "Unable to open file %s", filepath );
        return NULL;
    }

    bytesread = fread( affinity_shared_secret, sizeof(char), 32, fp );

    affinity_shared_secret[ bytesread ] = 0;
    fclose( fp );
    file_loaded = 1;

    return affinity_shared_secret;
}

/**
 * Helper function used to calculate the affinity hashes.
 *
 * @returns 0 on success
 * @returns 1 on failure
 */
static int rtsp_affinity_calculate_hash
    ( rtsp_affinity_t* a       ///< [in] pointer to the affinity state data structure
    , int hashnum              ///< [in] The hash number to compute, 1 or 2
    )
{
    assert( a );

    int rc = 1;
    SHA256_CTX ctx256;
    SHA256_Init( &ctx256 );

    if ( hashnum == 1 ) {
        SHA256_Update( &ctx256, a->Tn, sizeof(a->Tn) );
        SHA256_Update( &ctx256, a->An, sizeof(a->An) );
        SHA256_Update( &ctx256, a->SharedSecret, sizeof(a->SharedSecret) );
        SHA256_Final( a->H1, &ctx256 );
        rc = 0;
    } else if ( hashnum == 2 ) {
        SHA256_Update( &ctx256, a->An, sizeof(a->An) );
        SHA256_Update( &ctx256, a->Tn, sizeof(a->Tn) );
        SHA256_Update( &ctx256, a->SharedSecret, sizeof(a->SharedSecret) );
        SHA256_Final( a->H2, &ctx256 );
        rc = 0;
    } else {
        CRITICAL( "invalid hashnum, must be 1 or 2" );
        assert( 0 );
        rc = 1;
    }

    return rc;
}

/**
 * Helper function to convert a hex number in ASCII representation to binary.
 *
 * @returns 0 on success
 * @returns 1 on failure
 */
static int rtsp_affinity_string_to_bytes
    ( const char* string ///< [in] pointer to string containing bytes to convert
    , uint8_t* buf       ///< [out] pointer to buffer to store the converted output
    , const int buflen   ///< [in] length of buf in bytes
    )
{
    assert( string );
    assert( buf );

    int i;
    char* start;
    char hexbyte[3];

    for ( i=0, start=(char*)string; i<buflen; i++ ) {
        snprintf( hexbyte, 3, "%s", start + i*2 );
        buf[i] = (uint8_t)strtol( hexbyte, NULL, 16 );
    }

    return i == buflen ? 0 : 1;
}

/**
 * Helper function to convert a hex number from binary to its ASCII representation.
 *
 * @returns 0 on success
 * @returns 1 on failure
 */
static int rtsp_affinity_bytes_to_string(
        const uint8_t* bytes, ///< [in] pointer to binary data to encode
        int byteslen,         ///< [in] length of binary data to encode
        char* buf,            ///< [out] pointer to buffer to hold ASCII-encoded representation of binary data
        int buflen )          ///< [in] Length of output buffer
{
    assert( bytes );
    assert( buf );

    int i;
    char* p = buf;
    int maxbytes = (buflen-1)/2; // -1 for NULL and then 2 ASCII chars per bye

    for ( i=0; i<maxbytes && i<byteslen; i++ ) {
        snprintf( p, 3, "%02x", bytes[i] );
        p += 2;
    }
    // Null termination already taken care of by sprintf

    return i == byteslen ? 0 : 1;
}

/**
 * Helper function to extract the cryptographic values from the RTSP parameters
 * and put them into a form that the affinity code can use.
 *
 * @returns Same as rtsp_affinity_string_to_bytes()
 */
static int rtsp_affinity_get_parameter
    ( PARAMETER_STRUCT* param ///< [in] RTSP paramater to parse
    , uint8_t* buf            ///< [out] buffer to store the binary output in
    , int buflen              ///< [in] length of the output buffer
    )
{
    assert( param );
    assert( buf );

    if ( ! param->szValue ) {
        ERROR( "%s: expecting a parameter but didn't get one", __func__ );
        return 1;
    }

    return rtsp_affinity_string_to_bytes( param->szValue, buf, buflen );
}

/**
 * Performs the first step of the affinity handshake. Reads Tn, computes An,
 * computes H1, and sends An and H1.
 *
 * @returns 0 on success
 * @returns 1 on failure
 */
static int rtsp_affinity_handshake_init
    ( rtsp_affinity_t* a       ///< [in] pointer to the affinity state data structure
    , PARAMETER_STRUCT* p_req  ///< [in] pointer to RTSP input parameter containing Tn
    , PARAMETER_STRUCT* p_resp ///< [out] pointer to RTSP output parameter into which An and H1 will be placed
    )
{
    assert( a );
    assert( p_req );
    assert( p_resp );

    int rc;
    char An_str[65]; // 32 bytes binary is 64 bytes ASCII hex plus null
    char H1_str[65];

    a->affinity_check_passed = 0;

    // Read Tn
    DEBUG( "Tn:%s", p_req->szValue );

    rc = rtsp_affinity_get_parameter( p_req, a->Tn, 32 );
    if ( rc != 0 ) {
        p_resp->nId = rds_fail_intel_hdshk;
        snprintf( p_resp->szValue, RDS_STATIC_LEN,
                "Invalid or missing Tn parameter" );
        return 1;
    }

    // Compute H1
    rtsp_affinity_get_random_number( a->An, sizeof(a->An) );
    rtsp_affinity_calculate_hash( a, 1 );

    // Send An, H1
    rtsp_affinity_bytes_to_string( a->An, 32, An_str, sizeof(An_str) );
    rtsp_affinity_bytes_to_string( a->H1, 32, H1_str, sizeof(H1_str) );

    assert( strlen(An_str) == 64 );
    assert( strlen(H1_str) == 64 );

    snprintf( p_resp->szValue, RDS_STATIC_LEN, "%s %s", An_str, H1_str );

    DEBUG( "An:%s H1:%s", An_str, H1_str );

    p_resp->nId = rds_resp_intel_hdshk;

    return 0;
}

/**
 * Performs the second step of the affinity handshake. Reads H2 and checks H2.
 *
 * @returns 0 if the affinity handshake completed successfully
 * @returns 1 if the affinity handhsake did not pass
 */
static int rtsp_affinity_handshake_continue
    ( rtsp_affinity_t* a       ///< [in] pointer to the affinity state data structure
    , PARAMETER_STRUCT* p_req  ///< [in] pointer to RTSP input parameter containing H2
    , PARAMETER_STRUCT* p_resp ///< [out] pointer to RTSP output parameter for failure case
    )
{
    int rc;
    uint8_t other_H2[32];

    // Read H2 (from RDS source)
    rc = rtsp_affinity_get_parameter( p_req, other_H2, sizeof(other_H2) );
    if ( rc != 0 ) {
        p_resp->nId = rds_fail_intel_hdshk;
        snprintf( p_resp->szValue, RDS_STATIC_LEN, "Invalid or missing H2 parameter" );
        return 1;
    }

    // Calculate local H2
    rtsp_affinity_calculate_hash( a, 2 );

    // Verify local and other H2
    rc = memcmp( other_H2, a->H2, 32 );
    if ( rc != 0 ) {
        p_resp->nId = rds_fail_intel_hdshk;
        snprintf( p_resp->szValue, RDS_STATIC_LEN, "Hash verification failed" );
        return 1;
    }

    a->affinity_check_passed = 1;
    DEBUG( "WiDi affinity check successful" );
    return 0;
}

/**
 * Performs the RDS affinity handshake.
 *
 * @returns On the first invocation, returns RSTP_NO_ERROR if the affinity shared secret is not available.
 * @returns On the second invocation, returns the parameter response for the RTSP source
 * @returns On the third invocation, returns RTSP_NO_ERROR if the affinity check completed successfully; otherwise, returns a parameter response indicating the reason for failure.
 *
 */
long rtsp_affinity_check
    ( rtsp_handle h       ///< handle to RTSP library
    , PARAMETER_STRUCT* p ///< pointer to RTSP parameter from RTSP source
    )
{
    static rtsp_affinity_t affinity_vars;
    char* shared_secret = NULL;
    PARAMETER_STRUCT p_resp;
    int rc;

    shared_secret = rtsp_affinity_get_shared_secret();
    if ( shared_secret == NULL ) {
        ERROR( "unable to get shared secret; affinity check unable to proceed" );
        affinity_vars.affinity_check_passed = 0;
        h->affinity_check_passed = 0;
        return RTSP_NO_ERROR;
    }

    rtsp_affinity_string_to_bytes( shared_secret,
            affinity_vars.SharedSecret,
            sizeof(affinity_vars.SharedSecret) );

    memset( &p_resp, 0, sizeof(p_resp) );
    p_resp.szValue = p_resp.szStaticBuffer;

    if ( p->nId == rds_init_intel_hdshk ) {
        rtsp_affinity_handshake_init( &affinity_vars, p, &p_resp );
        rtsplib_setparamstruct( RTSP_MODE_RDS, h->hRtspLib, &p_resp );
    } else if ( p->nId == rds_cfrm_intel_hdshk ) {
        rc = rtsp_affinity_handshake_continue( &affinity_vars, p, &p_resp );
        // If the handshake fails, there is a parameter to send back,
        // otherwise there is no parameter (just a success message,
        // which the RTSP library will handle on its own)
        if ( rc != 0 ) {
            // failure case
            rtsplib_setparamstruct( RTSP_MODE_RDS, h->hRtspLib, &p_resp );
        }
    }

    h->affinity_check_passed = affinity_vars.affinity_check_passed;

    return RTSP_NO_ERROR;
}

/**
 * Provides test coverage for the functions in affinity.c.
 *
 * @returns None
 */
void rtsp_affinity_test_vector( void )
{
    static rtsp_affinity_t affinity_vars;
    char buf[100];

    memset( &affinity_vars, 0, sizeof(affinity_vars) );

    // Load test vector values - An, Tn, SharedSecret
    rtsp_affinity_string_to_bytes(
        "65e3fe2611fda08d2005149719fdc82fd9cee17569a47d75d3069bfc9153d109",
        affinity_vars.An, 32 );
    rtsp_affinity_string_to_bytes(
        "5d03eebed6753b69d3f7e5109da0457218e701443b353bf5d7e24ea2789edd23",
        affinity_vars.Tn, 32 );
    rtsp_affinity_string_to_bytes( "7EEDE2E8226911E0BD3EDE9BDFD72085",
        affinity_vars.SharedSecret, 16 );

    // Compute H1 & H2
    rtsp_affinity_calculate_hash( &affinity_vars, 1 );
    rtsp_affinity_calculate_hash( &affinity_vars, 2 );

    // Output all values
    rtsp_affinity_bytes_to_string( affinity_vars.SharedSecret, 16, buf, sizeof(buf) );
    printf("SharedSecret: %s\n", buf );

    rtsp_affinity_bytes_to_string( affinity_vars.An, 32, buf, sizeof(buf) );
    printf("An: %s\n", buf );

    rtsp_affinity_bytes_to_string( affinity_vars.Tn, 32, buf, sizeof(buf) );
    printf("Tn: %s\n", buf );

    rtsp_affinity_bytes_to_string( affinity_vars.H1, 32, buf, sizeof(buf) );
    printf("H1: %s\n", buf );

    rtsp_affinity_bytes_to_string( affinity_vars.H2, 32, buf, sizeof(buf) );
    printf("H2: %s\n", buf );
}
