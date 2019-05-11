#ifndef _IFC_ACTIONS_H
#define _IFC_ACTIONS_H

#ifdef __cplusplus 
extern "C" {
#endif

int ifc_enable_actions(const char * ifname);
int ifc_disable_actions(const char * ifname);
int ifc_enable_addr_actions(const char * ifname,const char * addr);
void ifc_clear_addr_actions(const char *ifname);
int ifc_get_mac_addr(const char *ifname, unsigned char *mac);
int ifc_set_ip_addr(const char *name, const char *ip);
int ifc_get_ip_addr(const char *name, char *ip);
int ifc_set_netmask_addr(const char *ifname, const char *ip);
int ifc_get_netmask_addr(const char *ifname, char *ip);
int ifc_set_route_addr(const char *ifname, const char *ip);
int ifc_get_route_addr(const char *ifname, char *ip);



#ifdef __cplusplus 
}
#endif

#endif
