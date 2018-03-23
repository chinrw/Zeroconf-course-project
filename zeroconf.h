//
// Created by Ruowen Qin on 22/03/2018.
//
#pragma once


#include <dns_sd.h>
#include <stdio.h>            // For stdout, stderr
#include <string.h>            // For strlen(), strcpy(), bzero()
#include <errno.h>          // For errno, EINTR
#include <time.h>
#include <sys/time.h>        // For struct timeval
#include <unistd.h>         // For getopt() and optind
#include <arpa/inet.h>        // For inet_addr()

int stopNow = 0;
DNSServiceErrorType err;

void HandleEvents(DNSServiceRef serviceRef) {
    int dns_sd_fd = DNSServiceRefSockFD(serviceRef);
    int nfds = dns_sd_fd + 1;
    fd_set readfds;
    struct timeval tv;
    // . . .
    while (!stopNow) {
        // . . .
        int result = select(nfds, &readfds, (fd_set *) NULL, (fd_set *) NULL, &tv);
        if (result > 0) {
            // . . .
            if (FD_ISSET(dns_sd_fd, &readfds))
                err = DNSServiceProcessResult(serviceRef);
            if (err) stopNow = 1;
        }
    }
}

static void
MyRegisterCallBack(DNSServiceRef service,
                   DNSServiceFlags flags,
                   DNSServiceErrorType errorCode,
                   const char *name,
                   const char *type,
                   const char *domain,
                   void *context) {
#pragma unused(flags)
#pragma unused(context)

    if (errorCode != kDNSServiceErr_NoError)
        fprintf(stderr, "MyRegisterCallBack returned %d\n", errorCode);
    else
        printf("%-15s %s.%s%s\n", "REGISTER", name, type, domain);
}

DNSServiceErrorType MyDNSServiceRegister(int ipaddr) {
    DNSServiceErrorType error;
    DNSServiceRef serviceRef;

    error = DNSServiceRegister(&serviceRef,
                               0,                  // no flags
                               0,                  // all network interfaces
                               "qinr",  // name
                               "_rps._tcp",       // service type
                               "local",             // register in default domain(s)
                               NULL,               // use default host name
                               htons(ipaddr),        // port number
                               0,                  // length of TXT record
                               NULL,               // no TXT record
                               MyRegisterCallBack, // call back function
                               NULL);              // no context

    if (error == kDNSServiceErr_NoError) {
        HandleEvents(serviceRef);
        DNSServiceRefDeallocate(serviceRef);
    }

    return error;
}


