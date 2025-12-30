//////////////////////////////////////////////////////////////////////////
#include "nau_main.h"


//////////////////////////////////////////////////////////////////////////
int
nau_iface_main(
	const char		*ifname)
{
    char			buf[8192];
    struct sockaddr_nl sa;
    struct iovec	iov = {buf, sizeof(buf)};
    struct msghdr	msg = {(void *)&sa, sizeof(sa), &iov, 1, NULL, 0, 0};
	struct nlmsghdr	*nh;
	unsigned int	ifindex;
	int				nl_sock;
	ssize_t			rc;

    // Step1: 创建Socket
    nl_sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if(nl_sock < 0) {

		perror("socket");
        return -1;
    }
	memset(&sa, 0, sizeof(sa));
	sa.nl_family = AF_NETLINK;
	sa.nl_groups = RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR;

	// Step2: 绑定Socket
	if(bind(nl_sock, (struct sockaddr *)&sa, sizeof(sa)) < 0) {

		perror("bind");
		close(nl_sock);
		return -2;
	}

    // Step3: 监听内核消息
	ifindex = if_nametoindex(ifname);
    while( true ) {

        rc = recvmsg(nl_sock, &msg, 0);
        if(rc < 0) {

			if(errno == EINTR || errno == EAGAIN) {

				continue;
			}
            perror("recvmsg");
            break;
        }
		if(rc == 0) {
			printf("Recv EOF\n");
			break;
		}

        // 4. 解析接收到的缓冲区数据
        for(nh = (struct nlmsghdr *)buf; NLMSG_OK(nh, rc); nh = NLMSG_NEXT(nh, rc)) {

			// 只处理对应接口的地址添加 & 删除消息
			if(nh->nlmsg_type != RTM_NEWADDR && nh->nlmsg_type != RTM_DELADDR) {
				continue;
			}
			struct ifaddrmsg *ifaddr = (struct ifaddrmsg *)NLMSG_DATA(nh);
			if(ifindex != ifaddr->ifa_index) {

				continue;
			}
			// 处理地址变化事件
			nau_ap_update();
        }
    }

    close(nl_sock);
    return 0;
}
int
nau_iface_addr(
	std::string&	addr,
	int				family,
	const char		*ifname)
{
	char			strip[128];
	struct ifaddrs	*ifaddr, *ifa;

	// Step1: 获取所有接口
	if(getifaddrs(&ifaddr) == -1) {

		perror("getifaddrs failed");
		return -1;
	}

	// Step2: 遍历所有接口
	for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {

		if(ifa->ifa_addr == NULL) {

			continue;
		}

		// 是否对应网卡
		if(	strcmp(ifa->ifa_name, ifname) != 0 ||
			family != ifa->ifa_addr->sa_family) {

			continue;
		}
		if(AF_INET == family) {

			const struct sockaddr_in *s4 = (const struct sockaddr_in *)(ifa->ifa_addr);
			inet_ntop(family, &(s4->sin_addr), strip, INET_ADDRSTRLEN);
		}else {

			const struct sockaddr_in6 *s6 = (const struct sockaddr_in6 *)(ifa->ifa_addr);
			inet_ntop(family, &(s6->sin6_addr), strip, INET6_ADDRSTRLEN);
			if(strcmp("fe80::1", strip) == 0 || strcmp("::1", strip) == 0) { // 去掉链路本地地址

				continue;
			}
		}
		addr = strip;
		break;
	}
	freeifaddrs(ifaddr);
	return 0;
}

std::string
nau_iface_4to6(
	std::string&	prefix,
	std::string&	address)
{
	return prefix + address;
}
std::string
nau_iface_6to4(
	std::string&	prefix,
	std::string&	address)
{
	struct in6_addr	ipv6;
	char			ipv4[INET_ADDRSTRLEN];

	// Step1: 转为二进制
	if(inet_pton(AF_INET6, address.c_str(), &ipv6) != 1) {

		std::cerr << "IPv6 Address Format Error!" << std::endl;
		return "";
	}
	// Step2: 取最后32位作为IPv4地址
	const unsigned char *bytes = ipv6.s6_addr;
	snprintf(ipv4, sizeof(ipv4), "%d.%d.%d.%d", bytes[12], bytes[13], bytes[14], bytes[15]);

	return ipv4;
}

