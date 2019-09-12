
#include <string>
#include <math.h>
#include <thread>
#include <chrono>

#include "galapagos_node.hpp"
#include "firmware/galapagos_kerns.h"


#define NUM_KERNELS 3


int main(int argc, const char** argv){

    std::vector <std::string> kern_info;

    std::string source_ip_str("10.1.2.155");
    std::string dest_ip_str("10.1.2.156");

    kern_info.push_back(source_ip_str);
    kern_info.push_back(dest_ip_str);
//    kern_info.push_back(source_ip_str);

    galapagos::node <ap_uint <PACKET_DATA_LENGTH> > node(kern_info, source_ip_str);
    node.add_kernel(0, kern_send);
    //node.add_kernel(1, kern_nn); //running on fpga
  //  node.add_kernel(2, kern_recv);

    node.start();
    node.end();

}
