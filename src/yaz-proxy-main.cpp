/*
 * Copyright (c) 1998-2001, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-proxy-main.cpp,v 1.19 2003-10-01 13:13:51 adam Exp $
 */

#include <yaz/log.h>
#include <yaz/options.h>

#include <yaz++/socket-manager.h>
#include <yaz++/pdu-assoc.h>
#include <yaz++/proxy.h>

void usage(char *prog)
{
    fprintf (stderr, "%s: [-c config] [-a log] [-m num] [-v level] [-t target] [-i sec] "
             "[-u auth] [-o optlevel] @:port\n", prog);
    exit (1);
}


int args(Yaz_Proxy *proxy, int argc, char **argv)
{
    char *addr = 0;
    char *arg;
    char *prog = argv[0];
    int ret;

    while ((ret = options("o:a:t:v:c:u:i:m:l:T:", argv, argc, &arg)) != -2)
    {
	int err;
        switch (ret)
        {
        case 0:
            if (addr)
	    {
		usage(prog);
		return 1;
	    }
	    addr = arg;
            break;
	case 'c':
	    err = proxy->set_config(arg);
	    if (err == -2)
	    {
		fprintf(stderr, "Config file support not enabled (proxy not compiled with libxml2 support)\n");
		exit(1);
	    }
	    else if (err == -1)
	    {
		fprintf(stderr, "Bad or missing file %s\n", arg);
		exit(1);
	    }
	    break;
	case 'a':
	    proxy->set_APDU_log(arg);
	    break;
        case 't':
	    proxy->set_default_target(arg);
	    break;
        case 'u':
            proxy->set_proxy_authentication(arg);
            break;
        case 'o':
	    proxy->option("optimize", arg);
	    break;
	case 'v':
	    yaz_log_init_level (yaz_log_mask_str(arg));
	    break;
	case 'l':
	    yaz_log_init_file (arg);
	    break;
	case 'm':
	    proxy->set_max_clients(atoi(arg));
	    break;
        case 'i':
	    proxy->set_client_idletime(atoi(arg));
	    break;
        case 'T':
	    proxy->set_target_idletime(atoi(arg));
	    break;
        default:
	    usage(prog);
	    return 1;
        }
    }
    if (addr)
    {
	proxy->server(addr);
    }
    else
    {
	usage(prog);
	return 1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    Yaz_SocketManager mySocketManager;
    Yaz_Proxy proxy(new Yaz_PDU_Assoc(&mySocketManager));

    args(&proxy, argc, argv);
    while (mySocketManager.processEvent() > 0)
	;
    exit (0);
    return 0;
}
