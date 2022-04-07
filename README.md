## 环境配置

### 海康工业相机SDK

### opencv

### opencv-contirb

## 工业线阵相机的高效采样装置

## 图像拼接与融合

为了保证相邻相机首尾相连处数据不丢失，必须使两相机之间有效像素部分重合。

将下面两张图作为输入，测试算法的性能。

> 无法对无镜头相机采集的图像进行拼接，因为根本识别不到特征点

![1.png](https://s2.loli.net/2021/12/18/2m6b3uCEMprKkPx.png) ![2.png](https://s2.loli.net/2021/12/18/q1sW5uBP6gvyZfe.png)

### 冗余图像特征点提取

用SIFT算法来实现图像拼接是很常用的方法，但是因为SIFT计算量很大，所以在速度要求很高的场合下不再适用。
所以，它的改进方法SURF因为在速度方面有了明显的提高（速度是SIFT的3倍）。虽说SURF精确度和稳定性不及SIFT，但是其综合能力还是优越一些。
同样，在计算速度方面作了优化的还有ORB算法。ORB（Oriented FAST and Rotated BRIEF）是一种快速特征点提取和描述的算法。
ORB特征是将FAST特征点的检测方法与BRIEF特征描述子结合起来，并在它们原来的基础上做了改进与优化。
据说，ORB算法的速度是sift的100倍，是surf的10倍。

根据应用场景的特点，我复现了Orb和Surf算法

### 特征点匹配

- Surf特征点匹配方法
  ![surf_match.png](https://s2.loli.net/2021/12/18/rqgOF7DtzB9ARG3.png)
- Orb特征点匹配方法
  ![orb_combine.png](https://s2.loli.net/2021/12/18/rbMndcWKixlEwuJ.png)

### 计算图像配准点与图像配准

- 对Surf提取的特征点进行配准
  ![surf_result.jpg](https://s2.loli.net/2021/12/18/vdUG1DVWHZr9Njy.jpg)
- 对Orb提取的特征点进行配准
  ![orb_result.jpg](https://s2.loli.net/2021/12/18/PpvFiq4xweZByOd.jpg)

### 配准图像优化

在拼接图的交界处，两图因为光照色泽的原因使得两图交界处的过渡很糟糕，所以需要特定的处理解决这种不自然。

利用加权融合，在重叠部分由前一幅图像慢慢过渡到第二幅图像，即将图像的重叠区域的像素值按一定的权值相加合成新的图像。

![1639827466571.png](https://s2.loli.net/2021/12/18/3yv5u9pqrQXnA12.png)

### 使用线阵相机的图像拼接与融合

目前观察到，这类拼接算法都是左右图像拼接。所以，在拼接之前，要对采集到的图像顺时针旋转90度，再进行拼接。

> 案例图像为400x200的分辨率，两种拼接耗时可以控制在毫秒级。在实际应用场景的速度还有待测试。

## 参考资料

https://blog.csdn.net/sss_369/article/details/87740843

https://www.cnblogs.com/skyfsm/p/7401523.html

https://www.cnblogs.com/skyfsm/p/7411961.html

https://blog.csdn.net/Marchal_G/article/details/51066901

https://zhuanlan.zhihu.com/p/71777362

https://blog.csdn.net/qq_26907755/article/details/81772309?utm_medium=distribute.pc_relevant.none-task-blog-2~default~baidujs_title~default-3&spm=1001.2101.3001.4242.2

https://blog.csdn.net/weixin_42717395/article/details/85768313
