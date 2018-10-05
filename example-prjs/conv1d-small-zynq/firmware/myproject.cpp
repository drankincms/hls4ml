//
//    rfnoc-hls-neuralnet: Vivado HLS code for neural-net building blocks
//
//    Copyright (C) 2017 EJ Kreinar
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include <iostream>

#include "parameters.h"
#include "myproject.h"

#include "nnet_layer.h"
#include "nnet_conv.h"
#include "nnet_conv2d.h"
#include "nnet_batchnorm.h"
#include "nnet_activation.h"

//hls-fpga-machine-learning insert weights
#include "weights/w1.h"
#include "weights/b1.h"
#include "weights/w2.h"
#include "weights/b2.h"
#include "weights/w3.h"
#include "weights/b3.h"
#include "weights/w4.h"
#include "weights/b4.h"
#include "weights/w5.h"
#include "weights/b5.h"

void hls4ml(
		  input_t data[Y_INPUTS_1][N_CHAN_1],
		  result_t res[N_OUTPUTS],
		  unsigned short &const_size_in,
		  unsigned short &const_size_out)
{

    //hls-fpga-machine-learning insert IO
    #pragma HLS ARRAY_RESHAPE variable=data complete dim=0 
    #pragma HLS ARRAY_RESHAPE variable=res complete dim=0 
    //#pragma HLS INTERFACE ap_vld port=data,res //gives error since function is no longer top
    #pragma HLS PIPELINE 


    const_size_in   = Y_INPUTS_1*N_CHAN_1;
    const_size_out  = N_OUTPUTS;

    // ****************************************
    // NETWORK INSTANTIATION
    // ****************************************

    //hls-fpga-machine-learning insert layers

    layer1_t layer1_out[Y_OUTPUTS_1*N_FILT_1];
    #pragma HLS ARRAY_PARTITION variable=layer1_out complete dim=0
    layer1_t conv_layer1_out[Y_OUTPUTS_1][N_FILT_1];
    #pragma HLS ARRAY_PARTITION variable=conv_layer1_out complete dim=0
    nnet::conv_1d<input_t, layer1_t, config1>(data, conv_layer1_out, w1, b1);
    layer1_t logits1[Y_OUTPUTS_1*N_FILT_1];
    #pragma HLS ARRAY_PARTITION variable=logits1 complete dim=0
    nnet::flatten<input_t, Y_OUTPUTS_1, N_FILT_1>(conv_layer1_out, logits1);
    nnet::relu<layer1_t, layer1_t, relu_config1>(logits1, layer1_out);

    layer2_t layer2_out[Y_OUTPUTS_2*N_FILT_2];
    #pragma HLS ARRAY_PARTITION variable=layer2_out complete dim=0
    layer1_t conv_layer2_in[Y_INPUTS_2][N_CHAN_2];
    #pragma HLS ARRAY_PARTITION variable=conv_layer2_in complete dim=0
    nnet::unflatten<layer1_t, Y_INPUTS_2, N_CHAN_2>(layer1_out, conv_layer2_in);
    layer2_t conv_layer2_out[Y_OUTPUTS_2][N_FILT_2];
    #pragma HLS ARRAY_PARTITION variable=conv_layer2_out complete dim=0
    nnet::conv_1d<layer1_t, layer2_t, config2>(conv_layer2_in, conv_layer2_out, w2, b2);
    layer2_t logits2[Y_OUTPUTS_2*N_FILT_2];
    #pragma HLS ARRAY_PARTITION variable=logits2 complete dim=0
    nnet::flatten<layer1_t, Y_OUTPUTS_2, N_FILT_2>(conv_layer2_out, logits2);
    nnet::relu<layer2_t, layer2_t, relu_config2>(logits2, layer2_out);

    layer3_t layer3_out[Y_OUTPUTS_3*N_FILT_3];
    #pragma HLS ARRAY_PARTITION variable=layer3_out complete dim=0
    layer2_t conv_layer3_in[Y_INPUTS_3][N_CHAN_3];
    #pragma HLS ARRAY_PARTITION variable=conv_layer3_in complete dim=0
    nnet::unflatten<layer2_t, Y_INPUTS_3, N_CHAN_3>(layer2_out, conv_layer3_in);
    layer3_t conv_layer3_out[Y_OUTPUTS_3][N_FILT_3];
    #pragma HLS ARRAY_PARTITION variable=conv_layer3_out complete dim=0
    nnet::conv_1d<layer2_t, layer3_t, config3>(conv_layer3_in, conv_layer3_out, w3, b3);
    layer3_t logits3[Y_OUTPUTS_3*N_FILT_3];
    #pragma HLS ARRAY_PARTITION variable=logits3 complete dim=0
    nnet::flatten<layer2_t, Y_OUTPUTS_3, N_FILT_3>(conv_layer3_out, logits3);
    nnet::relu<layer3_t, layer3_t, relu_config3>(logits3, layer3_out);

    layer4_t layer4_out[N_LAYER_4];
    #pragma HLS ARRAY_PARTITION variable=layer4_out complete dim=0
    layer4_t logits4[N_LAYER_4];
    #pragma HLS ARRAY_PARTITION variable=logits4 complete dim=0
    nnet::compute_layer<layer3_t, layer4_t, config4>(layer3_out, logits4, w4, b4);
    nnet::relu<layer4_t, layer4_t, relu_config4>(logits4, layer4_out);

    result_t logits5[N_OUTPUTS];
    #pragma HLS ARRAY_PARTITION variable=logits5 complete dim=0
    nnet::compute_layer<layer4_t, result_t, config5>(layer4_out, logits5, w5, b5);
    nnet::softmax<result_t, result_t, softmax_config5>(logits5, res);


}

void myproject(
    input_t in_stream[Y_INPUTS_1*N_CHAN_1],
    result_t out_stream[N_OUTPUTS])
{ //inputs are now flat, can be streamed in
    #pragma HLS DATAFLOW
    #pragma HLS INTERFACE s_axilite  port=return     bundle=CTRL_BUS //define interface protocol (set up for axi stream)
    #pragma HLS INTERFACE axis       port=in_stream                  //define interface protocol (set up for axi stream)
    #pragma HLS INTERFACE axis       port=out_stream                 //define interface protocol (set up for axi stream)

    input_t input[Y_INPUTS_1][N_CHAN_1];
    result_t output[N_OUTPUTS];
    unsigned int inid = 0;
    unsigned int outid = 0;
    unsigned short insize = Y_INPUTS_1*N_CHAN_1;
    unsigned short outsize = N_OUTPUTS;

    for (unsigned int i = 0; i < Y_INPUTS_1; i++) {
        for (unsigned int j = 0; j < N_CHAN_1; j++) {
            input[i][j] = in_stream[inid++]; //read in streamed inputs
        }
    }

    hls4ml(input, output, insize, outsize); //run actual hls4ml project (conv1d)

    for (unsigned int i = 0; i < N_OUTPUTS; i++) {
        out_stream[outid++] = output[i]; //write outputs
    }

    return;
}
