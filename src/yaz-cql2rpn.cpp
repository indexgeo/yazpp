/*
 * Copyright (c) 1998-2004, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-cql2rpn.cpp,v 1.4 2004-03-29 22:46:51 adam Exp $
 */

#include <yaz/log.h>
#include <yaz/pquery.h>
#include <yaz++/cql2rpn.h>

Yaz_cql2rpn::Yaz_cql2rpn()
{
    m_transform = 0;
}

Yaz_cql2rpn::~Yaz_cql2rpn()
{
    if (m_transform)
	cql_transform_close(m_transform);
}

void Yaz_cql2rpn::set_pqf_file(const char *fname)
{
    if (!m_transform)
	m_transform = cql_transform_open_fname(fname);
}

int Yaz_cql2rpn::query_transform(const char *cql_query, 
				 Z_RPNQuery **rpnquery, ODR o,
				 char **addinfop)
{
    const char *addinfo = 0;
    if (!m_transform)
	return -3;
    CQL_parser cp = cql_parser_create();

    int r = cql_parser_string(cp, cql_query);
    if (r)
    {
	yaz_log(LOG_LOG, "CQL Parse Error");
	r = 10;
    }
    else
    {
	char rpn_buf[1024];
	r = cql_transform_buf(m_transform, cql_parser_result(cp), 
			      rpn_buf, sizeof(rpn_buf)-1);
	if (!r)
	{
	    YAZ_PQF_Parser pp = yaz_pqf_create();

	    *rpnquery = yaz_pqf_parse(pp, o, rpn_buf);
	    if (!*rpnquery)
	    {
		size_t off;
		const char *pqf_msg;
		int code = yaz_pqf_error(pp, &pqf_msg, &off);
		yaz_log(LOG_WARN, "PQF Parser Error %s (code %d)",
			pqf_msg, code);
		r = -1;
	    }
	    yaz_pqf_destroy(pp);
	}
	else
	{
	    r = cql_transform_error(m_transform, &addinfo);
	    yaz_log(LOG_LOG, "CQL Transform Error %d %s", r,
		    addinfo ? addinfo : "");
	}
    }	
    cql_parser_destroy(cp);
    if (addinfo)
	*addinfop = odr_strdup(o, addinfo);
    else
	*addinfop = 0;
    return r;
}
