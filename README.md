# HL_Player
### 本项目是一个基于 FFmpeg 的播放器，使用 Qt 编写 UI 界面，使用 OpenGL 完成画面渲染及数据格式转换。
### 技术栈：C++、Qt、FFmpeg、OpenGL
#### 主要工作：
#### 1、使用 Qt 的多线程和信号槽机制实现解封装、解码、播放的并发处理;
#### 2、根据解码后的音频帧与视频帧的 pts，采取视频同步到音频的策略，完成音画播放同步;
#### 3、快进、快退、进度条跳转、声音控制、暂停等播放控制功能;
#### 4、使用了 OpenGL 的硬件加速功能，对视频进行了解码和渲染，以提高播放流畅度和图像质量;
#### 5、针对不同的视频和音频格式进行了优化，确保了在各种情况下都能正常播放。
![10086](https://github.com/Geckoops/HL_Player/assets/26892656/1e09d295-ef12-4539-97d0-a80812af8713)
