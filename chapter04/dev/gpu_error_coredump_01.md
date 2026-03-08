

# 【问题】运行出现coredump


既然你贴出了 cpu_bitmap.h 的源码，真相大白了！Segmentation Fault 的根源就在这个类的设计逻辑中：
1. 致命原因：未初始化的静态指针
看这段代码：

```
static CPUBitmap** get_bitmap_ptr( void ) {
    static CPUBitmap *gBitmap; // 这是一个静态局部变量，初始值为 NULL
    return &gBitmap;
}
```

在你之前的 main 函数中，你调用了 bitmap.get_bitmap_ptr()。注意：

* get_bitmap_ptr() 返回的是内部静态变量 gBitmap 的地址。
* 但是！gBitmap 本身只有在调用 display_and_exit() 时才会被赋值为 this。
* 如果你没调 display_and_exit() 就直接解引用这个指针，或者 cudaMemcpy 往 *get_bitmap_ptr() 指向的地址写数据，就是在往 NULL 地址写数据。

2. 修复方法
别用那个复杂的静态方法 get_bitmap_ptr()，直接用你对象自带的成员函数 get_ptr()（或者是你自己定义的 pixels 成员）。
请将 main 函数中的 cudaMemcpy 行修改为：

// 改用 get_ptr() 访问对象内部真正的 pixels 缓冲区
HANDLE_ERROR( cudaMemcpy( bitmap.get_ptr(), dev_bitmap, bitmap.image_size(), cudaMemcpyDeviceToHost ) );

3. 为什么会有 gl_helper.h 报错？
由于 cpu_bitmap.h 头部 #include "gl_helper.h"，而 gl_helper.h 又去引用 GL/glut.h，所以即使你没用到绘图功能，编译器也会因为找不到 OpenGL 头文件而罢工。
建议终极方案：
既然你已经给 CPUBitmap 加上了 save_ppm 方法，你现在的 julia_gpu.cu 编译命令应该改成这样来强行跳过 OpenGL 链接：

   1. 在 julia_gpu.cu 开头加上 #define GLUT_NO_LIB_PRAGMA（如果库里有的话）。
   2. 或者最简单的：修改 main 里的拷贝行，然后直接运行。

