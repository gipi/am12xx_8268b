
#include "common_d.h"

HeaderDecl(IntelWFDParams)
	// intel-enable-widi-rtcp = “intel_enable_widi_rtcp:” SP IPPORT CRLF
	ItemDecl(intel_enable_widi_rtcp)
	
	// intel-sink-version = “intel_sink_version:” SP product-id SP hw-version SP sw-version CRLF
	// product-id = "product_ID=" 1*16VCHAR
	// hw-version = "hw_version=" version-tag 
	// hw-version = "sw_version=" version-tag 
	// version-tag = major "." minor "." sku "." build 2
	// major = 1*2(DIGIT)
	// minor = 1*2(DIGIT)
	// sku = 1*2(DIGIT)
	// build = 1*4(DIGIT)
	ItemDecl(intel_sink_version)
	
	// intel-friendly-name = “intel_friendly_name:” SP 1*18(vchar_no_hyphen) CRLF
	// vchar-no-hyphen = %x21-2C / %x2E-7E
	ItemDecl(intel_friendly_name)
	
	// intel-sink-manufacturer-name = “intel_sink_manufacturer_name:” SP 1*32ALPHA CRLF
	ItemDecl(intel_sink_manufacturer_name)
	
	// intel-sink-model-name = “intel_sink_model_name:” SP 1*32ALPHA CRLF
	ItemDecl(intel_sink_model_name)
	
	// intel-sink-device-URL = “intel_sink_device_URL:” SP 1*255VCHAR CRLF
	ItemDecl(intel_sink_device_URL)
	
	// intel-sink-manufacturer-logo = “intel_sink_manufacturer_logo:” SP logo CRLF
	// logo = "none" / base64_logo
	// base64_logo = 464*76800(BASE64CHAR)
	ItemDecl(intel_sink_manufacturer_logo)
	
	// intel-lower-bandwidth = “intel_lower_bandwidth:” SP ("0"|"1") CRLF
	ItemDecl(intel_lower_bandwidth)

	// intel-sigma-pipeline-params = “intel_sigma_pipeline_params:” SP "PlaybackDelay=" 1*3DIGIT ";" SP "PositiveMaxStcPCR=" 1*3DIGIT ";" SP "NegativeMaxStcPCR=" 1*3DIGIT ";" SP CRLF
	ItemDecl(intel_sigma_pipeline_params)

	// intel-overscan-comp = “intel_overscan_comp:” SP "x=" ("0".."15") "," SP "y=" ("0".."15") CRLF
	ItemDecl(intel_overscan_comp)

	// intel-wfd-usboip       = “intel_usboip:” SP usboip-support CRLF
	// usboip-support         = "none" / usboip-supported
	// usboip-supported       = "version=" usboip-version SP usboip-host-or-client SP usboip-active 
	// usboip-version         = “version=” 1*3(DIGIT) “.” 1(DIGIT) 
	// usboip-host-or-client  = ( "host=" / "client=" ) IPADDRESS
	// usboip-active          = "active=0" / ( "active=1" usboip-opt-args )
	// usboip-opt-args        = SP "discovery_port=" IPPORT // only sent for host
	
	ItemDecl(intel_usboip)
 
TailDecl(IntelWFDParams)

HeaderDecl(USBoIPType)
	ItemDecl2(USBoIP_host, host)
	ItemDecl2(USBoIP_device, device)
TailDecl(USBoIPType)


