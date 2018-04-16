#ifndef __DNS_Lib_H__
#define __DNS_Lib_H__

#include "DNSHeader.h"
#include "DNSQuestion.h"
#include "DNSRecord.h"

struct DNSReq
{
    struct DNSHeader header;
    struct DNSQuestion question;
    struct DNSRecord record;

};

struct DNSReq serialize(){
    struct DNSReq toSend;
    toSend.header.AA = 0;
    toSend.header.RD = 0;
    toSend.header.RA = 0;
    toSend.header.Z = 0;
    toSend.header.NSCOUNT = 0;
    toSend.header.ARCOUNT = 0;
    toSend.question.QTYPE = 1;
    toSend.question.QCLASS = 1;
    toSend.record.TTL = 0;
    return toSend;
}

#endif