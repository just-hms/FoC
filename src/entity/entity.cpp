#include "entity.h"

// StatusCodeFromCSocketErrorCodes given the c style error returns an entity::Error
entity::Error entity::StatusCodeFromCSocketErrorCodes(int code){
    
    if (code == 0)  return entity::ERR_TIMEOUT;
    if (code < 0)   return entity::ERR_BROKEN;
    
    return entity::ERR_OK;
}