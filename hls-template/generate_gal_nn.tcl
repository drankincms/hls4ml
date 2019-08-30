set galapagos_path $::env(GALAPAGOS_PATH)
set board_name $::env(GALAPAGOS_BOARD_NAME)
set part_name $::env(GALAPAGOS_PART)
set src_path_root [pwd] 

cd $galapagos_path/hlsBuild/${board_name}/ip

open_project kern_nn
set_top kern_nn
open_solution "solution1"
set_part {xc7vx690tffg1927-2}
add_files firmware/myproject.cpp -cflags "-I[file normalize nnet_utils] -std=c++0x"
add_files firmware/galapagos_kerns.cpp -cflags "-I $galapagos_path/middleware/include -I firmware -I firmware/weights -I[file normalize nnet_utils] -std=c++0x"
catch {config_array_partition -maximum_size 4096}
create_clock -period 5 -name default
config_interface -expose_global
csynth_design
export_design -format ip_catalog
close_project

quit
