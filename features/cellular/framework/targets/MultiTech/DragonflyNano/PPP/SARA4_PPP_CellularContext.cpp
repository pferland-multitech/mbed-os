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

bool SARA4_PPP_CellularContext::get_context()
{
    bool modem_supports_ipv6 = get_property(PROPERTY_IPV6_PDP_TYPE);
    bool modem_supports_ipv4 = get_property(PROPERTY_IPV4_PDP_TYPE);
    _at.cmd_start("AT+CGDCONT?");
    _at.cmd_stop();
    _at.resp_start("+CGDCONT:");
    _cid = -1;
    int cid_max = 0; // needed when creating new context
    char apn[MAX_ACCESSPOINT_NAME_LENGTH];
    int apn_len = 0;

    while (_at.info_resp()) {
        int cid = _at.read_int();
        if (cid > cid_max) {
            cid_max = cid;
        }
        char pdp_type_from_context[10];
        int pdp_type_len = _at.read_string(pdp_type_from_context, sizeof(pdp_type_from_context));
        if (pdp_type_len > 0) {
            apn_len = _at.read_string(apn, sizeof(apn));
            if (apn_len >= 0) {
                if (_apn && (strcmp(apn, _apn) != 0)) {
                    continue;
                }
                // PDP type must match exactly
                if(strcmp(pdp_type_from_context, "IPV4V6") != 0){
                    continue;
                }

                // APN matched -> Check PDP type
                pdp_type_t pdp_type = string_to_pdp_type(pdp_type_from_context);

                // Accept exact matching PDP context type
                if (get_property(pdp_type_t_to_cellular_property(pdp_type))) {
                    _pdp_type = pdp_type;
                    _cid = cid;
                }
            }
        }
    }

    _at.resp_stop();
    if (_cid == -1) { // no suitable context was found so create a new one
        if (!set_new_context(cid_max + 1)) {
            return false;
        }
    }

    // save the apn
    if (apn_len > 0 && !_apn) {
        memcpy(_found_apn, apn, apn_len + 1);
    }

    tr_info("Found PDP context %d", _cid);

    return true;
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