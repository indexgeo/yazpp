/*
 * Copyright (c) 1998-2005, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: z-assoc.h,v 1.10 2005-06-25 15:53:19 adam Exp $
 */

#ifndef YAZ_Z_ASSOC_INCLUDED
#define YAZ_Z_ASSOC_INCLUDED

#include <yaz/srw.h>
#include <yaz/proto.h>
#include <yaz/odr.h>
#include <yaz++/pdu-observer.h>

namespace yazpp_1 {
/** Z39.50 Assocation.
    This object implements the client - and server role of a generic
    Z39.50 Association.
*/
class YAZ_EXPORT Z_Assoc : public IPDU_Observer {
 public:
    /// Create object using the PDU Observer specified
    Z_Assoc(IPDU_Observable *the_PDU_Observable);
    /// Destroy assocation and close PDU Observer
    virtual ~Z_Assoc();
    /// Receive PDU
    void recv_PDU(const char *buf, int len);
    /// Connect notification
    virtual void connectNotify() = 0;
    /// Failure notification
    virtual void failNotify() = 0;
    /// Timeout notification
    virtual void timeoutNotify() = 0;
    /// Timeout specify
    void timeout(int timeout);
    /// Begin Z39.50 client role
    int client(const char *addr);
    /// Begin Z39.50 server role
    int server(const char *addr);
    /// Close connection
    void close();
    /// Decode Z39.50 PDU.
    Z_GDU *decode_GDU(const char *buf, int len);
    /// Encode Z39.50 PDU.
    int encode_GDU(Z_GDU *apdu, char **buf, int *len);
    /// Send Z39.50 PDU
    int send_Z_PDU(Z_APDU *apdu, int *len);
    int send_GDU(Z_GDU *apdu, int *len);
    /// Receive Z39.50 PDU
    virtual void recv_GDU(Z_GDU *apdu, int len) = 0;
    /// Create Z39.50 PDU with reasonable defaults
    Z_APDU *create_Z_PDU(int type);
    /// Request Alloc
    ODR odr_encode ();
    ODR odr_decode ();
    ODR odr_print ();

    void set_APDU_log(const char *fname);
    const char *get_APDU_log();

    /// OtherInformation
    void get_otherInfoAPDU(Z_APDU *apdu, Z_OtherInformation ***oip);
    Z_OtherInformationUnit *update_otherInformation (
        Z_OtherInformation **otherInformationP, int createFlag,
        int *oid, int categoryValue, int deleteFlag);
    void set_otherInformationString (
        Z_OtherInformation **otherInformationP,
        int *oid, int categoryValue,
        const char *str);
    void set_otherInformationString (
        Z_OtherInformation **otherInformation,
        int oidval, int categoryValue,
        const char *str);
    void set_otherInformationString (
        Z_APDU *apdu,
        int oidval, int categoryValue,
        const char *str);

    Z_ReferenceId *getRefID(char* str);
    Z_ReferenceId **get_referenceIdP(Z_APDU *apdu);
    void transfer_referenceId(Z_APDU *from, Z_APDU *to);

    const char *get_hostname();

    int set_APDU_yazlog(int v);

 private:
    static int yaz_init_flag;
    static int yaz_init_func();
    IPDU_Observable *m_PDU_Observable;
    ODR m_odr_in;
    ODR m_odr_out;
    ODR m_odr_print;
    int m_log;
    FILE *m_APDU_file;
    char *m_APDU_fname;
    char *m_hostname;
    int m_APDU_yazlog;
};
};

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

