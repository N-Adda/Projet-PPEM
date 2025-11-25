#!/bin/bash

## Convert to C6x format
git clone https://github.com/preesm/yuv2dat.git
cd yuv2dat
cmake .
cmake --build . --target yuv2dat	
cd ..
./yuv2dat/Debug/yuv2dat.exe -x=270 -y=480 -i=BBB_3D_R.yuv -o=BBB_3D_R.dat -f=5
./yuv2dat/Debug/yuv2dat.exe -x=270 -y=480 -i=BBB_3D_L.yuv -o=BBB_3D_L.dat -f=5