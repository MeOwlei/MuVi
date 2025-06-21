#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>

float PI; 

void fft(float in[], float complex out[], size_t N)
{
    if (N<=1){
        out[0] = in[0];
        return;
    }
    size_t ret = N/2;
    float odd[ret];
    float even[ret];
    float complex T[ret], odd_out[ret], even_out[ret];
    for (size_t i = 0; i < ret; i++){
        even[i] = in[2*i];
        odd[i]  = in[2*i+1];
    }
    fft(even, even_out, ret);
    fft(odd , odd_out , ret);
    for (size_t k = 0; k < ret; k++) {
        T[k] = cexp(-I*2*PI*k/N)*odd_out[k];
    }
    for (size_t k = 0; k < ret; k++){
        out[k]      =   even_out[k] + T[k];
        out[k+ret]  =   even_out[k] - T[k];
    }
}

int main(void){
    PI = atan2f(1, 1)*4;
    size_t n = 8;
    float in[n];
    float complex out[n];
    for (size_t i; i < n ; i++) {
        float t = (float)i/n;
        in[i] = cosf(2*PI*t*2) + sinf(2*PI*t*1);
    }
    fft(in,out,n);


    for (size_t f = 0;f < n; f++){
        printf("%02zu: %.2f    %.2f\n", f, creal(out[f]), cimag(out[f]));
}
}
