/*
 * Copyright (c) 2000-2001, Index Data.
 * See the file LICENSE for details.
 * 
 * $Log: yaz-z-server-sr.cpp,v $
 * Revision 1.2  2001-04-04 14:02:49  adam
 * URSULA / Z-ruth service.
 *
 * Revision 1.1  2001/03/27 15:02:14  adam
 * New server facility scheme.
 *
 */

#include <yaz/log.h>
#include <yaz++/yaz-z-server.h>

Z_Records *Yaz_Facility_Retrieval::pack_records (Yaz_Z_Server *s,
						 const char *resultSetName,
						 int start, int xnum,
						 Z_RecordComposition *comp,
						 int *next, int *pres,
						 int *format)
{
    int recno, total_length = 0, toget = xnum, dumped_records = 0;
    Z_Records *records =
	(Z_Records *) odr_malloc (odr_encode(), sizeof(*records));
    Z_NamePlusRecordList *reclist =
	(Z_NamePlusRecordList *) odr_malloc (odr_encode(), sizeof(*reclist));
    Z_NamePlusRecord **list =
	(Z_NamePlusRecord **) odr_malloc (odr_encode(), sizeof(*list) * toget);

    records->which = Z_Records_DBOSD;
    records->u.databaseOrSurDiagnostics = reclist;
    reclist->num_records = 0;
    reclist->records = list;
    *pres = Z_PRES_SUCCESS;
    *next = 0;

    yaz_log(LOG_LOG, "Request to pack %d+%d", start, toget);
    yaz_log(LOG_LOG, "pms=%d, mrs=%d", m_preferredMessageSize,
	    m_maximumRecordSize);
    for (recno = start; reclist->num_records < toget; recno++)
    {
	Z_NamePlusRecord *this_rec =
	    (Z_NamePlusRecord *) odr_malloc (odr_encode(), sizeof(*this_rec));
	this_rec->databaseName = 0;
	this_rec->which = Z_NamePlusRecord_databaseRecord;
	this_rec->u.databaseRecord = 0;

	int this_length = 0;

	sr_record (resultSetName, recno, format, comp, this_rec, records);

	if (records->which != Z_Records_DBOSD)
	{
	    *pres = Z_PRES_FAILURE;
	    return records;
	}

	if (this_rec->which == Z_NamePlusRecord_databaseRecord &&
	    this_rec->u.databaseRecord == 0)
	{   // handler did not return a record..
	    create_surrogateDiagnostics(odr_encode(), this_rec, 0, 14, 0);
	}
	/*
	 * we get the number of bytes allocated on the stream before any
	 * allocation done by the backend - this should give us a reasonable
	 * idea of the total size of the data so far.
	 */
	total_length = odr_total(odr_encode()) - dumped_records;
	this_length = odr_total(odr_encode()) - total_length;
	yaz_log(LOG_LOG, "  fetched record, len=%d, total=%d",
		this_length, total_length);
	if (this_length + total_length > m_preferredMessageSize)
	{
	    /* record is small enough, really */
	    if (this_length <= m_preferredMessageSize)
	    {
	    	yaz_log(LOG_LOG, "  Dropped last normal-sized record");
		*pres = Z_PRES_PARTIAL_2;
		break;
	    }
	    if (this_length >= m_maximumRecordSize)
	    {   /* too big entirely */
	    	yaz_log(LOG_LOG, "Record > maxrcdsz");
		reclist->records[reclist->num_records] = this_rec;
		create_surrogateDiagnostics(odr_encode(), this_rec,
					    this_rec->databaseName, 17, 0);
		reclist->num_records++;
		*next = recno + 1;
		dumped_records += this_length;
		continue;
	    }
	    else /* record can only be fetched by itself */
	    {
	    	yaz_log(LOG_LOG, "  Record > prefmsgsz");
	    	if (toget > 1)
		{
		    yaz_log(LOG_DEBUG, "  Dropped it");
		    reclist->records[reclist->num_records] = this_rec;
		    create_surrogateDiagnostics(odr_encode(), this_rec,
						this_rec->databaseName,
						16, 0);
		    reclist->num_records++;
		    // *next = freq.last_in_set ? 0 : recno + 1;
		    *next = recno + 1;
		    dumped_records += this_length;
		    continue;
		}
	    }
	}
	reclist->records[reclist->num_records] = this_rec;
	reclist->num_records++;
	*next = recno + 1;
    }
    return records;
}

void Yaz_Facility_Retrieval::fetch_via_piggyback (Yaz_Z_Server *s,
						  Z_SearchRequest *req,
						  Z_SearchResponse *res)
{
    bool_t *sr = (bool_t *)odr_malloc (odr_encode(), sizeof(*sr));
    *sr = 1;
    
    int toget = 0;
    
    Z_RecordComposition comp, *compp = 0;
    int hits = *res->resultCount;
    
    int *nulint = (int *)odr_malloc (odr_encode(), sizeof(*nulint));
    *nulint = 0;
    
    comp.which = Z_RecordComp_simple;
    /* how many records does the user agent want, then? */
    if (hits <= *req->smallSetUpperBound)
    {
	toget = hits;
	if ((comp.u.simple = req->smallSetElementSetNames))
	    compp = &comp;
    }
    else if (hits < *req->largeSetLowerBound)
    {
	toget = *req->mediumSetPresentNumber;
	if (toget > hits)
	    toget = hits;
	if ((comp.u.simple = req->mediumSetElementSetNames))
	    compp = &comp;
    }
    
    if (toget && !res->records)
    {
	res->presentStatus = (int *) odr_malloc (odr_encode(), sizeof(int));
	*res->presentStatus = Z_PRES_SUCCESS;
	res->records =
	    pack_records(s, req->resultSetName, 1, toget, compp, 
			 res->nextResultSetPosition,
			 res->presentStatus,
			 req->preferredRecordSyntax);
	if (!res->records)
	    return;
	if (res->records->which == Z_Records_DBOSD)
	    *res->numberOfRecordsReturned =
		res->records->u.databaseOrSurDiagnostics->num_records;
	res->searchStatus = sr;
	res->resultSetStatus = 0;
    }
    else
    {
	if (hits)
	    *res->nextResultSetPosition = 1;
	res->numberOfRecordsReturned = nulint;
	res->searchStatus = sr;
	res->resultSetStatus = 0;
	res->presentStatus = 0;
    }
}

void Yaz_Facility_Retrieval::fetch_via_present (Yaz_Z_Server *s,
						Z_PresentRequest *req,
						Z_PresentResponse *res)
{
    res->records =
	pack_records (s, req->resultSetId,*req->resultSetStartPoint, 
		      *req->numberOfRecordsRequested,
		      req->recordComposition,
		      res->nextResultSetPosition,
		      res->presentStatus,
		      req->preferredRecordSyntax);
    if (res->records->which == Z_Records_DBOSD)
	*res->numberOfRecordsReturned =
	    res->records->u.databaseOrSurDiagnostics->num_records;
}

int Yaz_Facility_Retrieval::init(Yaz_Z_Server *s, Z_InitRequest *initRequest,
				 Z_InitResponse *initResponse)
{
    Z_Options *req = initRequest->options;
    Z_Options *res = initResponse->options;
    
    if (ODR_MASK_GET(req, Z_Options_search))
	ODR_MASK_SET(res, Z_Options_search);
    if (ODR_MASK_GET(req, Z_Options_present))
	ODR_MASK_SET(res, Z_Options_present);
    m_preferredMessageSize = *initRequest->preferredMessageSize;
    m_maximumRecordSize = *initRequest->maximumRecordSize;
    return sr_init (initRequest, initResponse);
}

ODR Yaz_Facility_Retrieval::odr_encode()
{
    return m_odr_encode;
}

ODR Yaz_Facility_Retrieval::odr_decode()
{
    return m_odr_decode;
}

int Yaz_Facility_Retrieval::recv(Yaz_Z_Server *s, Z_APDU *apdu_request)
{   
    Z_APDU *apdu_response;
    m_odr_encode = s->odr_encode();
    m_odr_decode = s->odr_decode();
    switch (apdu_request->which)
    {
    case Z_APDU_searchRequest:
        yaz_log (LOG_LOG, "got SearchRequest p=%p", this);
	apdu_response = s->create_Z_PDU(Z_APDU_searchResponse);
	sr_search (apdu_request->u.searchRequest,
		       apdu_response->u.searchResponse);
	if (!apdu_response->u.searchResponse->records)
	{
	    fetch_via_piggyback(s, apdu_request->u.searchRequest,
				apdu_response->u.searchResponse);
	}
	s->send_Z_PDU(apdu_response);
	return 1;
    case Z_APDU_presentRequest:
        yaz_log (LOG_LOG, "got PresentRequest p=%p", this);
	apdu_response = s->create_Z_PDU(Z_APDU_presentResponse);
	sr_present (apdu_request->u.presentRequest,
		    apdu_response->u.presentResponse);
	if (!apdu_response->u.presentResponse->records)
	    fetch_via_present(s, apdu_request->u.presentRequest,
			      apdu_response->u.presentResponse);
	s->send_Z_PDU(apdu_response);
	return 1;
    }
    return 0;
}