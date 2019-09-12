#include <cstddef>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef CPU
#include <chrono>
#endif

#define NUM_INTERMEDIATE_HOPS 2

#include "galapagos_kerns.h"
#include "myproject.h"

#define MAX_FLITS_TRANS 170

void kern_send(short id, galapagos_stream * in, galapagos_stream  * out)
{
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS INTERFACE axis register both port=out
#pragma HLS INTERFACE axis register both port=in

    int num_flits = STREAMSIZE*N_INPUTS;
    
    galapagos_stream_packet gp;
    gp.id = id;

    #include "inputs.h"

    std::cout <<"******************************" << std::endl << "Input:" << std::endl << std::endl;

#ifdef CPU
    auto start = std::chrono::high_resolution_clock::now();
#endif

    for(int j=0; j<num_flits; j++){
        gp.dest = 1; // FIRST
        gp.data = 0;
        (gp.data)(IN_BITS-1,0) = input_vals[j];
        if(j==(num_flits -1))
            gp.last = 1;
        else
            gp.last = 0;
        std::cout <<"int[" << std::dec << j << "]: " << std::hex << gp.data << std::endl;
        out->write(gp);
    }
#ifdef CPU
    auto send = std::chrono::high_resolution_clock::now();
#endif


    std::cout << std::endl << std::endl << std::endl;
    std::cout <<"****************************** WRITTEN ALL DATA TO NETWORK, WAITING FOR FPGA TO PROCESS AND RETURN ****************" << std::endl;
    std::cout << std::endl << std::endl << std::endl;

    num_flits = STREAMSIZE*N_OUTPUTS;

    std::cout <<"******************************" << std::endl << "Output:" << std::endl << std::endl;
    //num_flits = STREAMSIZE*N_INPUTS;
    for(int j=0; j<num_flits; j++){
        gp = in->read();
        std::cout <<"out[" << std::dec << j << "]: " << std::hex << gp.data << std::endl;

     }
#ifdef CPU
    auto recv = std::chrono::high_resolution_clock::now();
#endif 
#ifdef CPU
    std::cout << std::endl << std::endl;
    std::cout << "Send/prep time:  " << ((std::chrono::duration<double>)(send - start)).count() << " s" << std::endl;
    std::cout << "   HLS4ML time:  " << ((std::chrono::duration<double>)(recv - send)).count() << " s" << std::endl;
#endif

}

//FINAL RECV RUNNING IN SOFTWARE
void kern_recv(short id, galapagos_stream * in, galapagos_stream  * out)
{
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS INTERFACE axis register both port=out
#pragma HLS INTERFACE axis register both port=in
    
    galapagos_stream_packet gp;
    short dest;
    
    dest = gp.dest + 1;
    gp.id = id;

    ap_uint <1> last = 0;
    
    int num_flits = STREAMSIZE*N_OUTPUTS;
    for(int j=0; j<num_flits; j++){
    //while(!last){
        gp = in->read();
        std::cout << "DATA IS " << gp.data << std::endl << std::flush;
        last = gp.last;
        out->write(gp);

    }
}

void kern_nn(
        const ap_uint<8> id,
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

    //reading: while(!last){
    reading: for (int i = 0; i < STREAMSIZE; i++) {
        #pragma HLS PIPELINE
        for (int j = 0; j < N_INPUTS; j++) {
            gp = in->read();
            //last = gp.last;
            in_buf[i][j](IN_BITS-1,0) = (gp.data)(IN_BITS-1,0);
        }
    
        hls4ml: myproject(in_buf[i],out_buf[i]);
    }

    short dest = gp.id;

    int curr_index=0;
    writing: for (int i = 0; i < STREAMSIZE; i++) {
        for (int j = 0; j < N_OUTPUTS; j++) {
            gp.data = 0;
            (gp.data)(OUT_BITS-1,0) = out_buf[i][j](OUT_BITS-1,0);
            gp.dest = dest;
            gp.last = (i==STREAMSIZE-1 && j==N_OUTPUTS-1 ? 1 : 0);
            gp.id = 1;
            out->write(gp);
        }
    }
}
