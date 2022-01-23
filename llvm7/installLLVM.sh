#  !bash

tar -xzvf llvm7.0.0.tar.gz
tar -xzvf binutils-2.37.tar.gz
mv binutils-2.37 ./llvm7.0.0/

cur_path=`pwd`
llvm_path=$cur_path/llvm7.0.0

# 1. build binutils-2.32
cd $llvm_path/bin_build
../binutils-2.37/configure --enable-gold --enable-plugins --disable-werro
make all-gold
if [ ! -f "$llvm_path/bin_build/gold/ld-new" ]; then
	exit 0
fi
cd -

# 2. build llvm with gold plugin
cd $llvm_path/build	
cmake -DCMAKE_BUILD_TYPE:String=Release -DLLVM_BINUTILS_INCDIR=../binutils-2.32/include ../llvm-7.0.0.src
make
if [ ! -f "$llvm_path/build/bin/clang" ]; then
	exit 0
fi
cd -


# 3. write environment variables
export LLVM_PATH=$llvm_path
export CLANG_PATH=$LLVM_PATH/build/bin
export PATH=$PATH:$CLANG_PATH

echo "export LLVM_PATH=$llvm_path"  >> /etc/profile
echo "export CLANG_PATH=$LLVM_PATH/build/bin" >> /etc/profile
echo "export PATH=$PATH:$CLANG_PATH" >> /etc/profile

cd /usr/bin/
sudo mv ld ld-gnu

binutils=$llvm_path/bin_build/binutils
sudo cp $llvm_path/bin_build/gold/ld-new ./ld

cd /usr/lib
sudo mkdir bfd-plugins
cd bfd-plugins
sudo cp $llvm_path/build/lib/LLVMgold.so ./
sudo cp $llvm_path/build/lib/libLTO.* ./