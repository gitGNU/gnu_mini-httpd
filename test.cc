/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <stdexcept>
#include "HTTPParser.hh"
#include "log.hh"
using namespace std;

int main(int, char** argv)
try
    {
    // Test resetable_variable.

    resetable_variable<int> number;
    assert(number.empty());

    number = 42;
    assert(!number.empty());
    assert(number == 42);
    assert(number > 41);
    assert(number < 43);

    number = 0;
    assert(!number.empty());

    number = resetable_variable<int>();
    assert(number.empty());

    // Now test it again with the URL structure.

    URL url;
    assert(url.host.empty());

    url.host = "foobar.example.org";
    assert(!url.host.empty());

    url.host = "foobar.example.org";
    assert(!url.host.empty());
    assert(static_cast<std::string>(url.host) == string("foobar.example.org"));
    assert(strcmp(url.host.data().c_str(), "foobar.example.org") == 0);

    url = URL();
    assert(url.host.empty());

    // Init the HTTP parser.

    HTTPParser parser;

    // Exit gracefully.

    return 0;
    }
catch(const exception& e)
    {
    error("Caught exception: %s", e.what());
    return 1;
    }
catch(...)
    {
    error("Caught unknown exception.");
    return 1;
    }
