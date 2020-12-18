## 锦屏数据转化器

### 使用场景
1. 转化原始数据到HDF5HDF5
2. **重点** 转化模拟输出数据到HDF5。

### Motivation
1. 锦屏模拟数据的结构包含了多层自定义结构体向量，当前主流工具无法解析，唯一的读取方案是ROOT配合字典，不便于非C++程序读取
2. 由于TriggerInfo 与 Truth 信息有多对一的关系（一个触发可能对应多个Vertex），JSAP 输出的模拟数据将Truth写了两遍，造成了冗余。

### Feature
1. 将`.root`格式转化为`.h5`格式数据
2. 将树状数据结构扁平化：
   将 Readout Tree 拆开为触发信息与波形信息
   将 SimTruth Tree 拆开为Truth信息、Deposit Energy、Primary Particle、Track
   将 Track 拆开为 TrackId 与 Step Points
   将 SimTriggerInfo Tree 拆开为TriggerId, PE, truthId

### 使用方法
1. 使用C++17标准安装root与geant4
2. 使用C++17标准安装JSAP，安装目录设置为$JSAPSYS
3. 克隆·仓库并编译软件
   `git clone https://gitlab.airelinux.org/wuyy/root2hdf5.git`
   `cd root2hdf5`
   `mkdir build`
   `cd build`
   `cmake ..`
   `make -j`
4. 运行 `./ConvertSimData -h` 与 `./ConvertRawData -h` 查看简单的使用说明
5. 运行 `./ConvertSimData input output` 转化模拟数据。因为锦屏模拟数据中的track信息有三种保存模式，最终的输出形态可能有三种
