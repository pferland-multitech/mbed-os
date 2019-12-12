#include "SARA4_PPP.h"
#include "SARA4_PPP_CellularContext.h"
#include "CellularLog.h"
#include "CellularUtil.h"

using namespace mbed;
using namespace events;

SARA4_PPP_CellularContext::SARA4_PPP_CellularContext(ATHandler &at, CellularDevice *device, const char *apn, bool cp_req, bool nonip_req) : AT_CellularContext(at, device, apn, cp_req, nonip_req)
{
    
}

SARA4_PPP_CellularContext::~SARA4_PPP_CellularContext()
{

}

bool SARA4_PPP_CellularContext::set_new_context(int cid)
{
    bool modem_supports_ipv6 = get_property(PROPERTY_IPV6_PDP_TYPE);
    bool modem_supports_ipv4 = get_property(PROPERTY_IPV4_PDP_TYPE);
    bool modem_supports_nonip = get_property(PROPERTY_NON_IP_PDP_TYPE);

    char pdp_type_str[8 + 1] = {0};
    pdp_type_t pdp_type = IPV4_PDP_TYPE;

    // if (_nonip_req && _cp_in_use && modem_supports_nonip) {
    //     strncpy(pdp_type_str, "Non-IP", sizeof(pdp_type_str));
    //     pdp_type = NON_IP_PDP_TYPE;
    // } else if (modem_supports_ipv6 && modem_supports_ipv4) {
        strncpy(pdp_type_str, "IPV4V6", sizeof(pdp_type_str));
        pdp_type = IPV4V6_PDP_TYPE;
    // } else if (modem_supports_ipv6) {
    //     strncpy(pdp_type_str, "IPV6", sizeof(pdp_type_str));
    //     pdp_type = IPV6_PDP_TYPE;
    // } else if (modem_supports_ipv4) {
    //     strncpy(pdp_type_str, "IP", sizeof(pdp_type_str));
    //     pdp_type = IPV4_PDP_TYPE;
    // } else {
    //     return false;
    // }

    //apn: "If the value is null or omitted, then the subscription value will be requested."
    bool success = false;
    _at.cmd_start("AT+CGDCONT=");
    _at.write_int(cid);
    _at.write_string(pdp_type_str);
    _at.write_string(_apn);
    _at.cmd_stop_read_resp();
    success = (_at.get_last_error() == NSAPI_ERROR_OK);

    if (success) {
        _pdp_type = pdp_type;
        _cid = cid;
        _new_context_set = true;
        tr_info("New PDP context %d, type %d", _cid, pdp_type);
    }

    return success;
}