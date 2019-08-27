#include <cstddef>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_INTERMEDIATE_HOPS 2

#include "galapagos_kerns.h"
#include "myproject.h"

void kern_send(galapagos_stream * in, galapagos_stream  * out)
{
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS INTERFACE axis register both port=out
#pragma HLS INTERFACE axis register both port=in

    int num_flits = STREAMSIZE*N_INPUTS;
    
    galapagos_stream_packet gp;

    #include "inputs.h"

    for(int j=0; j<num_flits; j++){
        gp.dest = 1; // FIRST
        gp.data = 0;
        (gp.data)(input_vals[j].length()-1,0) = input_vals[j];
        if(j==(num_flits -1))
            gp.last = 1;
        else
            gp.last = 0;
        out->write(gp);
    }
}

//FINAL RECV RUNNING IN SOFTWARE
void kern_recv(galapagos_stream * in, galapagos_stream  * out)
{
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS INTERFACE axis register both port=out
#pragma HLS INTERFACE axis register both port=in
    
    galapagos_stream_packet gp;
    short dest;
    
    dest = gp.dest + 1;

    ap_uint <1> last = 0;
    
    int num_flits = STREAMSIZE*N_INPUTS;
    for(int j=0; j<num_flits; j++){
    //while(!last){
        gp = in->read();
        std::cout << "DATA IS " << gp.data << std::endl << std::flush;
        last = gp.last;
        out->write(gp);

    }
}

void kern_nn(
        galapagos_stream *in, // Read-Only Vector
        galapagos_stream *out       // Output Result
        )
{
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS INTERFACE axis register both port=out
#pragma HLS INTERFACE axis register both port=in
    
    galapagos_stream_packet gp;

    ap_uint <1> last = 0;

    input_t in_buf[STREAMSIZE][N_INPUTS];
    result_t out_buf[STREAMSIZE][N_OUTPUTS];
    #pragma HLS ARRAY_PARTITION variable=in_buf complete dim=2
    #pragma HLS ARRAY_PARTITION variable=out_buf complete dim=2 
    #pragma HLS ARRAY_PARTITION variable=in_buf cyclic factor=GAL_FAC dim=1
    #pragma HLS ARRAY_PARTITION variable=out_buf cyclic factor=GAL_FAC dim=1 

    //reading: while(!last){
    reading: for (int i = 0; i < STREAMSIZE; i++) {
        #pragma HLS PIPELINE
        for (int j = 0; j < N_INPUTS; j++) {
            gp = in->read();
            //last = gp.last;
            in_buf[i][j](in_buf[i][j].length()-1,0) = (gp.data)(in_buf[i][j].length()-1,0);
        }
    
        hls4ml: myproject(in_buf[i],out_buf[i]);
    }

    short dest = gp.dest + 1;

    writing: for (int i = 0; i < STREAMSIZE; i++) {
        for (int j = 0; j < N_OUTPUTS; j++) {
            gp.data = 0;
            (gp.data)(out_buf[i][j].length()-1,0) = out_buf[i][j](out_buf[i][j].length()-1,0);
            gp.dest = dest;
            gp.last = (i==STREAMSIZE-1 && j==N_OUTPUTS-1 ? 1 : 0);
            out->write(gp);
        }
    }
}
