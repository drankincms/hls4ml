#ifndef GALAPAGOS_KERNS_H_
#define GALAPAGOS_KERNS_H_

#include "parameters.h"
#include "defines.h"
#include "packet.h"

void kern_send(galapagos_stream * in, galapagos_stream * out);
void kern_recv(galapagos_stream * in, galapagos_stream * out);
void kern_nn(galapagos_stream * in, galapagos_stream * out);

#endif
