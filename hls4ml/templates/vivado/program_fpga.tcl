open_hw
connect_hw_server
open_hw_target
set_property PROGRAM.FILE {GALAPAGOS_DIR/projects/test/1/1.runs/impl_1/shellTop.bit} [get_hw_devices xcku115_0]
set_property PROBES.FILE {GALAPAGOS_DIR/projects/test/1/1.runs/impl_1/shellTop.ltx} [get_hw_devices xcku115_0]
set_property FULL_PROBES.FILE {GALAPAGOS_DIR/projects/test/1/1.runs/impl_1/shellTop.ltx} [get_hw_devices xcku115_0]
current_hw_device [get_hw_devices xcku115_0]
refresh_hw_device [lindex [get_hw_devices xcku115_0] 0]
program_hw_devices [get_hw_devices xcku115_0]
refresh_hw_device [lindex [get_hw_devices xcku115_0] 0]
