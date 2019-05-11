#ifndef AM_NET_SNMP_H
#define AM_NET_SNMP_H

#define AM_SNMPV3_MIN_SZ    (32)
#define AM_SNMPV3_MIDDLE_SZ (128)
#define AM_SNMPV3_MAX_SZ    (256)

typedef enum{
    AM_SNMPV3_RT_OK = 0,
    AM_SNMPV3_RT_ERR = 1
}am_snmpv3_ret_err_t;

typedef enum{
    AM_SNMPV3_HASH_MD5 = 0, /*default: WEB GUI only show up MD5 option*/
    AM_SNMPV3_HASH_SHA      /*in the future*/
}am_snmpv3_hash_type_t;

typedef enum{
    AM_SNMPV3_ENCRY_DES = 0, /*default: WEB GUI only show up DES option*/
    AM_SNMPV3_ENCRY_AES
}am_snmpv3_encry_type_t;

typedef struct am_snmpv3_hash_conf_s{
    am_snmpv3_hash_type_t type; /*default: MD5*/
    char code[AM_SNMPV3_MIN_SZ];
}am_snmpv3_hash_conf_t;

typedef struct am_snmpv3_encry_conf_s{
    am_snmpv3_encry_type_t type; /*default: DES*/
    char code[AM_SNMPV3_MIN_SZ];
}am_snmpv3_encry_conf_t;

#endif /* AM_NET_SNMP_H */
