# before start this script use
# $ chmod u+x ./runfirst.sh
mkdir build-3_publish-subscribe-Debug
cd build-3_publish-subscribe-Debug
conan install .. --build=missing
cmake ..
# cmake --build .
chmod u+x ../run.sh
../run.sh