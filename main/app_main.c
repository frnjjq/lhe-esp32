#include <stdint.h>
#include <sys/time.h>

#include "esp_log.h"

#include "include/quantizer.h"
#include "include/entropic_enc.h"
#include "include/lena.h"
#define IMG_WIDTH 512

//uint8_t img_data[120] = {0};
double time_diff(struct timeval x , struct timeval y)
{
    double x_ms , y_ms , diff;
     
    x_ms = (double)x.tv_sec*1000000 + (double)x.tv_usec;
    y_ms = (double)y.tv_sec*1000000 + (double)y.tv_usec;
     
    diff = (double)y_ms - (double)x_ms;
     
    return diff;
}

void app_main()
{
    uint8_t buffer[IMG_WIDTH] = {0};
    uint8_t result[IMG_WIDTH] = {0};
    uint8_t hops[IMG_WIDTH] = {0};
    int size;
    struct timeval before, after;
    init_cache();

    gettimeofday(&before, NULL);

    for (int i = 0; i<512; i++)
    {
        quantize_scanline(img_data + 512* IMG_WIDTH,IMG_WIDTH, hops, result);
        size = entropic_enc(hops, buffer, IMG_WIDTH);
    }
    gettimeofday(&after, NULL);

    ESP_LOGI("main","Total time elapsed : %.0lf us", time_diff(before , after));
}


