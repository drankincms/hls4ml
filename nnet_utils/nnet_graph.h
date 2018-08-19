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

#ifndef NNET_GRAPH_H_
#define NNET_GRAPH_H_

#include "nnet_common.h"
#include "nnet_layer.h"
#include "nnet_conv.h"
#include "hls_stream.h"
#include <math.h>

namespace nnet {
  
  struct edge_net_config
  {
    // Internal data type definitions
    typedef float bias_t;
    typedef float weight_t;
    typedef float accum_t;
    
    // Layer Sizes
    static const unsigned n_node = 4;
    static const unsigned n_edge = 4;
    static const unsigned n_input_dim = 7;
    static const unsigned n_hidden_dim = 4;
    
    // Resource reuse info
    static const unsigned io_type = io_parallel;
    static const unsigned reuse_factor = 1;
    static const bool store_weights_in_bram = false;
    static const unsigned n_zeros = 0;
  };
  
  template<class data_T, class res_T, typename CONFIG_T>
    void compute_node_features(
		   data_T    X[CONFIG_T::n_node][CONFIG_T::n_input_dim],
		   data_T    Ri[CONFIG_T::n_node][CONFIG_T::n_edge],
		   data_T    Ro[CONFIG_T::n_node][CONFIG_T::n_edge],
		   res_T     B[CONFIG_T::n_edge][2*CONFIG_T::n_input_dim])
  {
    data_T bo[CONFIG_T::n_edge][CONFIG_T::n_input_dim];
    data_T bi[CONFIG_T::n_edge][CONFIG_T::n_input_dim];
    
    if (CONFIG_T::io_type == io_parallel){
      // For parallel inputs:
      //   - completely partition arrays -- target fabric
      //   - if we have an unroll factor, limit number of multipliers
#pragma HLS PIPELINE II=CONFIG_T::reuse_factor
      
      // #pragma HLS ARRAY_PARTITION variable=weights complete // remove this line for now, it breaks compression sometimes
#pragma HLS ARRAY_PARTITION variable=bo complete
#pragma HLS ARRAY_PARTITION variable=bi complete
      //int multiplier_limit  = ceil(float(CONFIG_T::n_in*CONFIG_T::n_out) / float(CONFIG_T::reuse_factor)) - floor(float(CONFIG_T::n_zeros) / float(CONFIG_T::reuse_factor));
      //#pragma HLS ALLOCATION instances=mul limit=multiplier_limit operation
      
    } else if (CONFIG_T::io_type == io_serial){
#pragma HLS DATAFLOW
#pragma HLS STREAM variable=bo depth=1
#pragma HLS STREAM variable=bi depth=1
    }

    // Do the matrix-multiply
    for(int ii = 0; ii < CONFIG_T::n_edge; ii++) {
      if (CONFIG_T::io_type == io_serial){
#pragma HLS PIPELINE
      }
      for(int jj = 0; jj < CONFIG_T::n_input_dim; jj++) {
	//if (CONFIG_T::io_type == io_serial) {
	//int multiplier_limit  = ceil(float(CONFIG_T::n_out) / float(CONFIG_T::reuse_factor));
	//#pragma HLS ALLOCATION instances=mul limit=multiplier_limit operation
	//      }
	bi[ii][jj] = 0;
	bo[ii][jj] = 0;
	for(int kk = 0; kk < CONFIG_T::n_node; kk++) {
	  bi[ii][jj] += Ri[kk][ii]* X[kk][jj];
	  bo[ii][jj] += Ro[kk][ii]* X[kk][jj];
	}
	B[ii][jj] = (res_T) bo[ii][jj];
	B[ii][CONFIG_T::n_input_dim+jj] = (res_T) bi[ii][jj];
      }
    }
  }
}

#endif
