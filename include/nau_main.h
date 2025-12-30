#ifndef __NAU_MAIN_H__
#define	__NAU_MAIN_H__


//////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cstring>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "nau_item.h"
#include "nau_json.h"
#include "nau_arp.h"
#include "nau_db.h"
#include "nau_http.h"
#include "nau_iface.h"


//////////////////////////////////////////////////////////////////////////
#define	NAU_TYPE_AP		0
#define	NAU_TYPE_GW		1


//////////////////////////////////////////////////////////////////////////
void
nau_main_ap(void);
void
nau_main_gw(void);


void *
nau_ap_looparp(
	void			*arg);
void
nau_ap_update(void);
void
nau_ap_upload(
	const char		*fname);

void
nau_gw_update(void);
void
nau_gw_upload(
	const char		*fname);


#endif

