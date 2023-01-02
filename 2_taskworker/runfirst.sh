# before start this script use
# $ chmod u+x ./runfirst.sh
mkdir build-2_taskworker-Debug
cd build-2_taskworker-Debug
conan install .. --build=missing
cmake ..
# cmake --build .
chmod u+x ../run.sh
../run.sh