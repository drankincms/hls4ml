#ifndef PARAMETERS_H_
#define PARAMETERS_H_

#include <complex>
#include "ap_int.h"
#include "ap_fixed.h"
#include "nnet_layer.h"
#include "nnet_conv.h"
#include "nnet_conv2d.h"
#include "nnet_activation.h"
#include "nnet_common.h"
#include "nnet_batchnorm.h"

//hls-fpga-machine-learning insert numbers
typedef ap_fixed<16,6> accum_default_t;
typedef ap_fixed<16,6> weight_default_t;
typedef ap_fixed<16,6> bias_default_t;
typedef ap_fixed<16,6> input_t;
typedef ap_fixed<16,6> result_t;
#define Y_INPUTS_1 10
#define N_CHAN_1 4
#define Y_OUTPUTS_1 10
#define N_FILT_1 3
#define Y_INPUTS_2 10
#define N_CHAN_2 3
#define Y_OUTPUTS_2 10
#define N_FILT_2 2
#define Y_INPUTS_3 10
#define N_CHAN_3 2
#define Y_OUTPUTS_3 10
#define N_FILT_3 1
#define N_LAYER_4 5
#define N_OUTPUTS 5

//hls-fpga-machine-learning insert layer-precision
typedef ap_fixed<16,6> layer1_t;
typedef ap_fixed<16,6> layer2_t;
typedef ap_fixed<16,6> layer3_t;
typedef ap_fixed<16,6> layer4_t;

//hls-fpga-machine-learning insert layer-config
struct config1 : nnet::conv_config {
        static const unsigned pad_left = 1;
        static const unsigned pad_right = 2;
        static const unsigned y_in = Y_INPUTS_1;
        static const unsigned n_chan = N_CHAN_1;
        static const unsigned y_filt = 4;
        static const unsigned n_filt = N_FILT_1;
        static const unsigned stride = 1;
        static const unsigned y_out = Y_OUTPUTS_1;
        static const unsigned reuse_factor = 4;
        static const unsigned n_zeros = 0;
        static const bool store_weights_in_bram = false;
        typedef accum_default_t accum_t;
        typedef bias_default_t bias_t;
        typedef weight_default_t weight_t;
        };
struct relu_config1 : nnet::activ_config {
        static const unsigned n_in = Y_OUTPUTS_1*N_FILT_1;
        static const unsigned table_size = 1024;
        static const unsigned io_type = nnet::io_parallel;
        };
struct config2 : nnet::conv_config {
        static const unsigned pad_left = 1;
        static const unsigned pad_right = 2;
        static const unsigned y_in = Y_INPUTS_2;
        static const unsigned n_chan = N_CHAN_2;
        static const unsigned y_filt = 4;
        static const unsigned n_filt = N_FILT_2;
        static const unsigned stride = 1;
        static const unsigned y_out = Y_OUTPUTS_2;
        static const unsigned reuse_factor = 4;
        static const unsigned n_zeros = 0;
        static const bool store_weights_in_bram = false;
        typedef accum_default_t accum_t;
        typedef bias_default_t bias_t;
        typedef weight_default_t weight_t;
        };
struct relu_config2 : nnet::activ_config {
        static const unsigned n_in = Y_OUTPUTS_2*N_FILT_2;
        static const unsigned table_size = 1024;
        static const unsigned io_type = nnet::io_parallel;
        };
struct config3 : nnet::conv_config {
        static const unsigned pad_left = 1;
        static const unsigned pad_right = 2;
        static const unsigned y_in = Y_INPUTS_3;
        static const unsigned n_chan = N_CHAN_3;
        static const unsigned y_filt = 4;
        static const unsigned n_filt = N_FILT_3;
        static const unsigned stride = 1;
        static const unsigned y_out = Y_OUTPUTS_3;
        static const unsigned reuse_factor = 4;
        static const unsigned n_zeros = 0;
        static const bool store_weights_in_bram = false;
        typedef accum_default_t accum_t;
        typedef bias_default_t bias_t;
        typedef weight_default_t weight_t;
        };
struct relu_config3 : nnet::activ_config {
        static const unsigned n_in = Y_OUTPUTS_3*N_FILT_3;
        static const unsigned table_size = 1024;
        static const unsigned io_type = nnet::io_parallel;
        };
struct config4 : nnet::layer_config {
        static const unsigned n_in = Y_OUTPUTS_3*N_FILT_3;
        static const unsigned n_out = N_LAYER_4;
        static const unsigned io_type = nnet::io_parallel;
        static const unsigned reuse_factor = 4;
        static const unsigned n_zeros = 0;
        static const bool store_weights_in_bram = false;
        typedef accum_default_t accum_t;
        typedef bias_default_t bias_t;
        typedef weight_default_t weight_t;
        };
struct relu_config4 : nnet::activ_config {
        static const unsigned n_in = N_LAYER_4;
        static const unsigned table_size = 1024;
        static const unsigned io_type = nnet::io_parallel;
        };
struct config5 : nnet::layer_config {
        static const unsigned n_in = N_LAYER_4;
        static const unsigned n_out = N_OUTPUTS;
        static const unsigned io_type = nnet::io_parallel;
        static const unsigned reuse_factor = 4;
        static const unsigned n_zeros = 0;
        static const bool store_weights_in_bram = false;
        typedef accum_default_t accum_t;
        typedef bias_default_t bias_t;
        typedef weight_default_t weight_t;
        };
struct softmax_config5 : nnet::activ_config {
        static const unsigned n_in = N_OUTPUTS;
        static const unsigned table_size = 1024;
        static const unsigned io_type = nnet::io_parallel;
        };

#endif 
