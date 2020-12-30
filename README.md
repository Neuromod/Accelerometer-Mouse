# Gyro-based Digilent Arty S7 Windows mouse

## Hardware requirements

* [Digilent Arty S7](https://reference.digilentinc.com/reference/programmable-logic/arty-s7/start)
* [PmodNAV](https://store.digilentinc.com/pmod-nav-9-axis-imu-plus-barometer/)

<br/>

## Software requirements

* Xilinx Vivado & Vitis
* Digilent's PmodNAV drivers
* Microblaze soft processor
* C++ compiler

<br/>

## Instructions

1. Install Digilent's PmodNAV drivers.
2. Create a Microblaze soft processor and give it access to the board buttons, LEDs and the PMOD board.
3. Compile and run the FPGA C++ program.
4. Compile and run the Windows C++ program.
