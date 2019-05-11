#ifndef _ACT_PICASA_CURL_H_

#define _ACT_PICASA_CURL_H_

#include "act_picasa_common.h"

typedef enum{
	Picasaweb_OK=0,
	
	///< the following error is occured by auth see http://code.google.com/intl/zh-CN/apis/accounts/docs/AuthForInstalledApps.html
	Picasaweb_Auth_BadAuthentication=1, 	///< The login request used a username or password that is not recognized.
	Picasaweb_Auth_CaptchaRequired =2,	///< A CAPTCHA is required.
	Picasaweb_Auth_NotVerified=3,		///< The account email address has not been verified.
	Picasaweb_Auth_TermsNotAgreed=4,	///< The user has not agreed to terms.
	Picasaweb_Auth_AccountDeleted=5,	///< The user account has been deleted.
	Picasaweb_Auth_AccountDisabled=6,	///< The user account has been disabled.
	Picasaweb_Auth_ServiceDisabled=7,	///< The user's access to the specified service has been disabled. 
	Picasaweb_Auth_ServiceUnavailable=8,	///< The service is not available; try again later.
	PicasaWeb_Auth_Unknown=9,		///< The error is unknown or unspecified; 
	
}PicasawebCode_e;


typedef struct picasaweb_entry_info_s
{
	picasaweb_data_atomic_t id_url;
	picasaweb_data_atomic_t published;
	picasaweb_data_atomic_t updated;
	picasaweb_data_atomic_t edited;
	picasaweb_data_atomic_t scheme;
	picasaweb_data_atomic_t title;
	picasaweb_data_atomic_t summary;
	picasaweb_data_atomic_t rights;
	
}picasaweb_entry_info_t;



#endif
