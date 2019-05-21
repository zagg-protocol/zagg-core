// Copyright 2014 Stellar Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "main/CommandLine.h"
#include "main/DeprecatedCommandLine.h"
#include "util/Logging.h"

#include "crypto/ByteSliceHasher.h"
#include <cstdlib>
#include <sodium/core.h>
#include <xdrpp/marshal.h>

INITIALIZE_EASYLOGGINGPP

namespace stellar
{
static void
outOfMemory()
{
    std::fprintf(stderr, "Unable to allocate memory\n");
    std::fflush(stderr);
    std::abort();
}
}

int
main(int argc, char* const* argv)
{
    using namespace stellar;
    
    // Abort when out of memory
    std::set_new_handler(outOfMemory);

    Logging::init();
    if (sodium_init() != 0)
    {
        LOG(FATAL) << "Could not initialize crypto";
        return 1;
    }
    shortHash::initialize();

    xdr::marshaling_stack_limit = 1000;
    
    auto result = handleCommandLine(argc, argv);
    if (result)
    {
        return *result;
    }

    return handleDeprecatedCommandLine(argc, argv);

    //  std::cout<< fRet << "= 2 ==============================================================================================================\n";

    // if (!fRet)
    // {
    //     std::cout<< fRet << "= 3 ==============================================================================================================\n";
    //     Interrupt();
    // } else {
    //     std::cout<< fRet << "= 4 ==============================================================================================================\n";
    //     WaitForShutdown();
    //     std::cout<< fRet << "= 5 ==============================================================================================================\n";
    // }
    // Shutdown(interfaces);

}
