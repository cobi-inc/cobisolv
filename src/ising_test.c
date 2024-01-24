// #include "extern.h"  // qubo header file: global variable declarations
// #include "macros.h"
// #include "ising_graph_helper.h"

#include <pigpio.h>

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <time.h>

#define WL_DELAY         0.00001
#define SCAN_CLK_DELAY   0.000001
#define NUM_OF_NODES     100
#define BITS_PER_SAMPLE  8

#define WEIGHT_2  2
#define WEIGHT_3  3
#define SCANOUT_CLK  4
#define SAMPLE_CLK  17
#define ALL_ROW_HI  27
#define WEIGHT_1  22
#define WEIGHT_EN  10
#define COL_ADDR_4  9
#define ADDR_EN64_CHIP1  11
#define COL_ADDR_3  5
#define COL_ADDR_1  6
#define COL_ADDR_0  13
#define ROW_ADDR_2  19
#define ROW_ADDR_3  26
#define WEIGHT_5  14
#define SCANOUT_DOUT64_CHIP2  15
#define SCANOUT_DOUT64_CHIP1  18
#define WEIGHT_0  23
#define ROSC_EN  24
#define COL_ADDR_5  25
#define ROW_ADDR_5  8
#define ADDR_EN64_CHIP2  7
#define WEIGHT_4  1
#define COL_ADDR_2  12
#define ROW_ADDR_1  16
#define ROW_ADDR_0  20
#define ROW_ADDR_4  21

const int ROW_ADDRS[6] = {
    ROW_ADDR_0, ROW_ADDR_1,
    ROW_ADDR_2, ROW_ADDR_3,
    ROW_ADDR_4, ROW_ADDR_5
};

const int COL_ADDRS[6] = {
    COL_ADDR_0, COL_ADDR_1,
    COL_ADDR_2, COL_ADDR_3,
    COL_ADDR_4, COL_ADDR_5
};

const int WEIGHTS[6] = {
    WEIGHT_0, WEIGHT_1,
    WEIGHT_2, WEIGHT_3,
    WEIGHT_4, WEIGHT_5
};

const int NUM_GROUPS = 59;
const int COBIFIXED65_BASEGROUPS[59] = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10,11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61];

// tmp
int Verbose_ = 1;

int all_to_all_graph_write_0[64][64] = {
    {0,0,4,3,3,3,3,0,5,7,5,0,4,7,7,3,3,7,2,5,2,7,2,3,7,3,0,7,3,4,5,3,0,0,3,3,3,7,7,3,7,5,5,5,3,2,1,2,4,5,2,4,2,2,4,5,7,3,3,0,3,1,7,0},
    {0,7,0,0,0,0,-4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-2,1,1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-6,0,0,0,0,0,7,7},
    {0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,5,0,1,1,0,5,-1,-2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,3},
    {0,2,0,0,0,0,0,0,0,0,0,6,0,0,0,0,0,-5,0,0,0,0,0,0,0,0,0,0,0,2,-3,0,1,1,0,0,0,0,0,0,0,0,0,-3,-6,0,0,0,0,0,0,0,-5,0,0,0,0,0,3,0,7,0,0,3},
    {0,7,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,0,0,0,1,1,0,0,0,0,0,0,0,0,4,0,0,0,-4,0,0,7,0,0,0,0,0,0,0,0,0,7,0,0,0,4},
    {0,3,0,-7,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,-7,2,0,0,0,0,0,0,0,0,0,0,1,1,0,0,5,-4,0,0,0,0,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,3,0,0,3},
    {0,4,0,0,0,0,0,0,0,3,0,0,0,5,0,0,-2,0,0,0,-6,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,-3,0,0,0,0,0,0,-7,0,0,0,0,0,0,7,0,0,0,0,0,5},
    {0,7,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-5,0,0,0,-5,1,1,0,1,0,0,0,0,-3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,-7,7},
    {0,2,-3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,0,0,0,0,0,0,2,0,0,0,0,0,0,0,1,1,0,0,0,-1,0,0,0,-5,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,4},
    {0,4,0,-1,1,0,0,0,0,0,0,0,0,-6,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,4},
    {0,4,0,0,0,-6,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,0,-5,0,0,3,0,0,0,0,0,0,-3,0,0,7,0,0,0,0,0,0,0,0,0,3},
    {0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-2,0,0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,0,0,0,7,0,0,0,0,0,0,0,-5,0,0,3},
    {0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,2,0,0,7,0,0,0,0,0,0,0,0,0,0,0,2},
    {0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,-3,0,0,0,0,-3,0,0,0,0,-2,0,-2,-5,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,-3,0,0,0,-7,0,0,0,0,0,3},
    {0,2,0,0,0,0,0,0,0,-6,0,0,0,2,0,0,-5,0,-2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,-3,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,7,0,0,0,5},
    {0,3,0,0,0,2,-2,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,-5,1,1,0,0,0,0,0,0,-7,0,0,0,0,0,0,0,7,0,0,3,-2,0,0,0,0,0,0,0,0,0,0,5},
    {0,3,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,1,1,0,-2,-4,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
    {0,1,0,0,0,0,0,0,0,0,0,0,0,-3,0,0,-5,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,0,0,0,-7,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,-5,0,0,0,4},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-5,0,0,0,0,-1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5},
    {0,4,0,0,0,-6,0,3,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,4,4,0,0,1,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-7,0,0,3},
    {0,3,1,-5,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,1,0,4,0,0,0,-4,0,0,-3,0,0,5},
    {0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,5,0,0,0,4},
    {0,1,0,0,0,-4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,-5,0,0,0,0,0,0,0,1},
    {0,7,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,7,0,0,0,0,0,0,0,-7,0,0,0,0,-5,0,0,-4,0,0,0,0,0,0,7},
    {0,1,0,0,0,0,0,0,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,-2,0,0,0,0,0,0,1,1,0,0,0,6,0,7,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3},
    {0,7,0,-4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,0,-5,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
    {0,7,0,0,-6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,7,0,7,0,0,0,0,0,0,-7,0,0,-4,0,0,0,0,0,-1,0,0,-5,0,0,-2,0,7},
    {0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,2,0,0,0,-3,0,0,0,0,0,5,0,0,1,1,0,0,7,0,0,0,0,0,0,0,5,0,0,-4,0,0,0,0,0,0,0,0,0,0,6,0,0,-1,0,6},
    {0,2,0,0,-4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-5,0,0,-1,0,1,1,0,7,0,0,0,0,0,0,0,0,4,0,0,-3,0,0,0,0,0,0,0,0,1,0,0,0,0,6,0,3},
    {0,4,0,0,0,0,0,-7,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,1,1,7,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,-1,3},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,3,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,7,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-5,0,0,0,0,0,0,0,-6,0,0,0,0,0,-3,3},
    {0,3,0,0,0,0,0,0,0,0,0,0,0,7,1,0,0,0,0,0,0,0,0,0,4,-3,0,0,0,-7,7,0,1,1,0,-1,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,1,2,0,0,0,0,0,0,-3,6,0,4},
    {0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,7,-7,0,1,1,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,5},
    {0,3,0,0,0,0,0,4,0,0,1,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,7,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,-2,0,0,0,0,-5,0,0,0,0,0,0,0,0,-1,0,0,0,3},
    {0,7,0,0,0,0,0,0,0,-6,0,0,0,0,0,-5,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,1,1,0,-5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-2,0,0,0,0,0,-6,0,0,0,0,3,0,7},
    {0,7,-7,0,0,6,0,0,0,0,5,0,0,-4,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
    {0,1,0,0,0,0,0,0,-3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,-4,0,1,1,0,0,0,0,0,-3,0,0,0,1,0,0,0,0,0,0,-2,0,-3,0,0,0,0,0,0,0,0,0,0,3},
    {0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,5,0,0,0,0,0,0,0,0,7,0,0,0,0,0,4,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,3,0,0,0,0,0,0,2,0,0,0,0,0,0,0,7},
    {0,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,1,1,0,0,-4,0,0,0,0,0,0,0,0,-6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,7},
    {0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,4,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,-2,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,7},
    {0,2,0,0,0,0,0,0,0,3,0,0,0,-4,0,0,0,0,0,5,7,0,0,0,0,0,0,0,0,0,0,0,1,1,4,0,0,0,-6,0,0,0,0,0,0,0,0,0,0,0,-4,0,0,0,0,0,0,-7,-7,0,0,0,0,1},
    {0,2,1,0,0,0,0,6,-2,0,0,0,0,0,0,0,0,0,0,7,5,0,0,0,0,0,0,0,0,2,0,0,1,1,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
    {0,4,0,0,0,0,0,0,6,1,-6,0,0,0,5,-6,0,0,7,0,0,0,0,0,0,0,0,0,3,0,0,0,1,1,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,-3,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
    {0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-6,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,-6,0,0,7},
    {0,3,0,1,2,0,0,0,0,0,0,0,0,0,0,-3,7,-6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,-5,0,0,-5,0,0,0,0,0,0,0,-3,0,0,0,0,0,4},
    {0,1,0,0,0,0,0,0,0,0,0,0,-1,0,0,7,-3,0,-6,0,-1,4,0,0,5,0,0,-5,0,0,0,0,1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,-4,0,0,0,0,0,0,0,0,0,0,0,0,3},
    {0,7,0,-6,0,0,0,5,0,0,0,0,0,0,7,0,0,0,6,0,0,0,0,0,0,0,0,0,0,0,2,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
    {0,7,-4,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,-5,0,0,0,0,0,-5,0,0,0,7,0,1,1,0,0,0,0,0,5,0,0,0,0,2,0,-4,0,4,3,0,0,0,0,-6,0,0,5,0,0,0,0,0,7},
    {0,2,0,0,0,0,0,0,0,0,0,0,7,0,0,-2,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,-1,0,0,0,0,0,0,0,0,0,3},
    {0,1,0,0,0,0,0,2,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,6},
    {0,0,0,0,0,0,2,0,0,0,7,0,0,0,0,0,0,0,-7,0,0,0,0,0,0,0,5,0,2,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
    {0,7,0,-2,0,0,0,3,0,7,-1,0,0,0,0,0,0,0,1,0,3,0,0,0,0,0,0,-7,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-6,0,0,0,0,0,0,0,4,-2,0,0,0,0,7},
    {0,2,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,7,-2,0,0,0,0,0,-4,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
    {0,0,0,0,0,-2,0,7,0,4,0,3,0,0,6,0,0,0,0,6,0,0,0,-2,0,0,0,0,5,0,0,0,1,1,-7,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,6},
    {0,2,0,5,0,0,7,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,-3,0,0,0,0,0,0,0,0,0,0,0,0,0,-4,3},
    {0,1,0,0,0,7,0,-3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,0,0,0,0,0,1,1,0,0,0,0,0,0,0,-4,0,0,-6,0,0,0,3,0,0,0,0,-7,0,0,0,0,0,-1,0,0,0,2},
    {0,4,0,0,7,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,-4,0,-6,0,0,0,0,-1,-1,0,0,0,3,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,3},
    {0,2,0,7,0,0,6,0,0,-2,0,0,0,0,-7,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,-4,0,0,0,0,-6,0,0,0,0,0,0,0,0,0,0,-1,0,0,0,-7,0,0,0,0,3},
    {0,4,7,0,0,0,0,0,0,0,0,0,0,-4,0,0,-1,0,0,2,0,0,0,0,0,0,-7,0,0,0,0,0,1,1,0,0,0,0,0,0,4,0,0,1,0,0,0,0,0,0,0,0,0,0,0,-3,0,0,0,0,0,0,0,4},
    {0,0,4,4,4,2,4,7,4,7,7,6,3,7,7,4,4,7,4,4,1,7,7,7,7,4,2,7,3,5,4,3,1,1,2,4,7,7,7,3,7,5,4,5,3,7,7,2,5,5,1,3,3,3,3,4,7,5,3,4,3,4,7,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

// Misc utility functions

int usleep(long usecs)
{
   struct timespec rem;
   struct timespec req= {
       (int)(usecs / 1000000),
       (usecs % 1000000) * 1000
   };

   return nanosleep(&req , &rem);
}

int _rand_int_normalized()
{
// generate random int normalized to -7 to 7
//
    if (rand() % 2 == 1) {
        return rand() % 8;
    } else {
        return -1 * (rand() % 8);
    }
}

int **_malloc_array2d(int w, int h)
{
    int** a = malloc(sizeof(int *) * w);

    int i, j;
    for (i = 0; i < w; i++) {
        a[i] = malloc(sizeof(int) * h);
        for (j = 0; j < h; j++) {
            a[i][j] = 0;
        }
    }

    return a;
}

int **_gen_rand_array2d(int w, int h)
{
    int** a = _malloc_array2d(w, h);

    int i, j;
    for (i = 0; i < w; i++) {
        for (j = 0; j < h; j++) {
            a[i][j] = _rand_int_normalized();
        }
    }

    return a;
}

void _free_array2d(int **a, int w) {
    int i;
    for (i = 0; i < w; i++) {
        free(a[i]);
    }
    free(a);
}

void binary_splice_rev(int num, int *bin_list)
{
    // given num place lower 6 bits, in reverse order, into bin_list
    int i;
    int shift = 0;
    for (i = 0; i < 6; i++) {
        bin_list[i] = (num >> shift) & 1;
        shift++;
    }
}

// ising subproblem solver

typedef struct IsingData {
    int w;
    int h;
    int **programming_bits;
    uint8_t *chip2_test;
    /* uint8_t *samples; */
} IsingData;


IsingData *ising_data_mk()
{
    IsingData *d = malloc(sizeof(IsingData));
    d->w = 64;
    d->h = 64;
    d->programming_bits = _malloc_array2d(d->w, d->h);
    d->chip2_test = malloc(sizeof(uint8_t) * 504);
    return d;
}


int ising_gpio_setup()
{
        gpioSetMode(WEIGHT_2,    PI_OUTPUT);
        gpioSetMode(WEIGHT_3,    PI_OUTPUT);
        gpioSetMode(SCANOUT_CLK, PI_OUTPUT);
        gpioSetMode(SAMPLE_CLK,  PI_OUTPUT);
        gpioSetMode(ALL_ROW_HI,  PI_OUTPUT);
        gpioSetMode(WEIGHT_1,    PI_OUTPUT);
        gpioSetMode(WEIGHT_EN,   PI_OUTPUT);
        gpioSetMode(COL_ADDR_4,  PI_OUTPUT);
        gpioSetMode(ADDR_EN64_CHIP1, PI_OUTPUT);
        gpioSetMode(COL_ADDR_3,  PI_OUTPUT);
        gpioSetMode(COL_ADDR_1,  PI_OUTPUT);
        gpioSetMode(COL_ADDR_0,  PI_OUTPUT);
        gpioSetMode(ROW_ADDR_2,  PI_OUTPUT);
        gpioSetMode(ROW_ADDR_3,  PI_OUTPUT);
        gpioSetMode(WEIGHT_5,    PI_OUTPUT);
        gpioSetMode(SCANOUT_DOUT64_CHIP2, PI_INPUT);
        gpioSetMode(SCANOUT_DOUT64_CHIP1, PI_INPUT);
        gpioSetMode(WEIGHT_0,    PI_OUTPUT);
        gpioSetMode(ROSC_EN,     PI_OUTPUT);
        gpioSetMode(COL_ADDR_5,  PI_OUTPUT);
        gpioSetMode(ROW_ADDR_5,  PI_OUTPUT);
        gpioSetMode(ADDR_EN64_CHIP2, PI_OUTPUT);
        gpioSetMode(WEIGHT_4,    PI_OUTPUT);
        gpioSetMode(COL_ADDR_2,  PI_OUTPUT);
        gpioSetMode(ROW_ADDR_1,  PI_OUTPUT);
        gpioSetMode(ROW_ADDR_0,  PI_OUTPUT);
        gpioSetMode(ROW_ADDR_4,  PI_OUTPUT);

        gpioWrite(SAMPLE_CLK, PI_LOW);
}

void ising_weight_pins_low()
{
        gpioWrite(WEIGHT_0, PI_LOW);
        gpioWrite(WEIGHT_1, PI_LOW);
        gpioWrite(WEIGHT_2, PI_LOW);
        gpioWrite(WEIGHT_3, PI_LOW);
        gpioWrite(WEIGHT_4, PI_LOW);
        gpioWrite(WEIGHT_5, PI_LOW);
}

void ising_set_addr(const int *addrs, int *bin_num_list)
{
    int addr_name;
    int i;
    for (i = 0; i < 6; i++) {
        addr_name = addrs[i];
        if (bin_num_list[i] == 1) {
            gpioWrite(addr_name, PI_HIGH);
        } else {
            gpioWrite(addr_name, PI_LOW);
        }
    }
}

void ising_program_weights(int **programming_bits)
{
    if (Verbose_ > 0) {
        printf("Programming chip\n");
    }

    int enable_pin_name = ADDR_EN64_CHIP2;

    // initialize binary lists
    int bin_row_list[6];
    int bin_col_list[6];
    int bin_weight_list[6];

    // reset pins for programming
    ising_weight_pins_low();
    gpioWrite(ALL_ROW_HI, PI_LOW);

    // # run through each row of 64x64 cells in COBI/COBIFREEZE
    int x = 0;
    int y = 0;
    for (x = 0; x < 64; x++) { // #run through each row of 64x64 cells in COBI/COBIFREEZE
        binary_splice_rev(x, bin_row_list);
        for (y = 0; y < 64; y++) { // #run through each cell in a given row
            binary_splice_rev(y, bin_col_list);
            binary_splice_rev(programming_bits[x][y], bin_weight_list);

            ising_set_addr(ROW_ADDRS, bin_row_list); // #assign the row number
            ising_set_addr(COL_ADDRS, bin_col_list); // #assign the column number

            gpioWrite(enable_pin_name, PI_HIGH);

            // #set weight of 1 cell
            ising_set_addr(WEIGHTS, bin_weight_list); // #assign the weight corresponding to current cell
            // # time.sleep(.001) # Delay removed since COBIFIXED65 board does not have any level shifters which causes additional signal delay
            gpioWrite(enable_pin_name, PI_LOW);
            ising_weight_pins_low(); // #reset for next address
        }
    }

    if (Verbose_ > 0) {
        printf("Programming completed\n");
    }
}


/* int ising_cal_energy_ham() //# cal_energy_ham(self, size, graph_input_file) *\/ */
/* { */
/*     // TODO finish */
/*     // directly implement functionality of graph_helper.qubo_solve_file here */

/*     /\*     num_groups = len(groups) *\/ */
/*     /\* int ** arr = _malloc_array2d(64, 64); *\/ */

/*     /\* arr = import_graph(arr,infile) *\/ */
/*     /\* h,j = {},{} *\/ */

/*     /\* for x in range(num_groups): *\/ */
/*     /\*     for y in range(x+1,num_groups): *\/ */
/*     /\*         w = 0 *\/ */
/*     /\*         for x_i in groups[x]: *\/ */
/*     /\*             for y_i in groups[y]: *\/ */
/*     /\*                 w = w + arr[62-x_i,y_i+1] + arr[62-y_i,x_i+1] *\/ */
/*     /\*         if w != 0: *\/ */
/*     /\*             j[(x,y)] = -w *\/ */

/*     /\* response = QBSolv().sample_ising(h,j, timeout=180) *\/ */
/*     /\* list(response.samples()) *\/ */
/*     if(Verbose_ > 0) { */
/*         /\* print("Tabu search Hamilitonian Values (5 best values)=" + str(list(response.data_vectors['energy'][:5]))); *\/ */
/*     } */

/*     /\* minenergy = np.min(response.data_vectors['energy']) *\/ */
/*     int minenergy = 0; */
/*     return minenergy; */

/* } */

int **ising_gh_read_spins(IsingData *ising_data, int num_samples)
{
    // chip_data_len must equal 63*7

    int **sample_data = _malloc_array2d(num_samples, 63);
    int **excess_0s = _malloc_array2d(num_samples, 63);
    int **spins = _malloc_array2d(num_samples, NUM_GROUPS);

    int bit_index = 0;
    int num_index = 0;
    int sample_index = 0;
    int cur_val = 0;

    for (sample_index = 0; sample_index < num_samples; sample_index++) {
        num_index = 0;
        for (bit_index = 0; bit_index < chip_data_len; bit_index++) {
            if (chip_data[bit_index] == 0) {
                excess_0s[sample_index][num_index]++;
            } else {
                excess_0s[sample_index][num_index]--;
            }

            if (bit_index % 7 == 0 && bit_index > 0) {
                sample_data[sample_index][num_index] = cur_val;
                num_index++;

                cur_val = chip_data[bit_index];
            } else {
                cur_val = (cur_val << 1) + chip_data[bit_index];
            }
        }
    }

    int g;
    for (g = 0; g < NUM_GROUPS; g++) {
        num_index = COBIFIXED65_BASEGROUPS[g];
        for (sample_index = 0; sample_index < num_samples; sample_index++) {
            spins[sample_index][g] += excess_0s[sample_index][num_index];
        }
    }

    for (sample_index = 0; sample_index < num_samples; sample_index++) {
        for (num_index = 0; num_index < NUM_GROUPS; num_index++) {
            if (spins[sample_index][num_index] <= 0) {
                spins[sample_index][num_index] = -1;
            } else {
                spins[sample_index][num_index] = 1;
            }
        }
    }

    _free_array2d(sample_data, num_samples);
    _free_array2d(excess_0s, num_samples);
    return spins;
}

double ising_simple_descent(int *spins, int **weights)
{
    // TODO <<<HERE
}


void ising_gh_cal_energy_direct(int **spins, int num_samples, int **weights)
{
    // Should hold: num_samples == len(spins) and NUM_GROUPS == len(weights)

    int *hamiltonians = malloc(sizeof(double) * num_samples);
    double ham = 0;

    // implementing only the `descend == True` path in original code
    /* if descend: */
    int i;
    for (i = 0; i < num_samples; i++) {
        ham = ising_simple_descent(spins[i], weights);
        hamiltonians[i] = ham;
    }

    return hamiltonians;
}

void ising_gh_cal_energy(IsingData *ising_data, int num_samples)
{
    // TODO move to global BASEGROUPS in place of passing group param
    int **spins = read_spins(ising_data, num_samples);

    /* weights = np.zeros((num_groups,num_groups),dtype=np.int8) */
    int **weights = _malloc_array2d(NUM_GROUPS, NUM_GROUPS);

    /* graph_arr = np.zeros((64,64),dtype=np.int8) */
    /* graph_arr = import_graph(graph_arr,graph_file) */
    /* int **graph_arr = all_to_all_graph_write_0; */

    int x, y, i, j;
    for (x = 0; x < NUM_GROUPS; x++) {
        for (y = x + 1; y < NUM_GROUPS; y++){
            /* for i in groups[x]: */
            /*     for j in groups[y]: */
            i = COBIFIXED65_BASEGROUPS[x];
            j = COBIFIXED65_BASEGROUPS[y];

            weights[x,y] -= (all_to_all_graph_write_0[62-i,j+1] + all_to_all_graph_write_0[62-j,i+1]);
        }
    }

    /* return cal_energy_direct(spins,weights,descend,return_spins) */
    return ising_gh_cal_energy_direct(spins, num_samples, weights, num_groups);
}

/* // #cal_energy(self, sample_bits, size, sample_index, ising_data_array, sample_times, graph_problem_file,verbose=True, return_spins=False,store_time=[]): */
// ising_data_array is not used
void ising_cal_energy(IsingData *ising_data, int energy_ham)
{
    /* chip_file_name="run_files/chip2_test.txt" */
    /* all_samples_chip2 = np.zeros((504), dtype=np.int8) */ // 504 == 63 * 8 ?
    /* uint8_t *all_samples_chip2 = malloc(sizeof(uint8_t) * 504); */

    /* #if sample_index%3==0: */
    gpioWrite(ROSC_EN, PI_LOW);
    gpioWrite(ROSC_EN, PI_HIGH);

    /* #start_time = time.time() */

    // # Additional delay (~100us) to allow oscillator phases to settle
    /* for i in range (2500): */
    /*         pass */
    usleep(1);

    gpioWrite(SAMPLE_CLK, PI_HIGH);
        /* #time.sleep(0.0001) */
    usleep(1);
    gpioWrite(SAMPLE_CLK, PI_LOW);

    int bit = 0;
    for (bit = 0; bit < 441; bit++) {
        /*     # if (sample == 1) and (bit < 64): */
        /*     #     print(GPIO.input(scanout_dout64_chip1)) */
        if (gpioRead(SCANOUT_DOUT64_CHIP2) == 1) {
            ising_data->chip2_test[bit] = 1;

        } else {
            ising_data->chip2_test[bit] = 0;
        }

        if (bit == 440) {
            break;
        }

        gpioWrite(SCANOUT_CLK, PI_HIGH);
        gpioWrite(SCANOUT_CLK, PI_LOW);
    }


    /* int byteIndex = 0; */
    /* int curNum = 0; */
    /* for (bit = 0; bit < 441; bit++) { */
    /*     if (bit % 7 == 0) { */
    /*         ising_data->chip2_test[byteIndex] = curNum; */
    /*     } else { */
    /*         curNum = (curNum << 1) + all_samples_chip2[bit]; */
    /*     } */
    /* } */

    /* input_graph_array_chip1 = np.zeros((101,101), dtype=np.int8) */
    /* input_graph_array_chip1 = self.import_graph(input_graph_array_chip1, graph_problem_file) */
    /* int **input_graph_array_chip1 = all_to_all_graph_write_0; */

    ising_gh_cal_energy(ising_data, 1);

    /* ham_solution = ham_solution[0] */
    /* majority_vote_array = majority_vote_array[0] */
    /* acc = ham_solution/float((self.energy_ham))*100 */

    /* if(verbose):                                              # calculate chip accuracy */
    /*     #print("Computed Hamiltonian for sample = " + str(ham_solution)) */
    /*     print("Hamiltonian of current sample = " + str(round(acc,2)) + "%\t", end="")  # print chip accuracy */
    /*     #print("Spin Values (Ascending): " + str(majority_vote_array[0:size])+ '\n') */
    /* if not return_spins: */
    /*     return round(acc,2) */
    /* else: */
    /*     return round(acc,2), majority_vote_array */

}

void ising_modify_array_for_pins(int initial_array[64][64], int  **final_pin_array, int problem_size)
{
    int total_0_rows = 63 - problem_size;

    int y_diag = problem_size; //#set to y-location at upper right corner of problem region

    // #part 1: adjust all values within problem regions of array
    int x, y, integer_pin;
    for (x = total_0_rows; x < 63; x++) {
        for (y = 1; y < problem_size + 1; y++) {
            integer_pin = initial_array[x][y];

            if (integer_pin == -7) {
                final_pin_array[x][y] = 0b001110; // # load value of 14.0 to final_array
            } else if (integer_pin == -6) {
                final_pin_array[x][y] = 0b001100;     // # load value of 12.0 to final_array

            } else if (integer_pin == -5) {
                final_pin_array[x][y] = 0b001010;     // # load value of 10.0 to final_array

            } else if (integer_pin == -4) {
                final_pin_array[x][y] = 0b001000;     // # load value of 8.0 to final_array

            } else if (integer_pin == -3) {
                final_pin_array[x][y] = 0b000110;     // # load value of 6.0 to final_array

            } else if (integer_pin == -2) {
                final_pin_array[x][y] = 0b000100;     // # load value of 4.0 to final_array

            } else if (integer_pin == -1) {
                final_pin_array[x][y] = 0b000010;     // # load value of 2.0 to final_array

            } else if (integer_pin == 0) {
                final_pin_array[x][y] = 0b000000;     // # load value of 0.0 to final_array

            } else if (integer_pin == 1) {
                final_pin_array[x][y] = 0b000011;     // # load value of 3.0 to final_array

            } else if (integer_pin == 2) {
                final_pin_array[x][y] = 0b000101;     // # load value of 5.0 to final_array

            } else if (integer_pin == 3) {
                final_pin_array[x][y] = 0b000111;     // # load value of 7.0 to final_array

            } else if (integer_pin == 4) {
                final_pin_array[x][y] = 0b001001;     // # load value of 9.0 to final_array

            } else if (integer_pin == 5) {
                final_pin_array[x][y] = 0b001011;     // # load value of 11.0 to final_array

            } else if (integer_pin == 6) {
                final_pin_array[x][y] = 0b001101;     // # load value of 13.0 to final_array

            } else if (integer_pin == 8) {
                final_pin_array[x][y] = 0b011111;     // # load the strong positive coupling to the final_array
            } else if (integer_pin == -8) {
                final_pin_array[x][y] = 0b111110;     // # load the strong negative coupling to the final_array
            } else { // # integer_pin == 7
                // TODO rectify value in comment with actual value being assigned
                if (y == y_diag) { // #along diagonal
                    final_pin_array[x][y] = 0b001111; // # load value of 31.0 to final_array
                } else {
                    final_pin_array[x][y] = 0b001111; // # load value of 15.0 to final_array

                }
            }
        }
        y_diag--;
    }

    // #part 2 - adjust remaining 7s in diagonal
    y_diag = problem_size;
    for (x = 0; x < 64; x++) {
        for (y = 0; y < 64; y++) {
            integer_pin = initial_array[x][y];
            if (y == y_diag && integer_pin == 7) {
                final_pin_array[x][y] = 0b001111;
            }
        }
    }
    /* return final_pin_array; */
}


/*  void ising_bit_stream_generate_adj(adj,i) */
/*  { */
/* //     input_graph = graph_helper.realize_weights(graph_helper.cobifixed65_basegroups,adj) */
/* //     input_graph = graph_helper.add_shil(input_graph,4) */
/* //     input_graph = graph_helper.add_calibration(input_graph,"./calibration/cals.txt") */
/* //     create_graph.write_prog_from_matrix(input_graph,i) */
/* //     self.selected_file = "run_files/all_to_all_graph_write_%i.txt" %i */
/* //     self.programming_bits = self.modify_array_for_pins(input_graph,np.zeros((64,64),dtype=np.int8),6      */
/*  } */

// py: cobifixed65_rpi::test_multi_times
void ising_test_multi_times(IsingData *ising_data, int sample_times, int sample_bits, int size)
{
    ising_program_weights(ising_data->programming_bits);

    int times = 0;
    /* double *all_results = []; */
    double cur_best = 0;
    double res;

    /* int energy_ham = ising_cal_energy_ham(...); //# calculate Qbsolv energy once */
    int energy_ham = 0;
    /* int **ising_data_array = _malloc_array2d(400,sample_times); */

    gpioWrite(ALL_ROW_HI, PI_HIGH);
    gpioWrite(SCANOUT_CLK, PI_LOW);
    gpioWrite(WEIGHT_EN, PI_HIGH);
    gpioWrite(WEIGHT_EN, PI_LOW);

    while (times < sample_times) {
        ising_cal_energy(ising_data, energy_ham);  //# calculate H energy from chip data

            /* all_results.append(res); */
        times += 1;

        // TODO: track cur_best

        /* if (Verbose_ > 0) { */
        /*     printf(", best sample = %d %",  cur_best); */
        /* } */

    }

    gpioWrite(ALL_ROW_HI, PI_LOW);

    if (Verbose_ > 0) {
        printf("Finished!\n");
    }

    /* return all_results; */
}

/* void benchmark_adjacency(int **adj, int w, int h, int sample_count) */
/* { */
/* //     board1 = cobifixed65_rpi() */
/* //     board1.bit_stream_generate_adj(adj,0) */
/* //     graph_problem_file = "{0}{1}.txt".format("run_files/all_to_all_graph_write_", 0) */
/* //     result_tmp = board1.test_multi_times(sample_count, 7, 59, graph_problem_file, verbose=False,return_spins=True) */
/* //     return result_tmp */
/* } */


int ising_init()
{
    if (gpioInitialise() < 0) return 1;

    // setup GPIO pins
    ising_gpio_setup();
    return 0;
}

bool ising_established()
{
    // TODO
    // connection = getenv("DW_INTERNAL__CONNECTION");
    // if (connection == NULL) {
    return false;
    // }
    // return true;
}

void ising_solver(double **val, int maxNodes, int8_t *Q)
{
    // TODO
}

void ising_close()
{
    gpioTerminate();
}

int main()
{
    if (ising_init() == 1) {
        printf("Init failed\n");
        exit(1);
    }

    int size = 64;         // # problem size
    int sample_times = 20; //  # sample times
    int sample_bits = 8;    //# use 8 bits sampling


    IsingData *ising_data = ising_data_mk();

    /* int w = 59; */
    /* int h = 59; */
    /* int **adj = _gen_rand_array2d(w, h); */

    /* ising_benchmark_adjacency(ising_data, adj, w, h, 20); */

    //
    ising_modify_array_for_pins(all_to_all_graph_write_0, ising_data->programming_bits, 63);

    // # test_multi_times(sample_count, 7, 59, graph_problem_file, verbose=False,return_spins=True)
    // # (sample_times, sample_bits, size, graph_problem_file,verbose=True,return_spins =False)
    ising_test_multi_times(ising_data, sample_times, sample_bits, size);  // # store result



    // TODO display results
    int i;
    for (i = 0; i < 504; i++) {
        printf("%d", ising_data->chip2_test[i]);
    }
    printf("\n");

    ising_close();

}
