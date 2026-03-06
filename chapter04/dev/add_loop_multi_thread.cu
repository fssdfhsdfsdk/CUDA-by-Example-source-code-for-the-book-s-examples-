#include "common/book.h"
#include <iostream>
#include <pthread.h>
#include <string>
#define N 10

// 1. 定义参数结构体
struct ThreadArgs {
    int * a;
    int * b;
    int * c;
    int tid;
};

void* add(void *arg) {
    ThreadArgs* data = static_cast<ThreadArgs*>(arg);

    int tid = data->tid;
    while(tid < N) {
        data->c[tid] = data->b[tid] + data->a[tid];
        tid ++;
    }
    delete data;
    return nullptr;
}




int main(void) {
    int a[N], b[N], c[N];

    const int NUM_THREADS = 2;
    pthread_t threads[NUM_THREADS]; // 线程句柄数组

    for(int i=0; i < N; i++) {
        a[i] = -i;
        b[i] = i * i;
    }



    for (long i = 0; i < NUM_THREADS; i++) {
        ThreadArgs* args = new ThreadArgs();
        args->a = a;
        args->b = b;
        args->c = c;
        args->tid = i; 
        // 3. 创建线程：句柄, 属性(通常NULL), 函数名, 参数
        int rc = pthread_create(&threads[i], NULL, add, (void*)args);
        
        if (rc) {
            std::cerr << "错误：无法创建线程," << rc << std::endl;
            exit(-1);
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        void* status;
        pthread_join(threads[i], &status);
        std::cout << "线程 " << i << " 已结束，返回码: " << (long)status << std::endl;
    }


    for(int i=0; i < N; i++) {
        printf("%d ", c[i]);
    }


    return 0;
}

