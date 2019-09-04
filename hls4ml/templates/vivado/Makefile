include $(GALAPAGOS_PATH)/include.mk
middleware_path = $(GALAPAGOS_PATH)/middleware
hls_path = $(middleware_path)/hls
python_path = $(middleware_path)/python

SRC_LIB=$(GALAPAGOS_PATH)/middleware/CPP_lib/Galapagos_lib
CXXFLAGS = -DCPU -g -std=c++17 -pthread -isystem $(XILINX_VIVADO)/include -I$(GALAPAGOS_PATH)/middleware/include -I$(SRC_LIB) -I../nnet_utils

LDFLAGS = -lpthread

BOOST_LDFLAGS=-lboost_thread -lboost_system $(LDFLAGS) 

MIDDLEWARE_DIR= $(shell pwd)/middlewareInput
CONFIG_DIR = conf_oneFPGA
LOGICALFILE= $(MIDDLEWARE_DIR)/$(CONFIG_DIR)/logical.xml
MAPFILE= $(MIDDLEWARE_DIR)/$(CONFIG_DIR)/map.xml
PROJECTNAME = test

all: galapagos_kerns.o

galapagos_kerns.o: firmware/galapagos_kerns.cpp
	$(CXX) $(CXXFLAGS) -c firmware/galapagos_kerns.cpp -o $@ $(BOOST_LDFLAGS)

myproject.o: firmware/myproject.cpp
	$(CXX) $(CXXFLAGS) -c firmware/myproject.cpp -o $@ $(BOOST_LDFLAGS)

hls:
	vivado_hls generate_gal_nn.tcl

middleware:
	python3.5 ${python_path}/globalFPGAParser.py --logicalFile=${LOGICALFILE} \
		--mapFile=${MAPFILE} --projectName=${PROJECTNAME}
	chmod +x $(GALAPAGOS_PATH)/projects/$(PROJECTNAME)/createCluster.sh

hlsmiddleware:
	 $(MAKE) -C $(hls_path)


heterogeneous_node.exe: myproject.o galapagos_kerns.o heterogeneous_node.cpp 
	$(CXX) $(CXXFLAGS) -I$(SRC_LIB) -I$(GALAPAGOS_PATH)/util/argparse $^ -o $@ $(BOOST_LDFLAGS)  

cpu_node.exe: myproject.o galapagos_kerns.o cpu_node.cpp 
	$(CXX) $(CXXFLAGS) -I$(SRC_LIB) -I$(GALAPAGOS_PATH)/util/argparse $^ -o $@ $(BOOST_LDFLAGS)  

clean:
	rm -rf $(GALAPAGOS_PATH)/hlsBuild/$(GALAPAGOS_BOARD_NAME)/ip/myproject_prj
	rm -rf *.exe 
