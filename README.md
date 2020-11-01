## 锦屏数据转化器

### 使用场景
1. 转化原始数据到HDF5HDF5
2. **重点** 转化模拟输出数据到HDFHDF5

### 使用方法
1. 使用C++17标准安装root与geant4
2. 使用C++17标准安装JSAP，安装目录设置为$JSAPSYS
3. 克隆·仓库并编译软件
   `git clone https://gitlab.airelinux.org/wuyy/root2hdf5.git`
   `cd root2hdf5/C++`
   `mkdir build`
   `cd build`
   `cmake ..`
   `make -j`
4. 运行 `./ConvertSimData -h` 与 `./ConvertRawData -h` 查看简单的使用说明
5. 运行 `./ConvertSimData input output` 转化模拟数据。因为锦屏模拟数据中的track信息有三种保存模式，最终的输出形态可能有三种