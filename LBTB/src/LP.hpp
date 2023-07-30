#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include "timing_graph.hpp"

using namespace chrono;
/**
 * @brief Linear programing for mcr using GLPK
 * 
 * @param fileName 
 */
nanoseconds mcrLP(const string &fileName,double &tlp);
/**
 * @brief Linear programing for ell using GLPK
 * 
 * @param fileName 
 */
nanoseconds ellLP(const string &fileName,double &tlp,double &wlp);
