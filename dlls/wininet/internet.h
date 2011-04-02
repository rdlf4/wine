/*
 * Wininet
 *
 * Copyright 1999 Corel Corporation
 *
 * Ulrich Czekalla
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef _WINE_INTERNET_H_
#define _WINE_INTERNET_H_

#ifndef __WINE_CONFIG_H
# error You must include config.h to use this header
#endif

#include "wine/unicode.h"
#include "wine/list.h"

#include <time.h>
#ifdef HAVE_NETDB_H
# include <netdb.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <sys/types.h>
# include <netinet/in.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

#if !defined(__MINGW32__) && !defined(_MSC_VER)
#define closesocket close
#define ioctlsocket ioctl
#endif /* __MINGW32__ */

extern HMODULE WININET_hModule;

/* used for netconnection.c stuff */
typedef struct
{
    BOOL useSSL;
    int socketFD;
    void *ssl_s;
    DWORD security_flags;
} WININET_NETCONNECTION;

static inline void * __WINE_ALLOC_SIZE(1) heap_alloc(size_t len)
{
    return HeapAlloc(GetProcessHeap(), 0, len);
}

static inline void * __WINE_ALLOC_SIZE(1) heap_alloc_zero(size_t len)
{
    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, len);
}

static inline void * __WINE_ALLOC_SIZE(2) heap_realloc(void *mem, size_t len)
{
    return HeapReAlloc(GetProcessHeap(), 0, mem, len);
}

static inline void * __WINE_ALLOC_SIZE(2) heap_realloc_zero(void *mem, size_t len)
{
    return HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, mem, len);
}

static inline BOOL heap_free(void *mem)
{
    return HeapFree(GetProcessHeap(), 0, mem);
}

static inline LPWSTR heap_strdupW(LPCWSTR str)
{
    LPWSTR ret = NULL;

    if(str) {
        DWORD size;

        size = (strlenW(str)+1)*sizeof(WCHAR);
        ret = HeapAlloc(GetProcessHeap(), 0, size);
        if(ret)
            memcpy(ret, str, size);
    }

    return ret;
}

static inline LPWSTR heap_strndupW(LPCWSTR str, UINT max_len)
{
    LPWSTR ret;
    UINT len;

    if(!str)
        return NULL;

    for(len=0; len<max_len; len++)
        if(str[len] == '\0')
            break;

    ret = HeapAlloc(GetProcessHeap(), 0, sizeof(WCHAR)*(len+1));
    if(ret) {
        memcpy(ret, str, sizeof(WCHAR)*len);
        ret[len] = '\0';
    }

    return ret;
}

static inline WCHAR *heap_strdupAtoW(const char *str)
{
    LPWSTR ret = NULL;

    if(str) {
        DWORD len;

        len = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
        ret = HeapAlloc(GetProcessHeap(), 0, len*sizeof(WCHAR));
        if(ret)
            MultiByteToWideChar(CP_ACP, 0, str, -1, ret, len);
    }

    return ret;
}

static inline char *heap_strdupWtoA(LPCWSTR str)
{
    char *ret = NULL;

    if(str) {
        DWORD size = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
        ret = HeapAlloc(GetProcessHeap(), 0, size);
        if(ret)
            WideCharToMultiByte(CP_ACP, 0, str, -1, ret, size, NULL, NULL);
    }

    return ret;
}

static inline void WININET_find_data_WtoA(LPWIN32_FIND_DATAW dataW, LPWIN32_FIND_DATAA dataA)
{
    dataA->dwFileAttributes = dataW->dwFileAttributes;
    dataA->ftCreationTime   = dataW->ftCreationTime;
    dataA->ftLastAccessTime = dataW->ftLastAccessTime;
    dataA->ftLastWriteTime  = dataW->ftLastWriteTime;
    dataA->nFileSizeHigh    = dataW->nFileSizeHigh;
    dataA->nFileSizeLow     = dataW->nFileSizeLow;
    dataA->dwReserved0      = dataW->dwReserved0;
    dataA->dwReserved1      = dataW->dwReserved1;
    WideCharToMultiByte(CP_ACP, 0, dataW->cFileName, -1, 
        dataA->cFileName, sizeof(dataA->cFileName),
        NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, dataW->cAlternateFileName, -1, 
        dataA->cAlternateFileName, sizeof(dataA->cAlternateFileName),
        NULL, NULL);
}

typedef enum
{
    WH_HINIT = INTERNET_HANDLE_TYPE_INTERNET,
    WH_HFTPSESSION = INTERNET_HANDLE_TYPE_CONNECT_FTP,
    WH_HGOPHERSESSION = INTERNET_HANDLE_TYPE_CONNECT_GOPHER,
    WH_HHTTPSESSION = INTERNET_HANDLE_TYPE_CONNECT_HTTP,
    WH_HFILE = INTERNET_HANDLE_TYPE_FTP_FILE,
    WH_HFTPFINDNEXT = INTERNET_HANDLE_TYPE_FTP_FIND,
    WH_HHTTPREQ = INTERNET_HANDLE_TYPE_HTTP_REQUEST,
} WH_TYPE;

#define INET_OPENURL 0x0001
#define INET_CALLBACKW 0x0002

typedef struct _object_header_t object_header_t;

typedef struct {
    void (*Destroy)(object_header_t*);
    void (*CloseConnection)(object_header_t*);
    DWORD (*QueryOption)(object_header_t*,DWORD,void*,DWORD*,BOOL);
    DWORD (*SetOption)(object_header_t*,DWORD,void*,DWORD);
    DWORD (*ReadFile)(object_header_t*,void*,DWORD,DWORD*);
    DWORD (*ReadFileExA)(object_header_t*,INTERNET_BUFFERSA*,DWORD,DWORD_PTR);
    DWORD (*ReadFileExW)(object_header_t*,INTERNET_BUFFERSW*,DWORD,DWORD_PTR);
    DWORD (*WriteFile)(object_header_t*,const void*,DWORD,DWORD*);
    DWORD (*QueryDataAvailable)(object_header_t*,DWORD*,DWORD,DWORD_PTR);
    DWORD (*FindNextFileW)(object_header_t*,void*);
} object_vtbl_t;

#define INTERNET_HANDLE_IN_USE 1

struct _object_header_t
{
    WH_TYPE htype;
    const object_vtbl_t *vtbl;
    HINTERNET hInternet;
    BOOL valid_handle;
    DWORD  dwFlags;
    DWORD_PTR dwContext;
    DWORD  dwError;
    ULONG  ErrorMask;
    DWORD  dwInternalFlags;
    LONG   refs;
    INTERNET_STATUS_CALLBACK lpfnStatusCB;
    struct list entry;
    struct list children;
};


typedef struct
{
    object_header_t hdr;
    LPWSTR  agent;
    LPWSTR  proxy;
    LPWSTR  proxyBypass;
    LPWSTR  proxyUsername;
    LPWSTR  proxyPassword;
    DWORD   accessType;
} appinfo_t;


typedef struct
{
    object_header_t hdr;
    appinfo_t *appInfo;
    LPWSTR  hostName; /* the final destination of the request */
    LPWSTR  serverName; /* the name of the server we directly connect to */
    LPWSTR  userName;
    LPWSTR  password;
    INTERNET_PORT hostPort; /* the final destination port of the request */
    INTERNET_PORT serverPort; /* the port of the server we directly connect to */
    struct sockaddr_storage socketAddress;
    socklen_t sa_len;
} http_session_t;

#define HDR_ISREQUEST		0x0001
#define HDR_COMMADELIMITED	0x0002
#define HDR_SEMIDELIMITED	0x0004

typedef struct
{
    LPWSTR lpszField;
    LPWSTR lpszValue;
    WORD wFlags;
    WORD wCount;
} HTTPHEADERW, *LPHTTPHEADERW;


struct HttpAuthInfo;

typedef struct data_stream_vtbl_t data_stream_vtbl_t;

typedef struct {
    const data_stream_vtbl_t *vtbl;
}  data_stream_t;

typedef struct {
    data_stream_t data_stream;
    DWORD content_length;
    DWORD content_read;
} netconn_stream_t;

#define READ_BUFFER_SIZE 8192

typedef struct
{
    object_header_t hdr;
    http_session_t *session;
    LPWSTR path;
    LPWSTR verb;
    LPWSTR rawHeaders;
    WININET_NETCONNECTION netConnection;
    LPWSTR version;
    LPWSTR statusText;
    DWORD bytesToWrite;
    DWORD bytesWritten;
    HTTPHEADERW *custHeaders;
    DWORD nCustHeaders;
    FILETIME last_modified;
    HANDLE hCacheFile;
    LPWSTR cacheFile;
    FILETIME expires;
    struct HttpAuthInfo *authInfo;
    struct HttpAuthInfo *proxyAuthInfo;

    CRITICAL_SECTION read_section;  /* section to protect the following fields */
    DWORD contentLength;  /* total number of bytes to be read */
    BOOL  read_chunked;   /* are we reading in chunked mode? */
    BOOL  read_gzip;      /* are we reading in gzip mode? */
    DWORD read_pos;       /* current read position in read_buf */
    DWORD read_size;      /* valid data size in read_buf */
    BYTE  read_buf[READ_BUFFER_SIZE]; /* buffer for already read but not returned data */

    BOOL decoding;
    data_stream_t *data_stream;
    netconn_stream_t netconn_stream;
} http_request_t;



struct WORKREQ_FTPPUTFILEW
{
    LPWSTR lpszLocalFile;
    LPWSTR lpszNewRemoteFile;
    DWORD  dwFlags;
    DWORD_PTR dwContext;
};

struct WORKREQ_FTPSETCURRENTDIRECTORYW
{
    LPWSTR lpszDirectory;
};

struct WORKREQ_FTPCREATEDIRECTORYW
{
    LPWSTR lpszDirectory;
};

struct WORKREQ_FTPFINDFIRSTFILEW
{
    LPWSTR lpszSearchFile;
    LPWIN32_FIND_DATAW lpFindFileData;
    DWORD  dwFlags;
    DWORD_PTR dwContext;
};

struct WORKREQ_FTPGETCURRENTDIRECTORYW
{
    LPWSTR lpszDirectory;
    DWORD *lpdwDirectory;
};

struct WORKREQ_FTPOPENFILEW
{
    LPWSTR lpszFilename;
    DWORD  dwAccess;
    DWORD  dwFlags;
    DWORD_PTR dwContext;
};

struct WORKREQ_FTPGETFILEW
{
    LPWSTR lpszRemoteFile;
    LPWSTR lpszNewFile;
    BOOL   fFailIfExists;
    DWORD  dwLocalFlagsAttribute;
    DWORD  dwFlags;
    DWORD_PTR dwContext;
};

struct WORKREQ_FTPDELETEFILEW
{
    LPWSTR lpszFilename;
};

struct WORKREQ_FTPREMOVEDIRECTORYW
{
    LPWSTR lpszDirectory;
};

struct WORKREQ_FTPRENAMEFILEW
{
    LPWSTR lpszSrcFile;
    LPWSTR lpszDestFile;
};

struct WORKREQ_FTPFINDNEXTW
{
    LPWIN32_FIND_DATAW lpFindFileData;
};

struct WORKREQ_HTTPSENDREQUESTW
{
    LPWSTR lpszHeader;
    DWORD  dwHeaderLength;
    LPVOID lpOptional;
    DWORD  dwOptionalLength;
    DWORD  dwContentLength;
    BOOL   bEndRequest;
};

struct WORKREQ_HTTPENDREQUESTW
{
    DWORD     dwFlags;
    DWORD_PTR dwContext;
};

struct WORKREQ_SENDCALLBACK
{
    DWORD_PTR dwContext;
    DWORD     dwInternetStatus;
    LPVOID    lpvStatusInfo;
    DWORD     dwStatusInfoLength;
};

struct WORKREQ_INTERNETOPENURLW
{
    HINTERNET hInternet;
    LPWSTR     lpszUrl;
    LPWSTR     lpszHeaders;
    DWORD     dwHeadersLength;
    DWORD     dwFlags;
    DWORD_PTR dwContext;
};

struct WORKREQ_INTERNETREADFILEEXA
{
    LPINTERNET_BUFFERSA lpBuffersOut;
};

struct WORKREQ_INTERNETREADFILEEXW
{
    LPINTERNET_BUFFERSW lpBuffersOut;
};

typedef struct WORKREQ
{
    void (*asyncproc)(struct WORKREQ*);
    object_header_t *hdr;

    union {
        struct WORKREQ_FTPPUTFILEW              FtpPutFileW;
        struct WORKREQ_FTPSETCURRENTDIRECTORYW  FtpSetCurrentDirectoryW;
        struct WORKREQ_FTPCREATEDIRECTORYW      FtpCreateDirectoryW;
        struct WORKREQ_FTPFINDFIRSTFILEW        FtpFindFirstFileW;
        struct WORKREQ_FTPGETCURRENTDIRECTORYW  FtpGetCurrentDirectoryW;
        struct WORKREQ_FTPOPENFILEW             FtpOpenFileW;
        struct WORKREQ_FTPGETFILEW              FtpGetFileW;
        struct WORKREQ_FTPDELETEFILEW           FtpDeleteFileW;
        struct WORKREQ_FTPREMOVEDIRECTORYW      FtpRemoveDirectoryW;
        struct WORKREQ_FTPRENAMEFILEW           FtpRenameFileW;
        struct WORKREQ_FTPFINDNEXTW             FtpFindNextW;
        struct WORKREQ_HTTPSENDREQUESTW         HttpSendRequestW;
        struct WORKREQ_HTTPENDREQUESTW          HttpEndRequestW;
        struct WORKREQ_SENDCALLBACK             SendCallback;
        struct WORKREQ_INTERNETOPENURLW         InternetOpenUrlW;
        struct WORKREQ_INTERNETREADFILEEXA      InternetReadFileExA;
        struct WORKREQ_INTERNETREADFILEEXW      InternetReadFileExW;
    } u;

} WORKREQUEST, *LPWORKREQUEST;

void *alloc_object(object_header_t*,const object_vtbl_t*,size_t);
object_header_t *get_handle_object( HINTERNET hinternet );
object_header_t *WININET_AddRef( object_header_t *info );
BOOL WININET_Release( object_header_t *info );

DWORD INET_QueryOption( object_header_t *, DWORD, void *, DWORD *, BOOL );

time_t ConvertTimeString(LPCWSTR asctime);

HINTERNET FTP_Connect(appinfo_t *hIC, LPCWSTR lpszServerName,
	INTERNET_PORT nServerPort, LPCWSTR lpszUserName,
	LPCWSTR lpszPassword, DWORD dwFlags, DWORD_PTR dwContext,
	DWORD dwInternalFlags);

DWORD HTTP_Connect(appinfo_t*,LPCWSTR,
        INTERNET_PORT nServerPort, LPCWSTR lpszUserName,
        LPCWSTR lpszPassword, DWORD dwFlags, DWORD_PTR dwContext,
        DWORD dwInternalFlags, HINTERNET*);

BOOL GetAddress(LPCWSTR lpszServerName, INTERNET_PORT nServerPort,
	struct sockaddr *psa, socklen_t *sa_len);

void INTERNET_SetLastError(DWORD dwError);
DWORD INTERNET_GetLastError(void);
DWORD INTERNET_AsyncCall(LPWORKREQUEST lpWorkRequest);
LPSTR INTERNET_GetResponseBuffer(void);
LPSTR INTERNET_GetNextLine(INT nSocket, LPDWORD dwLen);

VOID SendAsyncCallback(object_header_t *hdr, DWORD_PTR dwContext,
                       DWORD dwInternetStatus, LPVOID lpvStatusInfo,
                       DWORD dwStatusInfoLength);

VOID INTERNET_SendCallback(object_header_t *hdr, DWORD_PTR dwContext,
                           DWORD dwInternetStatus, LPVOID lpvStatusInfo,
                           DWORD dwStatusInfoLength);
BOOL INTERNET_FindProxyForProtocol(LPCWSTR szProxy, LPCWSTR proto, WCHAR *foundProxy, DWORD *foundProxyLen);

BOOL NETCON_connected(WININET_NETCONNECTION *connection);
DWORD NETCON_init(WININET_NETCONNECTION *connnection, BOOL useSSL);
void NETCON_unload(void);
DWORD NETCON_create(WININET_NETCONNECTION *connection, int domain,
	      int type, int protocol);
DWORD NETCON_close(WININET_NETCONNECTION *connection);
DWORD NETCON_connect(WININET_NETCONNECTION *connection, const struct sockaddr *serv_addr,
		    unsigned int addrlen);
DWORD NETCON_secure_connect(WININET_NETCONNECTION *connection, LPWSTR hostname);
DWORD NETCON_send(WININET_NETCONNECTION *connection, const void *msg, size_t len, int flags,
		int *sent /* out */);
DWORD NETCON_recv(WININET_NETCONNECTION *connection, void *buf, size_t len, int flags,
		int *recvd /* out */);
BOOL NETCON_query_data_available(WININET_NETCONNECTION *connection, DWORD *available);
LPCVOID NETCON_GetCert(WININET_NETCONNECTION *connection);
int NETCON_GetCipherStrength(WININET_NETCONNECTION *connection);
DWORD NETCON_set_timeout(WININET_NETCONNECTION *connection, BOOL send, int value);
int sock_get_error(int);

extern void URLCacheContainers_CreateDefaults(void);
extern void URLCacheContainers_DeleteAll(void);

#define MAX_REPLY_LEN	 	0x5B4

/* Used for debugging - maybe need to be shared in the Wine debugging code ? */
typedef struct
{
    DWORD val;
    const char* name;
} wininet_flag_info;

#endif /* _WINE_INTERNET_H_ */
