#ifndef __NAU_HTTP_H__
#define	__NAU_HTTP_H__


//////////////////////////////////////////////////////////////////////////
int
nau_http_post(const char *addr, int port, const char *url, const char *fname);

void
nau_http_svr(const char *addr, int port, const char *url, const char *fpath, int	type);


#endif

