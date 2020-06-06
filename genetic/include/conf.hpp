#ifndef CONF_H
#define CONF_H  

#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <utility>
#include <random>
#include <chrono> 

/*****************
 * LOG
 * **************/
#define PRINT_LOG false

/*****************
 * STATISTICS
 * **************/
#define PRINT_STATS true

/*****************
 * COLORS
 * **************/
#define DEFAULT                 0
#define INITIALIZATION          1
#define FORK_JOIN_OPS           2
#define REDUCE_OPS              3
#define STEP_COMPUTATION        4
#define EVEN_STEP_COMPUTATION   5
#define ODD_STEP_COMPUTATION    6
#define DONE                    7

template<int event> struct color_event{
    static const constexpr char* value = "<useless>";
};
template<> struct color_event<INITIALIZATION>{
    static const constexpr char* value = "orange";
};
template<> struct color_event<FORK_JOIN_OPS>{
    static const constexpr char* value = "red";
};
template<> struct color_event<REDUCE_OPS>{
    static const constexpr char* value = "green";
};
template<> struct color_event<STEP_COMPUTATION>{
    static const constexpr char* value = "blue";
};
template<> struct color_event<EVEN_STEP_COMPUTATION> : color_event<STEP_COMPUTATION> { };
template<> struct color_event<ODD_STEP_COMPUTATION>{
    static const constexpr char* value = "RoyalBlue";
};

#endif // CONF_H