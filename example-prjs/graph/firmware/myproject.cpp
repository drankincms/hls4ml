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
#include "nnet_sublayer.h"
#include "nnet_conv.h"
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

void myproject(
	       input_t    X[N_NODES][N_FEATURES],
	       input_t    Ri[N_NODES][N_EDGES],
	       input_t    Ro[N_NODES][N_EDGES],
	       result_t   res[N_EDGES],
	       unsigned short &const_size_in,
	       unsigned short &const_size_out)
{
  
  //hls-fpga-machine-learning insert IO
#pragma HLS ARRAY_RESHAPE variable=X complete dim=0 
#pragma HLS ARRAY_RESHAPE variable=Ri complete dim=0 
#pragma HLS ARRAY_RESHAPE variable=Ro complete dim=0 
#pragma HLS ARRAY_RESHAPE variable=res complete dim=0 
#pragma HLS INTERFACE ap_vld port=X,Ri,Ro,res
#pragma HLS PIPELINE 
  
  
  const_size_in   = N_NODES*N_FEATURES;
  const_size_out  = N_EDGES;
  
  // ****************************************
  // NETWORK INSTANTIATION
  // ****************************************
  
  //hls-fpga-machine-learning insert layers

  input_t B[N_EDGES][2*N_FEATURES];
#pragma HLS ARRAY_PARTITION variable=B complete dim=0
  nnet::compute_node_features<input_t, input_t, edge_net_config1>(X, Ri, Ro, B);
  
  
}
