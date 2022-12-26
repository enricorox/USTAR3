//
// Created by enrico on 24/12/22.
//

#include <valarray>
#include "algos.h"

uint32_t d(uint32_t a, uint32_t b){
    return abs((int)a - (int)b);
}

uint32_t median(vector<uint32_t> v){
    size_t n = v.size() / 2;
    // find the middle element
    nth_element(v.begin(), v.begin()+n, v.end());
    return v[n];
}