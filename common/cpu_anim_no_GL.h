#include <cstdio>
#include <cstdlib>

struct CPUAnimBitmap {
    unsigned char *pixels;
    int width, height;
    void *dataBlock;
    FILE *ffmpegPipe; // 新增：用于通过管道写入 ffmpeg

    CPUAnimBitmap(int w, int h, void *d = NULL) {
        width = w;
        height = h;
        pixels = new unsigned char[width * height * 4];
        dataBlock = d;
        ffmpegPipe = NULL;
    }

    // 新增：初始化视频导出
    void start_video_record(const char* filename, int fps = 30) {
        char command[512];
        // 使用 ffmpeg 将 RGBA 原始流转换为 mp4
        // -f rawvideo: 输入格式为原始视频
        // -pix_fmt rgba: 像素格式
        // -s: 分辨率
        // -i -: 从标准输入读取
        sprintf(command, "ffmpeg -y -f rawvideo -pix_fmt rgba -s %dx%d -r %d -i - -c:v libx264 -pix_fmt yuv420p %s", 
                width, height, fps, filename);
        ffmpegPipe = popen(command, "w");
        if (!ffmpegPipe) {
            printf("Error: Could not open ffmpeg pipe. Make sure ffmpeg is installed.\n");
        }
    }

    // 新增：录制单帧并退出逻辑
    void record_frames(void (*f)(void*, int), int totalFrames) {
        for (int t = 0; t < totalFrames; t++) {
            // 调用你的 GPU/CPU 渲染函数
            f(dataBlock, t);
            
            // 将渲染好的像素写入管道
            if (ffmpegPipe) {
                fwrite(pixels, width * height * 4, 1, ffmpegPipe);
            }
            
            if (t % 10 == 0) printf("Recording frame %d/%d...\n", t, totalFrames);
        }
        
        if (ffmpegPipe) pclose(ffmpegPipe);
        printf("Video saved successfully.\n");
    }

    ~CPUAnimBitmap() {
        delete[] pixels;
    }
};
