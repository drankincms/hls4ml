#ifndef GALAPAGOS_KERNS_H_
#define GALAPAGOS_KERNS_H_

#include "parameters.h"

#ifdef CPU
#include "galapagos_stream.hpp"
#else
#include "galapagos_packet.h"
#endif
//#include "defines.h"
//#include "packet.h"

void kern_send(short id, galapagos_stream * in, galapagos_stream * out);
void kern_recv(short id, galapagos_stream * in, galapagos_stream * out);
void kern_nn(short id, galapagos_stream * in, galapagos_stream * out);

#endif
