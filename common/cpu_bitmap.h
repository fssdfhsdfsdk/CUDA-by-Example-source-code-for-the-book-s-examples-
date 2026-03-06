/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and
 * proprietary rights in and to this software and related documentation.
 * Any use, reproduction, disclosure, or distribution of this software
 * and related documentation without an express license agreement from
 * NVIDIA Corporation is strictly prohibited.
 *
 * Please refer to the applicable NVIDIA end user license agreement (EULA)
 * associated with this source code for terms and conditions that govern
 * your use of this NVIDIA software.
 *
 */


#ifndef __CPU_BITMAP_H__
#define __CPU_BITMAP_H__

#include "gl_helper.h"

struct CPUBitmap {
    unsigned char    *pixels;
    int     x, y;
    void    *dataBlock;
    void (*bitmapExit)(void*);

    CPUBitmap( int width, int height, void *d = NULL ) {
        pixels = new unsigned char[width * height * 4];
        x = width;
        y = height;
        dataBlock = d;
    }

    ~CPUBitmap() {
        delete [] pixels;
    }

    // 在 ~CPUBitmap() 析构函数后添加：
    void save_ppm(const char *filename) {
        FILE *fp = fopen(filename, "wb");
        if (!fp) {
            fprintf(stderr, "Error: could not open %s for write\n", filename);
            return;
        }
        // PPM 格式头：P6 = 二进制 RGB, 宽 高, 255 = 最大颜色值
        fprintf(fp, "P6\n%d %d\n255\n", x, y);
        
        // 写入像素数据（跳过 alpha 通道，只保存 RGB）
        for (int i = 0; i < x * y; i++) {
            fputc(pixels[i * 4 + 0], fp);  // R
            fputc(pixels[i * 4 + 1], fp);  // G
            fputc(pixels[i * 4 + 2], fp);  // B
        }
        fclose(fp);
        printf("✓ Image saved to %s\n", filename);
    }
    
    unsigned char* get_ptr( void ) const   { return pixels; }
    long image_size( void ) const { return x * y * 4; }

    void display_and_exit( void(*e)(void*) = NULL ) {
        CPUBitmap**   bitmap = get_bitmap_ptr();
        *bitmap = this;
        bitmapExit = e;
        // a bug in the Windows GLUT implementation prevents us from
        // passing zero arguments to glutInit()
        int c=1;
        char* dummy = "";
        glutInit( &c, &dummy );
        glutInitDisplayMode( GLUT_SINGLE | GLUT_RGBA );
        glutInitWindowSize( x, y );
        glutCreateWindow( "bitmap" );
        glutKeyboardFunc(Key);
        glutDisplayFunc(Draw);
        glutMainLoop();
    }

     // static method used for glut callbacks
    static CPUBitmap** get_bitmap_ptr( void ) {
        static CPUBitmap   *gBitmap;
        return &gBitmap;
    }

   // static method used for glut callbacks
    static void Key(unsigned char key, int x, int y) {
        switch (key) {
            case 27:
                CPUBitmap*   bitmap = *(get_bitmap_ptr());
                if (bitmap->dataBlock != NULL && bitmap->bitmapExit != NULL)
                    bitmap->bitmapExit( bitmap->dataBlock );
                exit(0);
        }
    }

    // static method used for glut callbacks
    static void Draw( void ) {
        CPUBitmap*   bitmap = *(get_bitmap_ptr());
        glClearColor( 0.0, 0.0, 0.0, 1.0 );
        glClear( GL_COLOR_BUFFER_BIT );
        glDrawPixels( bitmap->x, bitmap->y, GL_RGBA, GL_UNSIGNED_BYTE, bitmap->pixels );
        glFlush();
    }
};

#endif  // __CPU_BITMAP_H__
