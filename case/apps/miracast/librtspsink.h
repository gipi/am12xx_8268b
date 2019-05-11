/**
 * @file librtspsink.h
 *
 * Header file for librtspsink.c
 *
 * INTEL CONFIDENTIAL
 * Copyright 2010-2012 Intel Corporation All Rights Reserved.
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

#ifndef _SAMPLE_RTSP_SINK
#define _SAMPLE_RTSP_SINK

// {{{ Includes
#include "rtsp_abstraction.h"
// }}

#define MAX_EDID_LEN                512

#define RTP_PORT_DEFAULT            42030
#define RTSP_SOURCE_PORT_DEFAULT    554

// Connector types
typedef enum
    { RTSP_CONNECTOR_NONE
    , RTSP_CONNECTOR_VGA
    , RTSP_CONNECTOR_HDMI
    , RTSP_CONNECTOR_COMPOSITE
    , RTSP_CONNECTOR_DVI
    // As of 2012-05-01, selecting RTSP_CONNECTOR_WIFI_DISPLAY as the connector
    // type will cause the laptop to set the display type as VGA with the
    // graphics driver. This means that HDCP will not be available.
    , RTSP_CONNECTOR_WIFI_DISPLAY
    } rtsp_connector_t;

// All of these defines have an extra byte for the NULL
#define RDS_PRODUCT_ID_BUF_LEN    17

typedef struct rtsp_config* rtsp_config_p; // forward declaration to enable next prototype
//typedef long (*rtsp_fn_notification_callback)( int id, void* szValue, void* pContext );

/** @brief rtsp_param is a struct for holding RTSP parameters that have string
 * values.
 */
typedef struct rtsp_param* rtsp_param_p;
typedef struct rtsp_param {
    char* key;
    char* value;
    int deleted;
    rtsp_param_p next; // linked list, set to NULL to end
} rtsp_param_t;

typedef struct {
    int teardown;
    int pause;
    int unpause;
    // This data structure can be extended as needed to add additional request types
} rtsp_request_t;

/**
 * Used to manage the internal state of the RTSP library. Data structure
 * must be initialized by using rtsp_init(). Then, a pointer to
 * rtsp_config_t is used as a handle for all functions in the library.
 */
typedef struct rtsp_config {
    uint32_t    checksum;

    rtsp_fn_notification_callback  rtsp_notification_callback;
    uint16_t    rtp_port;
    int         rtsp_mode; // See RTSP_MODE_* defines
    void*       caller_context;

    // The caller can set these values at any time. The values will
    // be automatically and asynchronously provided to the RTSP source
    // whenever a GET PARAMETER request is received.
    uint8_t     edid[ MAX_EDID_LEN ];
    uint16_t    edidlen;
    uint8_t     display_connected;
    rtsp_connector_t connector_type;
    int         affinity_check_passed; // 1=passed, 0=not passed or not attempted
    uint16_t    hdcp2_port; // set to 0 to disable HDCP2 support
    char        manufacturer_logo_path[ FILENAME_MAX ];
    VERSION_STRUCT software_version; // use rtsp_version_string_to_struct to set this
    VERSION_STRUCT hardware_version;
    char        product_id[ RDS_PRODUCT_ID_BUF_LEN ];

    // for internal use only by the sample app
    char        source_ip_address[16];  // max is 128.128.128.128+NULL = 4*4 = 16
    uint16_t    source_port;

    uint64_t    s3d_support;
    uint32_t    cea_support;
    uint32_t    vesa_support;
    uint32_t    hh_support;

    uint32_t    lpcm_support;
    uint32_t    aac_support;
    uint32_t    ac3_support;

    rtsp_param_t* params;

    int         socket;
    int         socket_connected; ///< 0=not connected, 1=connected
    int         teardown_received;
    int         teardown_sent;
    int*        cancel_request;
    rtsp_request_t* requests;
    int         rtsp_session_running;

    void*       hRtspLib;
    int         library_is_shutdown; ///< 0=library has not been shut down; 1=library has been shutdown and data structure needs to be re-initialized
} rtsp_config_t, *rtsp_handle;

/*
 * Prototypes for functions that must be defined by the caller for the
 * sample app to work.
 */
//extern long rtsp_notification_callback( rtsp_config_t* rtsp_config, int id, void* szValue );

/*
 * Prototypes for functions that are intended to be called by other
 * programs.
 */

/**
 * Initializes RTSP library for use. Must be called before calling any other
 * functions in the RTSP libary. Returns a handle that must be passed as the
 * first argument to all other functions in the libary.
 *
 * @returns NULL if an error occured
 * @returns handle if library successfully initialized
 */
int rtsp_init
    ( rtsp_handle h                          ///< [out] rtsp library handle to initialize
    , const int rtsp_mode                    ///< [in]
    , const unsigned int rtp_port            ///< [in] RTP port to receive data plane; must be 1-65535
    , rtsp_fn_notification_callback callback ///< [in]
    , void* caller_context
    );

/**
 * Sets the EDID to 'newedid'. Extracts 3D support information from EDID and
 * configures RTSP libary to respond correctly to Wi-Fi Display queries for 3D
 * information.
 */
void rtsp_set_edid_and_connector
    ( rtsp_handle h                         ///< [in] configured rtsp library handle
    , const uint8_t* newedid                ///< [in] pointer to a binary buffer containing the new EDID
    , const int newedidlen                  ///< [in] length of the EDID buffer
    , const rtsp_connector_t connector_type ///< [in] VGA or HDMI
    );

void rtsp_set_hdcp2_port
    ( rtsp_handle h  ///< [in] configured rtsp library handle
    , const unsigned int port ///< [in] set HDCP2 port
    );

int rtsp_set_video_formats
    ( rtsp_handle h  ///< [in] configured rtsp library handle as returned by rtsp_init()
    , const uint32_t cea_support
    , const uint32_t vesa_support
    , const uint32_t hh_support
    , const uint64_t s3d_support
    , const int reset ///< [in] 0=do not reset existing values, 1=reset existing values to 0 first
    );


int rtsp_set_audio_codecs
    ( rtsp_handle h  ///< [in] configured rtsp library handle as returned by rtsp_init()
    , const uint32_t lpcm_support
    , const uint32_t aac_support
    , const uint32_t ac3_support
    , const int reset ///< [in] 0=do not reset existing values, 1=reset existing values to 0 first
    );

int rtsp_sink_run_session
    ( rtsp_handle h       ///< [in] configured rtsp library handle
    , const char* ip_address
    , const unsigned int port
    , int* cancel_request ///< [in]
    , rtsp_request_t* requests
    );

/**
 * Use this function to determine if an RTSP session is currenly active.
 *
 * @returns 1 if RTSP libary is configured and session is running with a source
 * @returns 0 if RTSP libary is not configured or session is not running
 */
int rtsp_sink_session_running
    ( rtsp_handle h  ///< [in] configured rtsp library handle as returned by rtsp_init()
    );

int rtsp_sink_teardown
    ( rtsp_handle h       ///< [in] configured rtsp library handle
    );

int rtsp_send_hotplug_to_source
    ( rtsp_handle h
    , const rtsp_connector_t connector_type
    );

int rtsp_shutdown( rtsp_handle h );

int  rtsp_version_string_to_struct( VERSION_STRUCT* buf, const char* string );

void rtsp_set_default_params( rtsp_handle h );
long rtsp_set_parameter_struct_szvalue( const int rtsp_mode, void* pVoid, const char* new_szValue );
long rtsp_set_parameter_struct_mime_encoded( rtsp_handle h, void* pVoid, uint8_t* buf, long buflen );
long rtsp_set_parameter_struct_logo( rtsp_handle h, PARAMETER_STRUCT* p );

uint8_t* rtsp_get_sample_edid( long* edidlen );
int      rtsp_read_file_into_buffer( const char* filepath, uint8_t** buffer, long* buf_len );
rtsp_param_p rtsp_add_parameter( rtsp_handle h, const char* param_name, const char* param_value );
int rtsp_free_parameters( rtsp_handle h );
const char* rtsp_get_parameter( const rtsp_handle h, const char* label );

int rtsp_param_set( rtsp_handle h, const char* key, const char* value );
int rtsp_checksum_ok( const rtsp_handle h );

#endif
