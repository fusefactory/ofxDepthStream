export ASTRA_ROOT=/Users/mark/code/AstraSDK-0.5.0-20160426T102621Z-darwin-x64/

if [ ! -d build ]; then
  mkdir build
fi

cd build

export ASTRA_ROOT=$ASTRA_ROOT && cmake ..

make -j4

echo ""
echo "AstraServer executable should be in the ./build folder"
echo ""
