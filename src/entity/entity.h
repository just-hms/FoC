#include <string>

namespace entity {
    typedef int Error;

    #define ERR_OK                  Error(+0)
    #define ERR_NOT_FOUND           Error(-1)
    #define ERR_TIMEOUT             Error(-2)
    #define ERR_BROKEN              Error(-3)
    #define ERR_MESSAGE_TOO_LONG    Error(-5)


    struct Response{
        Error err;
        std::string content;
    };
}