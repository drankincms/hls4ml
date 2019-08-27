#ifndef PACKET_H_
#define PACKET_H_
#include "ap_int.h"
#include "hls_stream.h"

typedef struct {
    ap_uint <PACKET_DATA_LENGTH> data;
    ap_uint <PACKET_DEST_LENGTH> dest;
#ifdef PACKET_LAST  
    ap_uint <1> last;
#endif   
#ifdef PACKET_ID_LENGTH  
    ap_uint <PACKET_ID_LENGTH> id;
#endif   
#ifdef PACKET_USER_LENGTH  
    ap_uint <PACKET_USER_LENGTH> user;
#endif   
#ifdef PACKET_KEEP_LENGTH  
    ap_uint <PACKET_KEEP_LENGTH> keep;
#endif   
}galapagos_stream_packet;

typedef hls::stream<galapagos_stream_packet> galapagos_stream;
#endif
