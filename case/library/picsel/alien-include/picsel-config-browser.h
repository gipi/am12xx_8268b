/**
 * Browser Configurations
 *
 * This file contains the key strings that can be used to set configuration
 * which are relevant to browsing websites
 *
 * @file
 * $Id: picsel-config-browser.h,v 1.61 2013/03/14 14:57:24 hugh Exp $
 */
/* Copyright (C) Picsel, 2006-2008. All Rights Reserved. */
/**
 * @defgroup TgvBrowserConfig Configuration settings for Browser
 * @ingroup TgvBrowser
 *
 * Configuration settings available in Browser.
 * These are key strings that can be used to set configuration
 * which are relevant to browsing websites. They should be set using
 * PicselConfig_setInt() or PicselConfig_setString().
 *
 * @{
 */

#ifndef PICSEL_CONFIG_BROWSER_H
#define PICSEL_CONFIG_BROWSER_H


/**
 * Set to enable JavaScript in the browser
 *
 * The default value is 1 (on); set to 0 (off) to disable Javascript
 */
#define PicselConfigBr_enableJs                "EnableJS"

/**
 * Set to enable AJAX support in the browser
 *
 * The default value is 1 (on); set to 0 (off) to disable AJAX
 */
#define PicselConfigBr_enableAjax              "EnableAjax"

/**
 * Set to enable browser's HTTP and/or HTTPS proxy settings
 *
 * The default value is 0 (off); set to 1 (on) to enable proxy settings
 */
#define PicselConfigBr_enableProxy             "HttpProxyEnable"

/**
 * Define HTTP Proxy to access web sites on the internet
 *
 * Format: host name followed by a colon and a port number
 *         http://proxyname:port
 * E.g.  : "proxy.isp.net:3128"
 *
 * Default : "" (empty string)
 */
#define PicselConfigBr_httpProxy               "HttpProxy"

/**
 * Define HTTPS Proxy to access secure sites on the internet.
 *
 * Format: host name followed by a colon and a port number
 *         https://proxyname:port
 * E.g.  : "proxy.isp.net:3128"
 *
 * Default : NULL
 */
#define PicselConfigBr_httpsProxy              "Picsel_httpsProxy"

/**
 * Define to exclude sites from proxy server settings, allowing us to
 * access those sites directly.
 *
 * Format: Single or multiple addresses (separated by space).
 * E.g.  : "www.bbc.co.uk"
 *
 * Default : "" (empty string)
 */
#define PicselConfigBr_noProxyFor              "NoProxyFor"

/**
 * Set to enable HTTP pipelining
 *
 * Default value is 1 (on);
 * Set to 0 (off) to disable pipelining
 */
#define PicselConfigBr_enablePipelining        "Picsel_enablePipelining"

/**
 * Set the number of pipelined HTTP requests that can be sent in
 * one operation
 *
 * Default value is 12. Value must be 1 or greater.
 *
 * Has no effect unless pipelining is enabled.
 */
#define PicselConfigBr_numPipelinedRequests    "Picsel_numPipelinedRequests"

/**
 * Set to configure the maximum number of tcp sockets that can be used
 *
 * The default value is 64
 *
 */
#define PicselConfigBr_maxSockets              "Picsel_maxSockets"

/**
 * Set to configure the maximum number of tcp sockets that can be
 * simultaneously open to a proxy server.
 *
 * The default value is the lower of PicselConfigBr_maxSockets and 4.
 */
#define PicselConfigBr_maxSocketsPerProxy      "Picsel_maxSocketsPerProxy"

/**
 * Set to configure the maximum number of tcp sockets that can be
 * simultaneously open to a server (when a proxy server is not used)
 *
 * The default value is the lower of PicselConfigBr_maxSockets and 2.
 * (2 is the recommended maximum in RFC 2616).
 */
#define PicselConfigBr_maxSocketsPerServer     "Picsel_maxSocketsPerServer"

/**
 * The connection idle check time, in seconds.  If a socket has been
 * unused for more than this number of seconds, we will issue a
 * PicselTcpCheckStatus request before attempting to use it.  (@see
 * PicselTcpRequestType)
 *
 * If set to 0 (the default), then we do not probe idle connections
 * before using them.
 */
#define PicselConfigBr_tcpIdleCheckStatusTime  "Picsel_tcpIdleCheckStatusTime"

/**
 * Set to configure the maximum content length, in bytes, where content will
 * be read and discarded for HTTP status codes 301, 302, 303, 307 and 401,
 * when the HTTP response has indicated that the connection is persistent.
 * The default value is 0, which means the content will always be read and
 * discarded when the connection is persistent, this sensibly allows the tcp
 * connection to be used for subsequent requests.
 * If a value between 1 and 1000 is specified, the configured value will be
 * set to 1000.
 *
 * When this setting is greater than 0, the tcp connection will be closed and
 * a new one opened if one or more of the following conditions are met for a
 * 301, 302, 303, 307 or 401 HTTP response:-
 *    - The Content-Length exceeds the value of this setting.
 *    - The response does not contain a Content-Length header.
 *    - The response indicates that chunked Transfer-Encoding is used.
 *
 * Picsel strongly recommends leaving this setting at the default value.
 * Typically, the amount of content for these status codes is less than
 * 400 bytes, so the content will already have been fetched from the network
 * when the HTTP headers are read.
 * Closing the tcp connection and opening another one would not reduce the
 * amount of data read from the network and would incur a delay while the
 * new connection is opened.
 */
#define PicselConfigBr_maxConsumeContent  "Picsel_maxConsumeContent"

/**
 * The size of the network buffer used in Picsel's HTTP implementation.
 *
 * The default value is 8K.
 *
 * Increasing this value may provide better network performance, but will
 * use more memory.
 *
 */
#define PicselConfigBr_httpBufferSize          "Picsel_httpBufferSize"

/**
 * Possible values for HTTP referrer policy
 */
typedef enum PicselConfigBr_HttpReferrer_Policy
{
    PicselConfigBr_HttpReferrer_Send = 0,    /**< Always send referrer */
    PicselConfigBr_HttpReferrer_Hide,        /**< Never send referrer */
    PicselConfigBr_HttpReferrer_Confirm      /**< Always confirm with user.
                                                  The confirmation will be
                                                  sent in a call to
                                                  AlienUserRequest_request */
}
PicselConfigBr_HttpReferrer_Policy;

/**
 * Set the policy for sending the HTTP referrer.  The default is always
 * send.  Use values from the PicselConfigBr_HttpReferrer_Policy
 * enumeration.
 */
#define PicselConfigBr_httpReferrerPolicy      "Picsel_httpReferrerPolicy"

/**
 * Possible values for form AutoComplete policy.
 * Form AutoComplete automatically fills an HTML form with previously saved
 *  data(if available) when the page is loaded, and saves the form data for
 *  future use when it is submitted.
 *
 * @note
 *
 * - Even with form AutoComplete disabled, scripts may still
 *   automatically fill a text input field.
 * - For username and password forms, URL string must be identical
 *   (including query fields) to previous for AutoComplete to occur.
 */
typedef enum PicselConfigBr_FormAutoComplete_Policy
{
    PicselConfigBr_FormAutoComplete_Disable = 65536,
    PicselConfigBr_FormAutoComplete_Enable,
    PicselConfigBr_FormAutoComplete_Confirm   /**< Confirm with the user
                                                   whether to perform
                                                   the form AutoComplete
                                                   actions. Only one
                                                   confirmation per page
                                                   is required */
}
PicselConfigBr_FormAutoComplete_Policy;

/**
 *  Set the policy for form AutoComplete. The default is disable.
 *  Use values from the PicselConfigBr_FormAutoComplete_Policy enumeration.
 */
#define PicselConfigBr_formAutoCompletePolicy      "Picsel_formAutoCompletePolicy"

/**
 * Indicates whether a single textfield/textarea/password form should auto submit
 * when edited. Forms which have no submit button, and a single text input control
 * are automatically submitted when data has been changed.
 *
 * Disabling this behaviour will mean there is no way to submit forms that do not
 * have a submit button.
 *
 * Default value is 1 (auto-submit);
 * Set to 0 (off) to disable auto-submission.
 */
#define PicselConfigBr_formAutoSubmit              "Picsel_formAutoSubmit"

/**
 * Possible values for HTTP redirection policy
 */
typedef enum PicselConfigBr_HttpRedirection_Policy
{
    PicselConfigBr_HttpRedirection_AllowAll = 0,   /**< Allow redirections,
                                                        up to a maximum set
                                                        by the config
                                                        below */
    PicselConfigBr_HttpRedirection_ConfirmAll,     /**< Confirm every
                                                        redirection with the
                                                        user */
    PicselConfigBr_HttpRedirection_ConfirmInsecure /**< Confirm every
                                                        insecure
                                                        redirection with the
                                                        user (HTTPS->HTTP) */
}
PicselConfigBr_HttpRedirection_Policy;

/**
 * Set the policy for HTTP redirection.  The default is to confirm insecure
 * redirections with the user.  Use values from the
 * PicselConfigBr_HttpRedirect_Policy enumeration.
 */
#define PicselConfigBr_httpRedirectionPolicy   "Picsel_httpRedirectionPolicy"

/**
 * Set the maximum number of consecutive HTTP redirections.  Default is 8.
 */
#define PicselConfigBr_maxHttpRedirections     "Picsel_maxHttpRedirections"

/**
 * Sets the local path of a user stylesheet that will be used to
 * specify default styling for pages.  If this is not provided, default
 * styling will be used.
 */
#define PicselConfigBr_userStyleSheet          "Picsel_userStyleSheet"

/**
 * Possible values for visited links policy
 */
typedef enum PicselConfigBr_VisitedLinks_Policy
{
    PicselConfigBr_VisitedLinks_Never = 0,    /**< Never mark a link as
                                                   visited */
    PicselConfigBr_VisitedLinks_CacheOnly,    /**< Mark a link as visited if
                                                   it is in the cache. */
    PicselConfigBr_VisitedLinks_AlienOnly,    /**< Mark a link as visited if
                                                   true is returned from
                                                   AlienBrowser_linkVisited */
    PicselConfigBr_VisitedLinks_CacheAndAlien /**< Mark a link as visited if
                                                   if a link is in the cache
                                                   or if true is returned from
                                                   AlienBrowser_linkVisited */
}
PicselConfigBr_VisitedLinks_Policy;

/**
 * Set the policy for visited links, which decides whether links should be
 * marked as visited.  If this is set to _AlienOnly or _CacheAndAlien,
 * AlienBrowser_linkVisited must be implemented (see alien-browser.h).  This
 * setting will only take effect when set in AlienConfig_ready.
 *
 * The default is PicselConfigBr_VisitedLinks_Never.
 */
#define PicselConfigBr_visitedLinksPolicy  "Picsel_visitedLinksPolicy"

/**
 * Set the download limit for a page in bytes.  If this is not provided,
 * there is no download limit for pages.
 *
 * When the download limit is reached, an
 * AlienInformation_DownloadLimitReached event is sent.
 *
 */
#define PicselConfigBr_downloadLimit           "Picsel_downloadLimit"

/**
 * Maximum number of cookies that will be stored, in total.
 * If this is not set, the default is 300.
 */
#define PicselConfigBr_maxCookies               "Picsel_maxCookies"

/**
 * Maximum number of cookies that will be stored per server.
 * If this is not set, the default is 20.
 */
#define PicselConfigBr_maxCookiesPerServer      "Picsel_maxCookiesPerServer"

/**
 * File path (or RAM: address) at which to store the cookie policies file.
 * Default is [PicselConfig_settingsPath]/cpolicies.dat.
 *
 * The maximum size of this file is 32 bytes.
 */
#define PicselConfigBr_cookiesPoliciesFile       "Picsel_cookiesPoliciesFile"

/**
 * File path (or RAM: address) at which to store the cookie index file.
 * Default is [PicselConfig_settingsPath]/cindex.dat.
 *
 * The maximum size of this file is (20 * max cookies) bytes.
 */
#define PicselConfigBr_cookiesIndexFile          "Picsel_cookiesIndexFile"

/**
 * File path (or RAM: address) at which to store the cookie data file.
 * Default is [PicselConfig_settingsPath]/cdata.dat.
 *
 * The maximum size of this file is (4096 * max cookies) bytes
 */
#define PicselConfigBr_cookiesDataFile           "Picsel_cookiesDataFile"

/**
 * Suggested number of cookies to get on each PicselBrowser_getCookies call
 * Default is 5.
 */
#define PicselConfigBr_cookiesPerGet             "Picsel_cookiesPerGet"

/**
 * File path (or RAM: address) at which to store the history list file.
 * Default is [PicselConfig_settingsPath]/history.dat.
 */
#define PicselConfigBr_historyListFile           "Picsel_historyListFile"

/**
 * File path (or RAM: address) at which to store the form AutoComplete file.
 * Default is [PicselConfig_settingsPath]/ac.dat.
 */
#define PicselConfigBr_formAutoCompletePath      "Picsel_formAutoCompletePath"

/**
 * Set the timeout period for any HTTP write operation. This must be stored
 * as whole seconds. If not set the default is 60.
 *
 * Setting the value to '0' disables the timer.
 */
#define PicselConfigBr_httpSendTimer             "Picsel_HTTP_sendTimer"

/**
 * Set the timeout period for any HTTP read operation. This must be stored
 * as whole seconds. If not set the default is 60.
 *
 * Setting the value to '0' disables the timer.
 */
#define PicselConfigBr_httpReadTimer             "Picsel_HTTP_receiveTimer"

/**
 * Set the timeout period when an idle socket will be closed. This must be
 * stored as whole seconds. If not set the default is 120.
 *
 * Setting the value to '0' disables the timer.
 */
#define PicselConfigBr_tcpIdleTimeout            "Picsel_TCP_idleTimeout"

/**
 * Maximum size of the GET line in a HTTP header, in bytes.  0 indicates that
 * there is no maximum size.  The default is 0.
 */
#define PicselConfigBr_httpMaxReqGetSize         "Picsel_httpReqMaxGetSize"

/**
 * Maximum allowed size of a HTTP request body, in bytes.
 *
 * The request body size is calculated in the same way that the
 * Content-Length header in the request is calculated.
 *
 * 0 indicates that there is no maximum size.  The default is 0.
 */
#define PicselConfigBr_httpMaxReqBodySize        "Picsel_httpReqMaxBodySize"

/**
 * Determine whether a fileselect widget's value may be changed via
 * its editbox.
 * 1 indicates that text may be entered via a fileselect's editbox.
 * 0 indicates that the fileselect's editbox is disabled.
 * The default value is 1.
 */
#define PicselConfig_EnableFileselectEditbox     "Picsel_EnableFileselectEditbox"

/**
 * Determine whether to notify the alien when a file select has been reset. This
 * done via an AlienInformation_FileSelectReset event.
 * 1 indicates that notification should be done.
 * 0 indicates that notification should be suppressed (default).
 */
#define PicselConfig_FileSelectNotifyReset       "Picsel_FileSelectNotifyReset"

/**
 * The maximum number of bytes that may be uploaded
 * when an HTML form is submitted.
 * 0 indicates unlimited upload.  The default value is 0.
 */
#define PicselConfig_MaxUploadSize               "Picsel_MaxUploadSize"

/**
 * This property relates to loading pages that are already in the cache
 * but do not have a Last-Modified header.
 *
 * If this property is set to 1, then in this situation the Date header
 * (if present) will be used for If-Modified-Since. Otherwise the current
 * device date will be used.
 *
 * If this property is set to 0 (the default), then no If-Modified-Since
 * header will be sent for this type of page.
 */
#define PicselConfig_UseDeviceDateForLastModified \
                                        "Picsel_UseDeviceDateForLastModified"

/**
 * If this property is set to 1, then URLs with a query will be cached.
 *
 * If this property is set to 0 (the default), then they will not be cached.
 */
#define PicselConfig_CacheQueryUrls             "Picsel_CacheQueryUrls"

/**
 * If this property is set to 1 (the default), redirected URLs will be cached.
 *
 * If this property is set to 0 , then they will not be cached.
 */
#define PicselConfig_CacheRedirections           "Picsel_CacheRedirections"

/**
 * When using a custom compatibility mode (@see PicselBrowser_setUserAgent()),
 * this property sets the value returned by the javascript property
 * "navigator.appName".
 *
 * When not using a custom compatibility mode, this property is ignored.
 *
 * Format  : Freeform string
 * E.g.    : "Picsel ePAGE Browser", "Netscape", "Microsoft Internet Explorer"
 *
 * Default : "Picsel ePAGE Browser"
 */
#define PicselConfigBr_navigatorAppName       "NavigatorAppName"

/**
 * Overrides the language string reported by Picsel to Javascript in the
 * navigator.language and navigator.userLanguage fields.
 *
 * Format  : string conforming to RFC 4646 and RFC 4647, without the script
 * specifier. The string is case sensitive. It is recommended by the RFCs
 * that the language code is in lower case, and the country code is in upper case.
 *
 * E.g.    : "ja-JP"
 *
 * Default : The string set by PicselLocale_set, with the language code in
 * lowercase, and the country code in uppercase.
 */
#define PicselConfigBr_navigatorLanguage       "NavigatorLanguage"

/**
 * This property sets the value returned by the javascript property
 * "navigator.platform".
 *
 * Format  : Freeform string
 * E.g.    : "Win32", "WinCE", "Linux", "Symbian", "Other"
 *
 * Default : One of the above values as appropriate.
 */
#define PicselConfigBr_navigatorPlatform      "NavigatorPlatform"

/**
 * This property sets the value returned by the javascript property
 * "navigator.appVersion".
 *
 * Format  : Freeform string
 * E.g.    : "5.0 (Linux)"
 *
 * Default : The substring from PicselBrowser_setUserAgent
 *           from after the first '/', to after the first ')'.
 *           The '/' is not included, the ')' is.
 *
 *           For example if the userAgent string is
 *           'Picsel/1.0 (Linux;2.6.23) (TrollTech Greenphone)', then the
 *           default is '1.0 (Linux;2.6.23)'
 */
#define PicselConfigBr_navigatorAppVersion      "NavigatorAppVersion"

/**
 * Possible values for HTTP Accept preferences.
 */
typedef enum PicselConfigBr_HttpAccept_Preferences
{
    PicselConfigBr_HttpAccept_PreferHTML  = 0,     /**< Prefer HTML over all
                                                    *   other content. */
    PicselConfigBr_HttpAccept_PreferXHTML = 0x10001/**< Prefer XHTML over HTML
                                                    */
}
PicselConfigBr_HttpAccept_Preferences;

/**
 * This property configures how the browser advertises its content
 * preferences in the "Accept:" HTTP header.  Certain websites (such
 * as facebook) use this information to decide what to send to the
 * client.
 *
 * By default, the browser prefers HTML over other content types such
 * as XHTML or WAP.  When PreferXHTML is set, the browser tells the
 * server it prefers XHTML over HTML.
 *
 * NOTE: Picsel's support for XHTML is currently incomplete, so this option
 * should not be used in production environments.
 *
 * Format  : One of the PicselConfigBr_HttpAccept_Preferences values.
 *
 * Default value is PicselConfigBr_HttpAccept_PreferHTML.
 */
#define PicselConfigBr_httpAcceptPreferences  "Picsel_HttpAccept_Preferences"

/**
 * This property sets the MIME types that the browser will ask your alien
 * application how it wants handled, using PicselUserRequest_DownloadConfirm
 * messages to AlienUserRequest_request().
 *
 * The value for this string should be a series of space-separated MIME types
 * identifying the types that you want your application to be given the option
 * of handling itself when they are opened (either locally or by being
 * downloaded), e.g. "video/x-ms-wmv video/x-ms-wvx video/x-ms-asf"
 * You may register MIME types that Picsel normally handles internally, e.g.
 * "image/gif", although doing so is neither suggested nor encouraged.
 *
 * The default behaviour is not to query your application for any MIME types,
 * and to handle them internally in the Picsel library.
 */
#define PicselConfigBr_downloadableMimeTypes  "DownloadableMimeTypes"

/**
 * If this property is set to 1 (the default), HTML object tags will be
 * displayed when they contain one of the MIME types set in the
 * PicselConfigBr_downloadableMimeTypes property.
 *
 * If this property is set to 0, these object tags will not be displayed
 *
 */
#define PicselConfigBr_supportObjectTagForDownloadableMimeTypes \
    "SupportObjectTagForDownloadableMimeTypes"

/**
 * If this property is set to 1, the first embedded item on the page which
 * matches one of the registered downloadable MIME types will be
 * automatically downloaded.
 *
 */
#define PicselConfigBr_autoDownloadFirstEmbeddedItemOnPage \
    "AutoDownloadFirstEmbeddedItemOnPage"


/**
 * If this property is set to 1, favicons will be downloaded for web pages.
 */
#define PicselConfigBr_enableFavicons "EnableFavicons"

/**
 * Allows the javascript blur method to be disabled. Some websites use the
 * blur method to remove keyboard focus, this can cause the focus to be lost
 * from a hyperlink.
 *
 * If this property is set to 1 (default), the blur method will be actioned.
 * If this property is set to 0, any uses of the blur method will be ignored,
 * note that this could mean some websites might not behave as expected.
 */
#define PicselConfigBr_enableBlur "Picsel_enableBlur"

/**
 * If this property is set to 1 then the server is requested to keep the HTTP
 * connection alive.  There is no guarantee the server will adhere to this
 * request.  The default is 0 - no Keep Alive request is sent.
 */
#define PicselConfigBr_httpSendKeepAlive "Picsel_HttpSendKeepAlive"

/**
 * @}
 */

/**
 * @addtogroup TgvConfigureCore
 * &nbsp;
 *
 * See @ref TgvBrowserConfig.
 */

#endif /* !PICSEL_CONFIG_BROWSER_H */
