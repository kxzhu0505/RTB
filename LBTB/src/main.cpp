#include <limits>
#include <tuple>
#include <map>
#include <iostream>
#include <chrono>
#include <string>
//#include "ell.hpp"
#include "cutting_plane.hpp"
#include "timing_graph.hpp"
#include "timing_oracle.hpp"
#include "howard.hpp"
#include "YTO.hpp"
#include "LP.hpp"
using namespace std;
using namespace chrono;
using Arr = xt::xarray<double, xt::layout_type::row_major>;
double a = 1.0;
int main(int argc, char *const argv[])
{
     auto change = high_resolution_clock::now() - high_resolution_clock::now();
     string filename(argv[1]);
     cout << filename << endl;
     int iter = atoi(argv[2]);
     double alpha = atof(argv[3]);
     Graph g_mcr;
     Graph g_ell;
     double tInitial;
     getMcrGraph(filename, g_mcr, tInitial);
     getEllGraph(filename, g_ell, tInitial);
     nanoseconds time_howard(0), time_yto(0), time_lp_mcr(0), time_ell(0), time_lp_ell(0);

     double t_lp_mcr, t_lp_ell, w_lp_ell;
     double t_howard, t_yto, t_ell, w_ell;
     for (int iq = 0; iq < iter; iq++)
     {
          //solve mcr by howard
          Howard how(g_mcr, boost::num_vertices(g_mcr), tInitial, 1e-10);
          auto beginTime = high_resolution_clock::now();
          how.run();
          auto endTime = high_resolution_clock::now();
          time_howard += endTime - beginTime;
          t_howard = tInitial - 2 * how.getRatio();
          //solve mcr by YTO
          YTO yto(g_mcr, boost::num_vertices(g_mcr));
          beginTime = high_resolution_clock::now();
          yto.run();
          endTime = high_resolution_clock::now();
          time_yto += endTime - beginTime;
          t_yto = tInitial - 2 * yto.getRatio();
          //solve mcr by LP
          time_lp_mcr += mcrLP(filename, t_lp_mcr);
          /*
          //solve ell
          auto P = timing_oracle<Graph>(g_ell);
          auto E = ell(alpha, Arr{tInitial, 0});
          const auto options = Options{1000000, 1e-6};
          auto t = std::numeric_limits<double>::max();
          beginTime = high_resolution_clock::now();
          auto [x, ell_info] = cutting_plane_dc(P, E, t, options);
          endTime = high_resolution_clock::now();
          time_ell += endTime - beginTime;
          t_ell = x[0];
          w_ell = x[1];
          //solve ell by LP
          time_lp_ell += ellLP(filename, t_lp_ell, w_lp_ell);
          */
     }
     cout << "=====================SUMMARY====================" << endl;
     cout << "Input file is " << filename << endl;
     cout << "Initial T is " << tInitial << " ns" << endl;
     cout << "MCR:" << endl;
     cout << "\t"
          << "#vertices = " << boost::num_vertices(g_mcr) << ", #edges = " << boost::num_edges(g_mcr) << endl;
     cout << "\t"
          << "T_Howard's = " << t_howard << " ns, time is " << time_howard.count() / (1000000. * iter) << " ms" << endl;
     cout << "\t"
          << "T_YTO's = " << t_yto << " ns, time is " << time_yto.count() / (1000000. * iter) << " ms" << endl;
     cout << "\t"
          << "T_LP_MCR = " << t_lp_mcr << " ns, time is " << time_lp_mcr.count() / (1000000. * iter) << " ms" << endl;
          /*
     cout << "ELL:" << endl;
     cout << "\t"
          << "#vertices = " << boost::num_vertices(g_ell) << ", #edges = " << boost::num_edges(g_ell) << endl;
     cout << "\t"
          << "T_ELL = " << t_ell << " ns, W_ELL = " << w_ell << " ns, time is " << time_ell.count() / (1000000. * iter) << " ms" << endl;
     cout << "\t"
          << "T_LP_ELL = " << t_lp_ell << " ns, W_LP_ELL = " << w_lp_ell << " ns, time is " << time_lp_ell.count() / (1000000. * iter) << " ms" << endl;
     return 0;
     */
}
