// $Header: /home/cvsroot/yaz++/zoom/zquery.cpp,v 1.6 2003-09-22 13:01:54 mike Exp $

// Z39.50 Query classes

#include "zoom.h"


namespace ZOOM {
    query::query() : q(ZOOM_query_create()) {
    }
    query::~query() {
	ZOOM_query_destroy(q);
    }

    prefixQuery::prefixQuery(const std::string &pqn) {
	if (ZOOM_query_prefix(q, pqn.c_str()) == -1) {
	    throw queryException(queryException::PREFIX, pqn);
	}
    }

    // The binding specification says we have to have destructors for
    // the query subclasses, so in they go -- even though they don't
    // actually _do_ anything that inheriting the base query type's
    // destructor wouldn't do.  It's an irritant of C++ that the
    // declaration of a subclass has to express explicitly the
    // implementation detail of whether destruction is implemented
    // by a specific destructor or by inheritance.  Oh well.
    //
    // ### Not sure whether I need to do nothing at all, and the
    // superclass destructor gets called anyway (I think that only
    // works when you _don't_ define a destructor so that the default
    // one pertains) or whether I need to duplicate the functionality
    // of that destructor.  Let's play safe by assuming the latter and
    // zeroing what we free so that we get bitten if we're wrong.
    //
    prefixQuery::~prefixQuery() {
    }



    CCLQuery::CCLQuery(const std::string &, void *) {
	throw "Oops.  No CCL support in ZOOM-C yet.  Sorry.";
    }

    CCLQuery::~CCLQuery() {
    }
}
