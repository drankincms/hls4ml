from __future__ import print_function
import tarfile
import yaml
from shutil import copyfile
import numpy as np
import random as rand
import os
import re
import glob
from collections import OrderedDict

rand.seed(47)

#######################################
## Print weight array to C++
#######################################
def print_array_to_cpp(var, odir, write_txt_file=True):

    h_file = open("{}/firmware/weights/{}.h".format(odir,var.name),"w")
    if write_txt_file:
        txt_file = open("{}/firmware/weights/{}.txt".format(odir,var.name),"w")

    #meta data
    h_file.write("//Numpy array shape {}\n".format(var.shape))
    h_file.write("//Min {:.12f}\n".format(np.min(var.min)))
    h_file.write("//Max {:.12f}\n".format(np.max(var.max)))
    h_file.write("//Number of zeros {}\n".format(var.nzeros))
    h_file.write("\n")

    h_file.write("#ifndef {}_H_\n".format(var.name.upper()))
    h_file.write("#define {}_H_\n".format(var.name.upper()))
    h_file.write("\n")

    if write_txt_file:
        h_file.write("#ifndef __SYNTHESIS__\n")
        h_file.write(var.definition_cpp() + ";\n")
        h_file.write("#else\n")

    h_file.write(var.definition_cpp() + " = {")

    #fill c++ array.
    #not including internal brackets for multidimensional case
    sep = ''
    for x in var:
        h_file.write(sep + x)
        if write_txt_file:
            txt_file.write(sep + x)
        sep = ", "
    h_file.write("};\n")
    if write_txt_file:
        h_file.write("#endif\n")
        txt_file.close()
    h_file.write("\n#endif\n")
    h_file.close()

def write_project_dir(model):
    if not os.path.isdir("{}/firmware/weights".format(model.config.get_output_dir())):
        os.makedirs("{}/firmware/weights".format(model.config.get_output_dir()))

    if not os.path.isdir("{}/middlewareInput/conf_oneFPGA".format(model.config.get_output_dir())):
        os.makedirs("{}/middlewareInput/conf_oneFPGA".format(model.config.get_output_dir()))

def write_project_cpp(model):
    ###################
    ## myproject.cpp
    ###################

    filedir = os.path.dirname(os.path.abspath(__file__))
    f = open(os.path.join(filedir,'../templates/vivado/firmware/myproject.cpp'),'r')
    fout = open('{}/firmware/{}.cpp'.format(model.config.get_output_dir(), model.config.get_project_name()),'w')

    model_inputs = model.get_input_variables()
    model_outputs = model.get_output_variables()

    indent = '    '

    for line in f.readlines():
        #Add headers to weights and biases
        if 'myproject' in line:
            newline = line.replace('myproject', model.config.get_project_name())
        elif '//hls-fpga-machine-learning insert header' in line:
            inputs_str = ', '.join([i.definition_cpp() for i in model_inputs])
            outputs_str = ', '.join([o.definition_cpp() for o in model_outputs])
            #insize_str = ', '.join(['unsigned short &const_size_in_{}'.format(i) for i in range(1, len(model_inputs) + 1)])
            #outsize_str = ', '.join(['unsigned short &const_size_out_{}'.format(i) for i in range(1, len(model_outputs) + 1)])

            newline = ''
            newline += indent + inputs_str + ',\n'
            newline += indent + outputs_str + '\n'
            #newline += indent + insize_str + ',\n'
            #newline += indent + outsize_str + '\n'

        elif '//hls-fpga-machine-learning insert weights' in line:
            newline = line
            for layer in model.get_layers():
                for w in layer.get_weights():
                    newline += '#include "weights/{}.h"\n'.format(w.name)

        elif '//hls-fpga-machine-learning insert load weights' in line:
            newline = line
            for layer in model.get_layers():
                for w in layer.get_weights():
                    if w.__class__.__name__ == 'CompressedWeightVariable':
                        newline += indent + '    nnet::load_compressed_weights_from_txt<{}, {}>({}, "{}.txt");\n'.format(w.type.name, w.nonzeros, w.name, w.name)
                    else:
                        newline += indent + '    nnet::load_weights_from_txt<{}, {}>({}, "{}.txt");\n'.format(w.type.name, w.data_length, w.name, w.name)

        #Add input/output type
        elif '//hls-fpga-machine-learning insert IO' in line:
            newline = line
            all_inputs = [i.cppname for i in model_inputs]
            all_outputs = [o.cppname for o in model_outputs]
            if model.config.get_config_value("IOType") == "io_parallel":
                #for i in model_inputs: newline += indent + '#pragma HLS ARRAY_RESHAPE variable={} complete dim=0 \n'.format(i.cppname)
                #for o in model_outputs: newline += indent + '#pragma HLS ARRAY_RESHAPE variable={} complete dim=0 \n'.format(o.cppname)
                #newline += indent + '#pragma HLS INTERFACE ap_vld port={},{} \n'.format(','.join(all_inputs), ','.join(all_outputs))
                if model.config.model_strategy == 'Resource':
                    newline += indent + '#pragma HLS DATAFLOW \n'
                else:
                    newline += indent + '#pragma HLS PIPELINE \n'
            if model.config.get_config_value("IOType") == "io_serial":
                newline += indent + '#pragma HLS INTERFACE axis port={},{} \n'.format(','.join(all_inputs), ','.join(all_outputs))
                newline += indent + '#pragma HLS DATAFLOW \n'

            #inval_str = '\n    '.join(['const_size_in_{} = {};'.format(i, inp.size_cpp()) for i, inp in enumerate(model_inputs, 1)])
            #outval_str = '\n    '.join(['const_size_out_{} = {};'.format(i, out.size_cpp()) for i, out in enumerate(model_outputs, 1)])
            #newline += '\n' + indent + inval_str
            #newline += '\n' + indent + outval_str
            #newline += '\n'

        elif '//hls-fpga-machine-learning insert layers' in line:
            newline = line + '\n'
            inputs = model.get_input_variables()
            outputs = model.get_output_variables()
            for layer in model.get_layers():
                vars = layer.get_variables()
                for var in vars:
                    if var not in inputs and var not in outputs:
                        newline += '    ' + var.definition_cpp() + ';\n'
                        if var.pragma:
                            newline += '    ' + var.pragma + '\n'
                func = layer.function_cpp()
                if func:
                    for line in func:
                        newline += '    ' + line + '\n'
                    newline += '\n'

        #Just copy line
        else:
            newline = line

        fout.write(newline)

    f.close()
    fout.close()

def write_project_header(model):
    #######################
    ## myproject.h
    #######################

    filedir = os.path.dirname(os.path.abspath(__file__))
    f = open(os.path.join(filedir,'../templates/vivado/firmware/myproject.h'),'r')
    fout = open('{}/firmware/{}.h'.format(model.config.get_output_dir(), model.config.get_project_name()),'w')

    model_inputs = model.get_input_variables()
    model_outputs = model.get_output_variables()

    indent = '    '

    for line in f.readlines():

        if 'MYPROJECT' in line:
            newline = line.replace('MYPROJECT',format(model.config.get_project_name().upper()))
        elif 'void myproject(' in line:
            newline = 'void {}(\n'.format(model.config.get_project_name())
        elif '//hls-fpga-machine-learning insert header' in line:
            inputs_str = ', '.join([i.definition_cpp() for i in model_inputs])
            outputs_str = ', '.join([o.definition_cpp() for o in model_outputs])
            #insize_str = ', '.join(['unsigned short &const_size_in_{}'.format(i) for i in range(1, len(model_inputs) + 1)])
            #outsize_str = ', '.join(['unsigned short &const_size_out_{}'.format(o) for o in range(1, len(model_outputs) + 1)])

            newline = ''
            newline += indent + inputs_str + ',\n'
            newline += indent + outputs_str + '\n'
            #newline += indent + insize_str + ',\n'
            #newline += indent + outsize_str + '\n'
        else:
            newline = line
        fout.write(newline)

    f.close()
    fout.close()

def write_galapagos_packet(model):
    filedir = os.path.dirname(os.path.abspath(__file__))
    f = open(os.path.join(filedir,'../templates/vivado/firmware/packet.h'),'r')
    fout = open('{}/firmware/packet.h'.format(model.config.get_output_dir()),'w')

    for line in f.readlines():
        fout.write(line)

    f.close()
    fout.close()

def write_galapagos_defines(model):
    filedir = os.path.dirname(os.path.abspath(__file__))
    f = open(os.path.join(filedir,'../templates/vivado/firmware/defines.h'),'r')
    fout = open('{}/firmware/defines.h'.format(model.config.get_output_dir()),'w')

    for line in f.readlines():
        fout.write(line)

    f.close()
    fout.close()

def write_galapagos_inputs(model):
    filedir = os.path.dirname(os.path.abspath(__file__))
    f = open(os.path.join(filedir,'../templates/vivado/firmware/inputs.h'),'r')
    fout = open('{}/firmware/inputs.h'.format(model.config.get_output_dir()),'w')

    for line in f.readlines():
        if '//hls-fpga-machine-learning insert input data' in line:
            newline = line + '    ' + (',\n    '.join(','.join(str(hex(int(rand.getrandbits(32)))) for i in range(model.get_input_variables()[0].size())) for j in range(model.get_streamsize())))

        elif 'N_INPUTS' in line:
            newline = line.replace('N_INPUTS', model.get_input_variables()[0].size_cpp())
            
        else:
            newline = line

        fout.write(newline)    

    f.close()
    fout.close()

def write_galapagos_cpp(model):
    ###################
    ## galapagos_kerns.cpp
    ###################

    filedir = os.path.dirname(os.path.abspath(__file__))
    f = open(os.path.join(filedir,'../templates/vivado/firmware/galapagos_kerns.cpp'),'r')
    fout = open('{}/firmware/galapagos_kerns.cpp'.format(model.config.get_output_dir()),'w')

    model_inputs = model.get_input_variables()
    model_outputs = model.get_output_variables()

    indent = '    '

    for line in f.readlines():
        if 'myproject' in line:
            newline = line.replace('myproject', model.config.get_project_name())

        elif 'N_INPUTS' in line:
            newline = line.replace('N_INPUTS', model.get_input_variables()[0].size_cpp())
            if 'input_t' in newline:
                newline = newline.replace('input_t', model.get_input_variables()[0].type_name())

        elif 'N_OUTPUTS' in line:
            newline = line.replace('N_OUTPUTS', model.get_output_variables()[0].size_cpp())
            if 'result_t' in newline:
                newline = newline.replace('result_t', model.get_output_variables()[0].type_name())

        elif 'IN_BITS' in line:
            bit_width = 32
            for inp in model.get_input_variables():
                layer_precision = inp.get_precision()
                bit_width = layer_precision[layer_precision.find("<")+len("<"):layer_precision.rfind(">")]
                bit_width = bit_width.split(",")[0]
            newline = line.replace('IN_BITS',bit_width)

        elif 'OUT_BITS' in line:
            bit_width = 32
            for outp in model.get_output_variables():
                layer_precision = outp.get_precision()
                bit_width = layer_precision[layer_precision.find("<")+len("<"):layer_precision.rfind(">")]
                bit_width = bit_width.split(",")[0]
            newline = line.replace('OUT_BITS',bit_width)

        #Just copy line
        else:
            newline = line

        fout.write(newline)

    f.close()
    fout.close()

def write_galapagos_header(model):
    #######################
    ## galapagos_kerns.h
    #######################

    filedir = os.path.dirname(os.path.abspath(__file__))
    f = open(os.path.join(filedir,'../templates/vivado/firmware/galapagos_kerns.h'),'r')
    fout = open('{}/firmware/galapagos_kerns.h'.format(model.config.get_output_dir()),'w')

    for line in f.readlines():
        fout.write(line)

    f.close()
    fout.close()

def write_parameters(model):
    filedir = os.path.dirname(os.path.abspath(__file__))
    f = open(os.path.join(filedir,'../templates/vivado/firmware/parameters.h'),'r')
    fout = open('{}/firmware/parameters.h'.format(model.config.get_output_dir()),'w')

    for line in f.readlines():

        #Insert numbers
        if '//hls-fpga-machine-learning insert numbers' in line:
            newline = line
            numbers = OrderedDict.fromkeys([layer.get_numbers_cpp() for layer in model.get_layers()])
            newline += ''.join(numbers)
            newline += '#define STREAMSIZE {streamsize}\n'.format(streamsize=model.get_streamsize())

        elif '//hls-fpga-machine-learning insert layer-precision' in line:
            newline = line
            all_precision = OrderedDict()
            for layer in model.get_layers():
                layer_precision = layer.get_layer_precision()
                all_precision.update(layer_precision)
            for used_type in all_precision.values():
                newline += used_type.definition_cpp()

        elif "//hls-fpga-machine-learning insert layer-config" in line:
            newline = line
            for layer in model.get_layers():
                config = layer.config_cpp()
                if config:
                    newline += config + '\n'
        else:
            newline = line
        fout.write(newline)
    f.close()
    fout.close()

def write_weights(model):
    for layer in model.get_layers():
        for weights in layer.get_weights():
            print_array_to_cpp(weights, model.config.get_output_dir())

def write_test_bench(model):
    ###################
    ## test bench
    ###################

    filedir = os.path.dirname(os.path.abspath(__file__))

    if not os.path.exists('{}/tb_data/'.format(model.config.get_output_dir())):
        os.mkdir('{}/tb_data/'.format(model.config.get_output_dir()))
    input_data = model.config.get_config_value('InputData')
    output_predictions = model.config.get_config_value('OutputPredictions')
    if input_data is not None:
        copyfile(input_data, '{}/tb_data/tb_input_features.dat'.format(model.config.get_output_dir()))
    if output_predictions is not None:
        copyfile(output_predictions, '{}/tb_data/tb_output_predictions.dat'.format(model.config.get_output_dir()))

    f = open(os.path.join(filedir,'../templates/vivado/myproject_test.cpp'),'r')
    fout = open('{}/{}_test.cpp'.format(model.config.get_output_dir(), model.config.get_project_name()),'w')

    for line in f.readlines():
        indent = ' ' * (len(line) - len(line.lstrip(' ')))

        #Insert numbers
        if 'myproject' in line:
            newline = line.replace('myproject', model.config.get_project_name())
        elif '//hls-fpga-machine-learning insert data' in line:
            newline = line
            for inp in model.get_input_variables():
                input_str = '      ' + inp.definition_cpp() + ' = {};\n'
                default_val = ','.join('in[{}]'.format(i) for i in range(inp.size()))
                newline += input_str.format('{' + default_val + '}')
            for out in model.get_output_variables():
                output_str = '      ' + out.definition_cpp() + ' = {};\n'
                default_val = ','.join(str(o) for o in [0] * out.size())
                newline += output_str.format('{' + default_val + '}')
        elif '//hls-fpga-machine-learning insert zero' in line:
            newline = line
            for inp in model.get_input_variables():
                input_str = '    ' + inp.definition_cpp() + ' = {};\n'
                default_val = ','.join(str(i) for i in [0] * inp.size())
                newline += input_str.format('{' + default_val + '}')
            for out in model.get_output_variables():
                output_str = '    ' + out.definition_cpp() + ' = {};\n'
                default_val = ','.join(str(o) for o in [0] * out.size())
                newline += output_str.format('{' + default_val + '}')
            newline = newline + '\n' + indent + '#include "firmware/defines.h"'
            newline = newline + '\n' + indent + '#include "firmware/inputs.h"'

            newline = newline + '\n' + indent + 'for(int i = 0; i < STREAMSIZE; i++) {'
            newline = newline + '\n' + indent + '  std::cout <<"******************************" << std::endl << "Input:" << std::endl << std::endl;'
            for inp in model.get_input_variables():
              newline = newline + '\n' + indent + '  for(int j = 0; j < {}; j++) {{'.format(inp.size_cpp())
              newline = newline + '\n' + indent + '    {}[j]({}[j].length()-1,0) = input_vals[i*{}+j]({}[j].length()-1,0);'.format(inp.cppname, inp.cppname, inp.size_cpp(), inp.cppname)
              newline = newline + '\n' + indent + '    std::cout <<"int[" << std::dec << j << "]: " << std::hex << input_vals[i*{}+j] << std::endl;'.format(inp.size_cpp())
              newline = newline + '\n' + indent + '  }'

        elif '//hls-fpga-machine-learning insert top-level-function' in line:
            newline = line

            #size_str = indent + 'unsigned short {},{};\n'
            #input_size_vars = ','.join(['size_in{}'.format(i) for i in range(1, len(model.get_input_variables()) + 1)])
            #output_size_vars = ','.join(['size_out{}'.format(o) for o in range(1, len(model.get_output_variables()) + 1)])
            #newline += size_str.format(input_size_vars, output_size_vars)

            input_vars = ','.join([i.cppname for i in model.get_input_variables()])
            output_vars = ','.join([o.cppname for o in model.get_output_variables()])
            top_level = indent + '  {}({},{});\n'.format(model.config.get_project_name(), input_vars, output_vars)
            newline += top_level
        elif '//hls-fpga-machine-learning insert predictions' in line:
            newline = line
            for out in model.get_output_variables():
                newline += indent + 'for(int i = 0; i < {}; i++) {{\n'.format(out.size_cpp())
                newline += indent + '  std::cout << pr[i] << " ";\n'
                newline += indent + '}\n'
                newline += indent + 'std::cout << std::endl;\n'
        elif '//hls-fpga-machine-learning insert tb-output' in line:
            newline = line
            for out in model.get_output_variables():
                newline += indent + 'for(int i = 0; i < {}; i++) {{\n'.format(out.size_cpp())
                newline += indent + '  fout << {}[i] << " ";\n'.format(out.cppname)
                newline += indent + '}\n'
                newline += indent + 'fout << std::endl;\n'
        elif '//hls-fpga-machine-learning insert output' in line:
            newline = line
            newline = newline + '\n' + indent + '  std::cout <<"******************************" << std::endl << "Output:" << std::endl << std::endl;\n'
            for out in model.get_output_variables():
                newline += indent + '  for(int j = 0; j < {}; j++) {{\n'.format(out.size_cpp())
                newline += indent + '    ap_uint<PACKET_DATA_LENGTH> tmpout; tmpout({}[j].length()-1,0) = {}[j]({}[j].length()-1,0);\n'.format(out.cppname, out.cppname, out.cppname)
                #newline += indent + '  std::cout << {}[i] << " ";\n'.format(out.cppname)
                newline += indent + '    std::cout <<"out[" << std::dec << j << "]: " << std::hex << tmpout << std::endl;\n'
                newline += indent + '  }\n'
                newline += indent + '}\n'
                newline += indent + 'std::cout << std::endl;\n'
        elif '//hls-fpga-machine-learning insert quantized' in line:
            newline = line
            for out in model.get_output_variables():
                newline += indent + 'for(int i = 0; i < {}; i++) {{\n'.format(out.size_cpp())
                newline += indent + '  std::cout << {}[i] << " ";\n'.format(out.cppname)
                newline += indent + '}\n'
                newline += indent + 'std::cout << std::endl;\n'
        else:
            newline = line
        fout.write(newline)
    f.close()
    fout.close()

def write_build_script(model):
    ###################
    # build_prj.tcl
    ###################

    filedir = os.path.dirname(os.path.abspath(__file__))
    nnetdir = os.path.abspath(os.path.join(filedir, "../templates/vivado/nnet_utils"))
    relpath = os.path.relpath(nnetdir, start=model.config.get_output_dir())

    f = open(os.path.join(filedir,'../templates/vivado/build_prj.tcl'),'r')
    fout = open('{}/build_prj.tcl'.format(model.config.get_output_dir()),'w')

    for line in f.readlines():

        line = line.replace('myproject',model.config.get_project_name())
        line = line.replace('nnet_utils', relpath)

        if 'set_part {xc7vx690tffg1927-2}' in line:
            line = 'set_part {{{}}}\n'.format(model.config.get_config_value('XilinxPart'))
        elif 'create_clock -period 5 -name default' in line:
            line = 'create_clock -period {} -name default\n'.format(model.config.get_config_value('ClockPeriod'))

        fout.write(line)
    f.close()
    fout.close()

def write_galapagos_send_build_script(model):
    ###################
    # generate_gal_send.tcl
    ###################

    filedir = os.path.dirname(os.path.abspath(__file__))
    nnetdir = os.path.abspath(os.path.join(filedir, "../nnet_utils"))
    relpath = os.path.relpath(nnetdir, start=model.config.get_output_dir())

    f = open(os.path.join(filedir,'../templates/vivado/generate_gal_send.tcl'),'r')
    fout = open('{}/generate_gal_send.tcl'.format(model.config.get_output_dir()),'w')

    for line in f.readlines():

        line = line.replace('myproject',model.config.get_project_name())
        line = line.replace('nnet_utils', relpath)

        if 'set_part {xc7vx690tffg1927-2}' in line:
            line = 'set_part {{{}}}\n'.format(model.config.get_config_value('XilinxPart'))
        elif 'create_clock -period 5 -name default' in line:
            line = 'create_clock -period {} -name default\n'.format(model.config.get_config_value('ClockPeriod'))

        fout.write(line)
    f.close()
    fout.close()

def write_galapagos_nn_build_script(model):
    ###################
    # generate_gal_nn.tcl
    ###################

    filedir = os.path.dirname(os.path.abspath(__file__))
    nnetdir = os.path.abspath(os.path.join(filedir, "../nnet_utils"))
    relpath = os.path.relpath(nnetdir, start=model.config.get_output_dir())

    f = open(os.path.join(filedir,'../templates/vivado/generate_gal_nn.tcl'),'r')
    fout = open('{}/generate_gal_nn.tcl'.format(model.config.get_output_dir()),'w')

    for line in f.readlines():

        line = line.replace('myproject',model.config.get_project_name())
        line = line.replace('nnet_utils', relpath)

        if 'set_part {xc7vx690tffg1927-2}' in line:
            line = 'set_part {{{}}}\n'.format(model.config.get_config_value('XilinxPart'))
        elif 'create_clock -period 5 -name default' in line:
            line = 'create_clock -period {} -name default\n'.format(model.config.get_config_value('ClockPeriod'))

        fout.write(line)
    f.close()
    fout.close()

def write_galapagos_cpu_node(model):
    ###################
    # cpu_node.cpp
    ###################

    filedir = os.path.dirname(os.path.abspath(__file__))

    f = open(os.path.join(filedir,'../templates/vivado/cpu_node.cpp'),'r')
    fout = open('{}/cpu_node.cpp'.format(model.config.get_output_dir()),'w')

    for line in f.readlines():

        fout.write(line)

    f.close()
    fout.close()

def write_galapagos_heterogeneous_node(model):
    ###################
    # heterogeneous_node.cpp
    ###################

    filedir = os.path.dirname(os.path.abspath(__file__))

    f = open(os.path.join(filedir,'../templates/vivado/heterogeneous_node.cpp'),'r')
    fout = open('{}/heterogeneous_node.cpp'.format(model.config.get_output_dir()),'w')

    for line in f.readlines():

        fout.write(line)

    f.close()
    fout.close()

def write_galapagos_program_fpga(model):
    ###################
    # program_fpga.tcl 
    ###################

    filedir = os.path.dirname(os.path.abspath(__file__))

    f = open(os.path.join(filedir,'../templates/vivado/program_fpga.tcl'),'r')
    fout = open('{}/program_fpga.tcl'.format(model.config.get_output_dir()),'w')

    for line in f.readlines():

        line = line.replace('GALAPAGOS_DIR',model.config.get_galapagos_dir())

        fout.write(line)
    f.close()
    fout.close()

def write_galapagos_logical_xml(model):
    ###################
    # logical.xml
    ###################

    filedir = os.path.dirname(os.path.abspath(__file__))

    f = open(os.path.join(filedir,'../templates/vivado/middlewareInput/conf_oneFPGA/logical.xml'),'r')
    fout = open('{}/middlewareInput/conf_oneFPGA/logical.xml'.format(model.config.get_output_dir()),'w')

    for line in f.readlines():

        fout.write(line)
    f.close()
    fout.close()

def write_galapagos_map_xml(model):
    ###################
    # map.xml
    ###################

    filedir = os.path.dirname(os.path.abspath(__file__))

    f = open(os.path.join(filedir,'../templates/vivado/middlewareInput/conf_oneFPGA/map.xml'),'r')
    fout = open('{}/middlewareInput/conf_oneFPGA/map.xml'.format(model.config.get_output_dir()),'w')

    for line in f.readlines():

        fout.write(line)
    f.close()
    fout.close()

def write_galapagos_make(model):
    ###################
    ## Makefile
    ###################

    filedir = os.path.dirname(os.path.abspath(__file__))

    f = open(os.path.join(filedir,'../templates/vivado/Makefile'),'r')
    fout = open('{}/Makefile'.format(model.config.get_output_dir()),'w')

    for line in f.readlines():

        line = line.replace('myproject',model.config.get_project_name())
        fout.write(line)
    f.close()
    fout.close()

def write_nnet_utils(model):
    ###################
    ## nnet_utils
    ###################

    filedir = os.path.dirname(os.path.abspath(__file__))

    srcpath = os.path.join(filedir,'../templates/vivado/nnet_utils/')
    dstpath = '{}/firmware/nnet_utils/'.format(model.config.get_output_dir())

    if not os.path.exists(dstpath):
        os.mkdir(dstpath)

    headers = [os.path.basename(h) for h in glob.glob(srcpath + '*.h')]

    for h in headers:
        copyfile(srcpath + h, dstpath + h)

def write_tar(model):
    ###################
    # Tarball output
    ###################

    with tarfile.open(model.config.get_output_dir() + '.tar.gz', mode='w:gz') as archive:
        archive.add(model.config.get_output_dir(), recursive=True)

def write_hls(model):
    print('Writing HLS project')
    write_project_dir(model)
    write_project_cpp(model)
    write_project_header(model)
    write_weights(model)
    write_parameters(model)
    write_test_bench(model)
    write_build_script(model)
    write_galapagos_header(model)
    write_galapagos_cpp(model)
    write_galapagos_packet(model)
    write_galapagos_defines(model)
    write_galapagos_inputs(model)
    write_galapagos_send_build_script(model)
    write_galapagos_nn_build_script(model)
    write_galapagos_heterogeneous_node(model)
    write_galapagos_cpu_node(model)
    write_galapagos_program_fpga(model)
    write_galapagos_logical_xml(model)
    write_galapagos_map_xml(model)
    write_galapagos_make(model)
    write_nnet_utils(model)
    write_tar(model)
    print('Done')