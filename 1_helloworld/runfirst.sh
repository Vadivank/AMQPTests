# before start this script use
# $ chmod u+x ./runfirst.sh
mkdir build
cd build
conan install .. --build=missing
cmake ..
# cmake --build .
chmod u+x ../run.sh
../run.sh