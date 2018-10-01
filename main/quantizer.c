#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "include/quantizer.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

unsigned char cache_hops[256][7][3];

void init_cache()
{
    for (int hop0 = 0; hop0 <= 255; hop0++)
    {
        for (int hop1 = 4; hop1 <= 10; hop1++)
        {
            float maxr = 2.7f;
            float minr = 1.0f;
            const float range = 0.70f;
            double rpos = min(maxr, pow(range * ((255 - hop0) / hop1), 1.0f / 3.0f));
            rpos = max(minr, rpos);

            double rneg = min(maxr, pow(range * (hop0 / hop1), 1.0f / 3.0f));
            rneg = max(minr, rneg);

            int h = (int)(hop0 - hop1 * rneg * rneg * rneg);
            int hop_min = 1;
            int hop_max = 255 - hop_min;
            h = min(hop_max, h);
            h = max(h, hop_min);
            cache_hops[hop0][hop1 - 4][0] = (unsigned char)h;

            h = (int)(hop0 - hop1 * rneg * rneg);
            h = min(hop_max, h);
            h = max(h, hop_min);
            cache_hops[hop0][hop1 - 4][1] = (unsigned char)h;

            h = (int)(hop0 - hop1 * rneg);
            h = min(hop_max, h);
            h = max(h, hop_min);
            cache_hops[hop0][hop1 - 4][2] = (unsigned char)h;
        }
    }
}

void quantize_scanline(uint8_t *orig,int width, uint8_t *hops, uint8_t *result)
{
    const int max_h1 = 10;
    const int min_h1 = 4;
    const int start_h1 = (max_h1 + min_h1) / 2;
    int h1 = start_h1;

    bool last_small_hop = true; //last hop was small
    bool small_hop = true;      //current hop is small

    int emin = 255; //error min
    int error = 0;  //computed error

    unsigned char oc = 127;       //orig_YUV[y][0];//original color
    unsigned char hop0 = 0;       //prediction
    unsigned char quantum = oc;   //final quantum asigned value
    unsigned char hop_value = 0;  //data from cache
    unsigned char hop_number = 4; // final assigned hop

    //mejora de prediccion
    int grad = 0;

    //unsigned char prev_color=128;
    //char * result_signal=result_YUV[y];
    //char * result_hops=hops[y];
    //this bucle is for only one scanline, excluding first pixel
    //----------------------------------------------------------
    for (int x = 0; x < width;)
    {

        // --------------------- PHASE 1: PREDICTION---------------------------------------------------------

        if (x % 2 == 0)
            oc = (orig[x] + orig[x + 1]) / 2;
        else
            oc = orig[x]; //original color

        if (x == 0)
            hop0 = 127;
        else
            hop0 = quantum;

        //-------------------------PHASE 2: HOPS COMPUTATION-------------------------------

        hop0 = hop0 + grad > 255 ? 255 : hop0 + grad < 1 ? 1 : hop0 + grad;

        hop_number = 4;   // prediction corresponds with hop_number=4
        quantum = hop0;   //this is the initial predicted quantum, the value of prediction
        small_hop = true; //i supossed initially that hop will be small (3,4,5)
        emin = oc - hop0;
        if (emin < 0)
            emin = -emin; //minimum error achieved

        if (emin > h1 / 2)
        { //only enter in computation if emin>threshold

            //positive hops
            //--------------
            if (oc >= hop0)
            {
                //case hop0 (most frequent)
                //--------------------------

                if ((quantum + h1) > 255)
                    goto phase3;

                //case hop1 (frequent)
                //---------------------
                error = emin - h1;

                if (error < 0)
                    error = -error;

                if (error < emin)
                {

                    hop_number = 5;
                    emin = error;
                    quantum += h1;

                    //if (emin<4) goto phase3;
                }
                else
                    goto phase3;

                // case hops 6 to 8 (less frequent)
                // --------------------------------
                for (int i = 3; i < 6; i++)
                {
                    //cache normal
                    //hop_value=cache_hops[hop0][h1-4][i];//indexes(i) are 3 to 5

                    //cache de 5KB simetrica
                    hop_value = 255 - cache_hops[255 - hop0][h1 - 4][5 - i]; //indexes are 2 to 0

                    error = oc - hop_value;
                    if (error < 0)
                        error = -error;
                    if (error < emin)
                    {

                        hop_number = i + 3;
                        emin = error;
                        quantum = hop_value;

                        //if (emin<4) break;// go to phase 3
                    }
                    else
                        break;
                }
            }

            //negative hops
            //--------------
            else
            {

                //case hop0 (most frequent)
                //--------------------------
                if ((quantum - h1) < 0)
                    goto phase3;

                //case hop1 (frequent)
                //-------------------
                error = emin - h1;
                if (error < 0)
                    error = -error;

                if (error < emin)
                {

                    hop_number = 3;
                    emin = error;
                    quantum -= h1;
                    //if (emin<4) goto phase3;
                }
                else
                    goto phase3;

                // case hops 2 to 0 (less frequent)
                // --------------------------------
                for (int i = 2; i >= 0; i--)
                {

                    hop_value = cache_hops[hop0][h1 - 4][i]; //indexes are 2 to 0

                    //hop_value=255-cache_hops[255-hop0][h1-4][5-i];//indexes are 2 to 0

                    error = hop_value - oc;
                    if (error < 0)
                        error = -error;
                    if (error < emin)
                    {

                        hop_number = i;
                        emin = error;
                        quantum = hop_value;
                        //if (emin<4) break;// go to phase 3
                    }
                    else
                        break;
                }
            }

        } //endif emin

    //------------- PHASE 3: assignment of final quantized value --------------------------
phase3:

        result[x] = quantum;
        hops[x] = hop_number;

        //------------- PHASE 4: h1 logic  --------------------------
        if (hop_number > 5 || hop_number < 3)
        {
            small_hop = false;
        } 

        //if (small_hop==true && last_small_hop==true){
        if (small_hop * last_small_hop)
        {

            if (h1 > min_h1)
                h1--;
        }
        else
        {

            h1 = max_h1;
        }

        last_small_hop = small_hop;
        //  printf("%d,",hop_number);
        //if (h1<min_h1 || h1>max_h1) printf("fatal error %d \n", h1);

        if (hop_number == 5)
            grad = 1;
        else if (hop_number == 3)
            grad = -1;
        else if (!small_hop)
            grad = 0;
       
        if (hop_number >= 5 || hop_number <= 3)
            x++;
        else
            x = (x + 2) & ~(1);
    }

    return;
}
