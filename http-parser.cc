/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <iostream>
#include <stdexcept>
#include "uri.hh"
using namespace std;
using namespace spirit;

inline std::ostream& operator<< (std::ostream& os, const URI& uri)
    {
    os << "<uri>\n"
       << "    <full>       " << uri.full        << "\n"
       << "    <scheme>     " << uri.scheme      << "\n"
       << "    <userinfo>   " << uri.userinfo    << "\n"
       << "    <host>       " << uri.host        << "\n"
       << "    <opaque_part>" << uri.opaque_part << "\n"
       << "    <ipaddress>  " << uri.ipaddress   << "\n"
       << "    <port>       " << uri.port        << "\n"
       << "    <path>       " << uri.path        << "\n"
       << "    <query>      " << uri.query       << "\n"
       << "</uri>" << endl;
    return os;
    }

int main(int argc, char** argv)
try
    {
    if (argc < 2)
        {
        cerr << "Command line syntax error." << endl;
        return 1;
        }

    for (int i = 1; i < argc; ++i)
        {
        cout << "Parsing '" << argv[i] << "' ... " << endl;
        URI uri;
        uri_parser parser(uri);
        parser(argv[i]);
        cout << uri;
        }

    return 0;
    }
catch(const exception& e)
    {
    cerr << "Caught exception: " << e.what() << endl;
    return 1;
    }
catch(...)
    {
    cerr << "Caught unknown exception; aborting." << endl;
    return 1;
    }
