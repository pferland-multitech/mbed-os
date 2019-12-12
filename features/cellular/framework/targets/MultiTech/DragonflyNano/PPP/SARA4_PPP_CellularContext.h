#ifndef SARA4_PPP_CELLULAR_CONTEXT_H_
#define SARA4_PPP_CELLULAR_CONTEXT_H_

#include "AT_CellularContext.h"

namespace mbed {

class SARA4_PPP_CellularContext : public AT_CellularContext {
public:
    SARA4_PPP_CellularContext(ATHandler &at, CellularDevice *device, const char *apn = 0, bool cp_req = false, bool nonip_req = false);
    virtual ~SARA4_PPP_CellularContext();

protected:
    virtual bool set_new_context(int cid);
    virtual bool get_context();
};

} // namespace mbed

#endif // SARA4_PPP_CELLULAR_CONTEXT_H_