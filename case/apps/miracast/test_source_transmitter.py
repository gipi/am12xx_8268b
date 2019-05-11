#!/usr/bin/env python

# @file: test_source_transmitter.py
# @brief Python script to simulate a host sending RDS/RTSP commands
#
# INTEL CONFIDENTIAL
# Copyright 2010 Intel Corporation All Rights Reserved.
#
# The source code contained or described herein and all documents related to 
# the source code ("Material") are owned by Intel Corporation or its 
# suppliers or licensors.  Title to the Material remains with Intel 
# Corporation or its suppliers and licensors.  The Material contains trade 
# secrets and proprietary and confidential information of Intel or its 
# suppliers and licensors.  The Material is protected by worldwide copyright 
# and trade secret laws and treaty provisions. No part of the Material may 
# be used, copied, reproduced, modified, published, uploaded, posted, 
# transmitted, distributed, or disclosed in any way without Intel's prior 
# express written permission.
#
# No license under any patent, copyright, trade secret or other intellectual 
# property right is granted to or conferred upon you by disclosure or 
# delivery of the Materials,  either expressly, by implication, inducement, 
# estoppel or otherwise.  Any license under such intellectual property 
# rights must be express and approved by Intel in writing.

from socket import *
from time import sleep
#from random import shuffle, random, choice
import string
import random
import re

# Set the socket parameters
listen_ip = "10.23.124.40" 
listen_port = 2027


class rtsp_tester:
    """Runs RTSP tests on received payloads and keeps track of
    pass/fail statistics."""
    def __init__( self ):
        self.tests_run = 0
        self.tests_failed = 0
        self.tests_passed = 0

    def info( self, mesg ):
        """Prints info message."""
        print "info: " + mesg.rstrip()

    def warn( self, mesg ):
        """Prints info message."""
        print "warning: " + mesg.rstrip()

    def failed( self, mesg ):
        """Prints error message and increments global error count."""
        self.tests_failed += 1
        print "fail: " + mesg.rstrip()

    def passed( self, mesg ):
        self.tests_passed += 1
        print "pass: " + mesg.rstrip()

    def rtsp_assert( self, true_if_passed, assert_mesg ):
        self.tests_run += 1
        if true_if_passed:
            self.passed( assert_mesg )
        else:
            self.failed( assert_mesg )
        return not true_if_passed

    def check_header( self, needle, haystack ):
        ok = False
        for i in haystack:
            if re.match( needle, i ) != None:
            #if i.count( needle ) > 0:
                ok = True
                break
        return self.rtsp_assert( ok,
            "Required header [%s] exists" % needle )

    def check_parameter( self, param, value, in_params ):
        found = False
        for i in in_params:
            matches = re.match( r"(%s): (%s)$" % (param,value), i )
            if matches:
                found = True
                break
        return self.rtsp_assert( found,
            "Parameter [%s] has value [%s]" % (param, value) )

    def print_stats( self ):
        self.info( "test summary: %d total, %d pass, %d fail" % (
            self.tests_run, self.tests_passed, self.tests_failed ) )


class rtsp_connection:
    def __init__( self, my_ip_address, port, sink=False, source=False,
        other_ip_address=None, other_port=None ):
        if sink and source:
            raise Exception( "Cannot specify both sink and source mode" )
        if (not sink) and (not source):
            raise Exception( "Did not specify sink or source mode" )

        self.RTSP_COMMANDS = [ "RTSP/1.0", "OPTIONS", "SETUP", "PLAY",
                "TEARDOWN", "GET_PARAMETER", "SET_PARAMETER" ]
        self.RTSP_OK = "RTSP/1.0 200 OK"

        self.sink = sink
        self.source = source
        self.my_ip_address = my_ip_address
        self.other_ip_address = other_ip_address
        self.port = port
        self.other_port = other_port
        self.my_cseq = -1
        self.other_cseq = None
        self.sock = None
        self.conn = None
        self.remote_addr = None
        self.crlf = "\r\n"
        self.test = rtsp_tester()
        self.randomize_headers = True
        self.fake_headers = True
        self.received_data = ""
        self.session = ''.join(random.choice(string.letters) for i in xrange(6))

    def open_listen_socket( self ):
        addr = ( self.my_ip_address, self.port )
        self.sock = socket( AF_INET, SOCK_STREAM )
        self.sock.setsockopt( SOL_SOCKET, SO_REUSEADDR, 1 )
        #self.sock.settimeout( 5 )
        self.sock.bind( addr )

        self.test.info( "Waiting for connection on %s:%d..." % addr )
        self.sock.listen( 1 )
        self.conn, self.remote_addr = self.sock.accept()

        self.test.info( "Incoming connection from sink %s:%d" % self.remote_addr )

    def connect_to_sink( self ):
        addr = ( self.other_ip_address, self.other_port )
        self.conn = socket( AF_INET, SOCK_STREAM )
        #self.conn.settimeout( 5 )

        other = "source" if self.sink else "sink"
        self.test.info( "Connecting to %s %s:%d..." % ( other, addr[0],
            addr[1]) )
        self.conn.connect( addr )

    def read_socket( self ):
        """Reads all available data from TCP socket into internal
        buffer."""
        incoming = self.conn.recv( 4096 )
        other = "source" if self.sink else "sink"
        self.test.info( "received payload from %s <<<%s>>>" % ( other,
            incoming ) )
        self.received_data += incoming

    def close_socket( self ):
        if self.source:
            self.sock.close()
        else:
            self.conn.close()

    def start( self ):
        if self.source:
            self.open_listen_socket()
        else:
            self.connect_to_sink()

    def stop( self ):
        self.close_socket()

    def get_fake_header( self ):
        return "Fake-Header-%s: %s" % ( str(random.random()), str(random.random()) )

    def get_header( self, label, headers):
        for i in headers:
            field, val = i.split(':')
            if field.lower() == label.lower():
                return val
        return None

    @property
    def rds_session_timeout( self ):
        return random.randrange( 1, 90 )

    def normalize_params( self, params ):
        out = []
        for i in params:
            out.append( (i,i) if type(i) == str else i )
        return out

    def send( self, command, headers=None, params=None, cseq=None ):
        """Sends payload to other side.  Automatically adds CSeq and
        Content-Length.  Automatically increments CSeq. Randomizes headers
        if requested for additional check. Sends extra fake headers if
        requested for additional check."""
        if not headers: headers=[]
        if not params: params=[]
        params = self.normalize_params( params )

        headers.append( "CSeq: %d" % cseq )
        if self.fake_headers:
            headers.append( self.get_fake_header() )
        if self.randomize_headers:
            random.shuffle( params )

        params_text = self.crlf.join( [ i for i,v in params ] )
        if len(params_text) > 0:
            headers.append( "Content-Length: %d" % len(params_text) )
            headers.append( "Content-Type: text/parameters" )
        if self.randomize_headers: 
            random.shuffle( headers )
        headers_text = self.crlf.join( headers ) + self.crlf

        payload = self.crlf.join( (command, headers_text, params_text) )
        self.conn.send( payload )
        other = "source" if self.sink else "sink"
        self.test.info( "sent payload to %s >>>%s<<<" % ( other,
            payload ) )

    def advance_buffer( self, bytes_to_advance ):
        self.received_data = self.received_data[ bytes_to_advance: ]

    def is_valid_rtsp_command( self, command_to_test ):
        for i in self.RTSP_COMMANDS:
            if command_to_test.startswith( i ):
                return True
        return False

    def get_rtsp_message( self ):
        """Returns a tuple with a fully-formed RTSP message:
        (command,headers,content) """
        self.read_socket() # make sure we get any waiting data

        # split up the received data by CRLF
        # for each CRLF at the end of the string, .split() returns a ""
        lines = self.received_data.split( self.crlf )

        # search until we find a valid RTSP command
        command = None
        while not command:
            if len(lines) == 0:
                raise Exception( "No data received but was expecting some" )
            command = lines.pop( 0 )
            self.advance_buffer( len(command + self.crlf) )
            if not self.is_valid_rtsp_command( command ):
                self.test.rtsp_assert( False, 
                    "expecting valid RTSP message header, received [%s]" % command )
                command = None

        self.test.rtsp_assert( True, "received valid RTSP message header" )

        # extract the headers (up to the next CRLF)
        headers = []
        while len(lines) > 0:
            next_input = lines.pop( 0 )
            if next_input == "": 
                break # end of headers
            headers.append( next_input )
            self.advance_buffer( len(next_input) + len(self.crlf) )

        # now we need a CRLF to end the headers
        # due to unpredictable behavior from .split(), we'll test the receive
        # buffer directly
        eol = self.received_data[0:2]
        self.test.rtsp_assert( eol == self.crlf, "CRLF after headers" )
        self.advance_buffer( len(eol) )

        # If we have a Content-Length header, read that number of bytes
        # raw
        try:
            val = int( self.get_header( 'content-length', headers ) )
            content = self.received_data[:val]
            self.advance_buffer( val )
        except:
            content = ""

        return ( command, headers, content )

    def send_request( self, command, headers=None, params="" ):
        self.my_cseq += 1
        return self.send( command, headers, params, cseq=self.my_cseq )

    def send_response( self, headers=None, params=None ):
        command = self.RTSP_OK
        return self.send( command, headers, params, cseq=self.other_cseq )

    def verify( self, rtsp_message, ref_command, ref_headers=None, params=None,
            cseq=None ):
        """Verifies that RTSP payload has correct command and headers (in
        any order.).  CSeq and Content-Length (if any) will automatically be
        checked. Parameters to check is an array of lists, with the
        expected parameter and the expected value (as a regex)
            [ ( "rds_field1", "expected_value1" ),
              ( "rds_field2", ".*" ),
              ... ]
        """
        command_in, headers_in, content_in = rtsp_message
        if self.test.rtsp_assert( command_in.count( ref_command ) > 0,
                "received command [%s] == expected command [%s]" % (
                command_in, ref_command ) ):
            return rtsp_message

        # Add sequence number and parameter headers (if any)
        if not ref_headers: ref_headers=[]
        ref_headers.append( "CSeq: %d" % cseq )
        if not params: params=[]
        if len(params) > 0:
            actual_length = len( content_in )
            ref_headers.append( "Content-Type: text/parameters" )
            ref_headers.append( "Content-Length: %d" % actual_length ) 

        # Check for headers in any order
        for req_header in ref_headers:
            self.test.check_header( req_header, headers_in )

        # Check for required parameters and values
        content_lines = content_in.split( self.crlf )
        for p, v in self.normalize_params( params ):
            self.test.check_parameter( p, v, content_lines )
        #for i in params:
            #p, v = i, i if type(i) == str else i
            #self.test.check_parameter( p, v, content_lines )

        return rtsp_message

    def verify_response( self, headers=None, params=None ):
        ref_command = self.RTSP_OK
        return self.verify( self.get_rtsp_message(), ref_command,
                headers, params, cseq=self.my_cseq )

    def adjust_incoming_cseq( self, rtsp_message ):
        if not self.other_cseq:
            self.other_cseq = int( self.get_header( 'cseq', rtsp_message[1] ) )
        else:
            self.other_cseq += 1

    def verify_request( self, command, headers=None, params=None ):
        message = self.get_rtsp_message()
        self.adjust_incoming_cseq( message )
        return self.verify( message, command, headers, params,
                cseq=self.other_cseq )

    def send_receive( self, source_opts, sink_opts ):
        if self.source:
            if source_opts.get( 'sender', False ):
                first = self.send_request
                second = self.verify_response
            else:
                first = self.verify_request
                second = self.send_response

            command = source_opts.get( 'command', None )
            headers = source_opts.get( 'headers', None )
            params = source_opts.get( 'params', None )
            first( command=command, headers=headers, params=params )

            headers = sink_opts.get( 'headers', None )
            params = sink_opts.get( 'params', None )
            second( headers=headers, params=params )
        else:
            if source_opts.get( 'sender', True ):
                first = self.verify_request
                second = self.send_response
            else:
                first = self.send_request
                second = self.verify_response

            command = source_opts.get( 'command', None )
            headers = source_opts.get( 'headers', None )
            params = source_opts.get( 'params', None )
            first( command=command, headers=headers, params=params )

            headers = sink_opts.get( 'headers', None )
            params = sink_opts.get( 'params', None )
            second( headers=headers, params=params )
            


def send_options( rtsp ):
    source = {
        'command': "OPTIONS * RTSP/1.0",
        'headers': [ "Require: com.intel.rds1.0" ],
        'sender':  True,
    }
    sink = {
        'headers': [ "Public: com.intel.rds1.0, SET_PARAMETER, GET_PARAMETER" ],
    }
    rtsp.send_receive( source, sink )

def receive_options( rtsp ):
    source = {
        'command': "OPTIONS * RTSP/1.0",
        'headers': [ "Require: com.intel.rds1.0" ],
        'sender':  False,
    }
    sink = {
        'headers': [ "Public: com.intel.rds1.0, SET_PARAMETER, " + 
                        "GET_PARAMETER, PLAY, TEARDOWN, SETUP" ],
    }
    rtsp.send_receive( source, sink )

def get_parameters( rtsp ):
    param_list = """rds_friendly_name rds_unique_device_id rds_status rds_audio_formats 
        rds_video_formats rds_rtp_profile rds_content_protection rds_sink_version""".split()
    if rtsp.randomize_headers:
        random.shuffle( param_list )
    param_text = ", ".join( param_list )

    source = {
        'command': "GET_PARAMETER rtsp://localhost/rds1.0 RTSP/1.0",
        'params':  [ param_text ],
        'sender':  True,
    }
    sink = {
        'params':  [ ( "rds_friendly_name", ".{1,18}" ),
                        ( "rds_unique_device_id", "[0-9A-Fa-f]{12}" ),
                        ( "rds_status", "busy=[01], display_connected=(?:1|0(?:, YPbPr)?)" ),
                        ( "rds_video_formats", ".*" ),
                        ( "rds_rtp_profile", ".*" ),
                        ( "rds_sink_version", ".*" ), ]
    }
    rtsp.send_receive( source, sink )

    #RE for video_formats
    #import re

    #rvf=r"(?:MPEG-2-MP@HL|RDS-CLASS[123]|RDS-CLASS1b)"
    #rvfs=rvf+"(?, "+rvf+"){0,4}"

    #test1 = "RDS-CLASS1"
    #re.match( rvfs, test1 )


def set_trigger( rtsp ):
    source = {
        'command': "SET_PARAMETER rtsp://localhost/rds1.0 RTSP/1.0",
        'params':  [ "rds_trigger_method: SETUP",
                        "rds_overscan_comp: x=0, y=15",
                        "rds_presentation_URL: rtsp://%s/rds1.0/streamid=0" % rtsp.my_ip_address, ],
        'sender':  True,
    }
    sink = { }
    rtsp.send_receive( source, sink )


def get_edid( rtsp ):
    source = {
        'command': "GET_PARAMETER rtsp://localhost/rds1.0 RTSP/1.0",
        'params':  [ "rds_display_edid" ],
        'sender':  True,
    }
    sink = { 'params':  [ ( "rds_display_edid", ".*" ) ], }
    rtsp.send_receive( source, sink )


def receive_setup( rtsp ):
    source = {
        'command': "SETUP rtsp://%s/rds1.0/streamid=0 RTSP/1.0" % rtsp.my_ip_address,
        'headers': [ r"Transport: RTP/AVPF/UDP;unicast;client_port=\d+-\d+;mode=play" ],
        'sender':  False,
    }
    sink = {
        'headers': [ "Transport: RTP/AVP/UDP;unicast;client_port=12000-12001;server_port=5600-5601;mode=play",
                     "Session: %s" % rtsp.session, ]
    }
    rtsp.send_receive( source, sink )

    #rtsp.test.warn( "sending incorrect transport header" )
    # implement as a closure and pass function to send_receive to run in between??
    #transport_in = rtsp.get_header( 'transport', rtsp_msg[1] )



def receive_play( rtsp ):
    #rtsp.test.warn( "hard coding adapter IP address" )
    #global listen_ip
    #ip_addr = listen_ip
    ip_addr = rtsp.my_ip_address if rtsp.source else rtsp.other_ip_address
    session_header = "Session: %s" % rtsp.session

    source = {
        'command': "PLAY rtsp://%s/rds1.0/streamid=0 RTSP/1.0" % ip_addr,
        'headers': [ session_header ],
        'sender':  False,
    }
    sink = { 'headers': [ session_header ], }
    rtsp.send_receive( source, sink )


def send_keepalives( rtsp ):
    session_header = "Session: %s" % rtsp.session

    num = 2
    rest = 10
    rtsp.test.info( "will send %d keep alives (test duration %d seconds)" % (
        num, rest*num ) )
    while True:
        source = {
            'command': "SET_PARAMETER rtsp://localhost/rds1.0 RTSP/1.0",
            'headers': [ session_header ],
            'params':  [ "rds_keepalive:", ],
            'sender':  True,
        }
        sink = { 'headers': [ session_header ], }

        rtsp.send_receive( source, sink )

        num -= 1
        if num <= 0: break

        rtsp.test.info( "sleeping for %d s before sending next keep alive" % rest )
        sleep(rest)


def send_teardown_trigger( rtsp ):
    session_header = "Session: %s" % rtsp.session
    source = {
        'command': "SET_PARAMETER rtsp://localhost/rds1.0 RTSP/1.0",
        'headers': [ session_header ],
        'params':  [ "rds_trigger_method: TEARDOWN", ],
        'sender':  True,
    }
    sink = { 'headers': [ session_header ], }
    rtsp.send_receive( source, sink )

def receive_teardown( rtsp ):
    session_header = "Session: %s" % rtsp.session
    source = {
        'command': "TEARDOWN rtsp://%s/rds1.0/streamid=0 RTSP/1.0" % rtsp.my_ip_address,
        'headers': [ session_header ],
        'sender':  False,
    }
    sink = { 'headers': [ session_header ], }
    rtsp.send_receive( source, sink )

def get_session_timeout( rtsp ):
    """ not used """
    params = [ "rds_session_timeout", "rds_max_session_timeout" ]
    rtsp.get_parameters( params )

    var_list = "rds_session_timeout rds_max_session_timeout".split()
    if rtsp.randomize_headers:
        random.shuffle( var_list )

    params = ", ".join( var_list )
    rtsp.send_request( command, params=params )

    #RE for video_formats
    #import re

    #rvf=r"(?:MPEG-2-MP@HL|RDS-CLASS[123]|RDS-CLASS1b)"
    #rvfs=rvf+"(?, "+rvf+"){0,4}"

    #test1 = "RDS-CLASS1"
    #re.match( rvfs, test1 )

    expecting_parameters = [
            ( "rds_friendly_name", ".{1,18}" ),
            ( "rds_unique_device_id", "[0-9A-Fa-f]{12}" ),
            ( "rds_status", "busy=[01], display_connected=(?:1|0(?:, YPbPr)?)" ),
            ( "rds_video_formats", ".*" ),
            ( "rds_rtp_profile", ".*" ),
            ( "rds_sink_version", ".*" ),
        ]
    rtsp.verify_response( params=expecting_parameters )


BASIC_TEST = [
    send_options, 
    receive_options,
    get_parameters,
    get_edid,
    set_trigger,
    receive_setup,
    receive_play,
    send_keepalives,
    send_teardown_trigger,
    receive_teardown,
]

def test_remote_device( rtsp ):
    try:
        rtsp.start()
        testnum = 1
        for test in BASIC_TEST:
            print "--- beginning test %d: %s ---" % ( testnum, test.__name__ )
            test( rtsp )
            print "--- finished test: %s ---" % test.__name__ 
            testnum += 1
    finally:
        rtsp.stop()
        rtsp.test.print_stats()


sink = False
source = not sink
rtsp = rtsp_connection( listen_ip, listen_port, sink=sink,
        source=source, other_ip_address=listen_ip, other_port=2027)

test_remote_device( rtsp )
