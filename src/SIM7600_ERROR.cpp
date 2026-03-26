#include "SIM7600_ERROR.h"


// 🔹 Universal error handler
const char *getSim7600ErrorString(SIM7600ErrorCategory category, int errCode)
{
    switch (category)
    {
    // 🌐 MQTT errors
    case SIM7600_ERR_MQTT:
        switch (errCode)
        {
        case 0:  return "Operation succeeded";
            case 1:  return "Failed";
            case 2:  return "Bad UTF-8 string";
            case 3:  return "Socket connect fail";
            case 4:  return "Socket create fail";
            case 5:  return "Socket close fail";
            case 6:  return "Message receive fail";
            case 7:  return "Network open fail";
            case 8:  return "Network close fail";
            case 9:  return "Network not opened";
            case 10: return "Client index error";
            case 11: return "No connection";
            case 12: return "Invalid parameter";
            case 13: return "Not supported operation";
            case 14: return "Client is busy";
            case 15: return "Require connection fail";
            case 16: return "Socket sending fail";
            case 17: return "Timeout";
            case 18: return "Topic is empty";
            case 19: return "Client is already in use";
            case 20: return "Client not acquired";
            case 21: return "Client not released";
            case 22: return "Length out of range";
            case 23: return "Network is opened";
            case 24: return "Packet fail";
            case 25: return "DNS error";
            case 26: return "Socket is closed by server";
            case 27: return "Connection refused: Unaccepted protocol version";
            case 28: return "Connection refused: Identifier rejected";
            case 29: return "Connection refused: Server unavailable";
            case 30: return "Connection refused: Bad user name or password";
            case 31: return "Connection refused: Not authorized";
            case 32: return "Handshake fail";
            case 33: return "Certificate not set";
            default: return "Unknown or unlisted MQTT error";
        }

    // 🌎 HTTP errors (example codes from SIM7600 manual)
    case SIM7600_ERR_HTTP:
    switch (errCode)
    {
    // ------------------------------
    // 🌎 Standard HTTP Response Codes
    // ------------------------------
    case 100: return "Continue";
    case 101: return "Switching Protocols";
    case 102: return "Processing";
    case 200: return "OK (Request succeeded)";
    case 201: return "Created";
    case 202: return "Accepted";
    case 203: return "Non-Authoritative Information";
    case 204: return "No Content";
    case 205: return "Reset Content";
    case 206: return "Partial Content";
    case 207: return "Multi-Status";
    case 208: return "Already Reported";
    case 226: return "IM Used";

    case 300: return "Multiple Choices";
    case 301: return "Moved Permanently";
    case 302: return "Found (Redirect)";
    case 303: return "See Other";
    case 304: return "Not Modified";
    case 305: return "Use Proxy";
    case 307: return "Temporary Redirect";
    case 308: return "Permanent Redirect";

    case 400: return "Bad Request";
    case 401: return "Unauthorized";
    case 402: return "Payment Required";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 406: return "Not Acceptable";
    case 407: return "Proxy Authentication Required";
    case 408: return "Request Timeout";
    case 409: return "Conflict";
    case 410: return "Gone";
    case 411: return "Length Required";
    case 412: return "Precondition Failed";
    case 413: return "Payload Too Large";
    case 414: return "URI Too Long";
    case 415: return "Unsupported Media Type";
    case 416: return "Range Not Satisfiable";
    case 417: return "Expectation Failed";
    case 418: return "I'm a teapot ☕";
    case 421: return "Misdirected Request";
    case 422: return "Unprocessable Entity";
    case 423: return "Locked";
    case 424: return "Failed Dependency";
    case 425: return "Too Early";
    case 426: return "Upgrade Required";
    case 428: return "Precondition Required";
    case 429: return "Too Many Requests";
    case 431: return "Request Header Fields Too Large";
    case 451: return "Unavailable For Legal Reasons";

    case 500: return "Internal Server Error";
    case 501: return "Not Implemented";
    case 502: return "Bad Gateway";
    case 503: return "Service Unavailable";
    case 504: return "Gateway Timeout";
    case 505: return "HTTP Version Not Supported";
    case 506: return "Variant Also Negotiates";
    case 507: return "Insufficient Storage";
    case 508: return "Loop Detected";
    case 510: return "Not Extended";
    case 511: return "Network Authentication Required";

    // ------------------------------
    //  SIM7600 Internal HTTP Errors
    // ------------------------------
    case 0:   return "Success";
    case 701: return "Alert state";
    case 702: return "Unknown error";
    case 703: return "Busy";
    case 704: return "Connection closed error";
    case 705: return "Timeout";
    case 706: return "Receive/send socket data failed";
    case 707: return "File not exists or memory error";
    case 708: return "Invalid parameter";
    case 709: return "Network error";
    case 710: return "Failed to start new SSL session";
    case 711: return "Wrong state";
    case 712: return "Failed to create socket";
    case 713: return "Get DNS failed";
    case 714: return "Connect socket failed";
    case 715: return "Handshake failed";
    case 716: return "Close socket failed";
    case 717: return "No network error";
    case 718: return "Send data timeout";
    case 719: return "CA (certificate) missed";

    // ------------------------------
    default:  return "Unknown HTTP or server response code";
    }


    // 🌐 TCP/IP errors (AT+CIPOPEN / AT+CIPRXGET)
    case SIM7600_ERR_TCP:
        switch (errCode)
        {
        case 0:  return "TCP connected successfully";
        case 1:  return "TCP connection failed";
        case 2:  return "TCP send failed";
        case 3:  return "TCP closed";
        default: return "Unknown TCP error";
        }

    // 🔐 SSL errors
    case SIM7600_ERR_SSL:
        switch (errCode)
        {
        case 0:  return "SSL OK";
        case 1:  return "SSL handshake failed";
        case 2:  return "SSL certificate error";
        case 3:  return "SSL timeout";
        default: return "Unknown SSL error";
        }

    // ⚙️ General or fallback errors
    case SIM7600_ERR_GENERAL:
    default:
        switch (errCode)
        {
        case 0:  return "OK";
        case 1:  return "Generic failure";
        case 2:  return "Invalid command";
        case 3:  return "Timeout or no response";
        default: return "Unknown SIM7600 error";
        }
    }
}