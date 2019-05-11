/**
 * @file sample_rtsp_sink_test.c
 *
 * Unit test harness for RTSP sink library code.
 *
 * INTEL CONFIDENTIAL
 * Copyright 2012 Intel Corporation All Rights Reserved.
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

#define WIDI_TEST
#include "sample_rtsp_sink.c"

#include <check.h>
/*#include <stdlib.h>*/
/*#include <unistd.h>*/
/*#include <pthread.h>*/

START_TEST( test_1 )
{
    fail_if( 0 );
}
END_TEST

//
//
//

Suite *
test_suite( void )
{
    Suite* s = suite_create( "librtspsink" );

    TCase* tc_core = tcase_create( "Core" );
    tcase_add_test( tc_core, test_1 );
    suite_add_tcase( s, tc_core );

    return s;
}

int main( void )
{
    int number_failed;

    Suite* s = test_suite();
    SRunner* sr = srunner_create( s );
    srunner_run_all( sr, CK_NORMAL );
    number_failed = srunner_ntests_failed( sr );
    srunner_free( sr );
    return ( number_failed == 0 ) ? EXIT_SUCCESS : EXIT_FAILURE;

}
