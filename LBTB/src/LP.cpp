#include <glpk.h>
#include <cstdio>
#include <iostream>

#include "LP.hpp"

nanoseconds mcrLP(const string &fileName, double &tlp)
{
    auto change = high_resolution_clock::now() - high_resolution_clock::now();
    DelayGraph dg;
    double tInitial = 0;
    string fe = extension(fileName);
    if (fe == "out")
        assert(getDelayGraph(fileName, dg, tInitial));
    else if (fe == "rslt")
        assert(getRsltDelayGraph(fileName, dg, tInitial));
    //cout << "delay graph:" << endl;
    //cout << "#vertices = " << num_vertices(dg) << ", #edges = " << num_edges(dg) << endl;
    auto _v = boost::num_vertices(dg);
    auto _c = boost::num_edges(dg);
    int numvar = 4 * _v + 1;
    int numcon = 10 * _c;
    int numcof = 21 * _c; //nums of cof
    boost::graph_traits<DelayGraph>::edge_iterator eibegin, eiend;

    glp_prob *lp;
    int *ia = new int[1 + numcof];
    int *ja = new int[1 + numcof]; //start from 1
    double *ar = new double[1 + numcof];
    lp = glp_create_prob();
    glp_set_prob_name(lp, "timing");
    glp_set_obj_dir(lp, GLP_MIN);
    glp_add_rows(lp, numcon);
    glp_add_cols(lp, numvar);
    glp_set_col_bnds(lp, 1, GLP_LO, 0.0, 0.0);
    for (int i = 2; i <= numvar; i++)
    {
        //glp_set_col_name(lp, i);
        glp_set_col_bnds(lp, i, GLP_FR, 0.0, 0.0);
    }
    int row = 1; //rth constraint
    int cn = 1;  //the index of subi/subj

    for (tie(eibegin, eiend) = boost::edges(dg); eibegin != eiend && cn <= numcof && row <= numcon; ++eibegin)
    {
        int i = 4 * dg[boost::source(*eibegin, dg)].idx + 2; //source node 分裂后第一个节点存储位置 （a A d D）注意图中节点从0开始编号
        int j = 4 * dg[boost::target(*eibegin, dg)].idx + 2; //target node 分裂后第一个节点存储位置
        double Dmin = dg[*eibegin].minDelay;
        double Dmax = dg[*eibegin].maxDelay;
        //-af<=0
        ia[cn] = row;
        ja[cn] = j;
        ar[cn++] = -1.;
        glp_set_row_bnds(lp, row, GLP_UP, 0.0, 0.0);
        ++row;
        //-T+Af<=0
        ia[cn] = row;
        ja[cn] = 1;
        ar[cn++] = -1.;
        ia[cn] = row;
        ja[cn] = j + 1;
        ar[cn++] = 1.;
        glp_set_row_bnds(lp, row, GLP_UP, 0.0, 0.0);
        ++row;
        //ai-di<=0
        ia[cn] = row;
        ja[cn] = i;
        ar[cn++] = 1.;
        ia[cn] = row;
        ja[cn] = i + 2;
        ar[cn++] = -1.;
        glp_set_row_bnds(lp, row, GLP_UP, 0.0, 0.0);
        ++row;
        //T/2-di<=0
        ia[cn] = row;
        ja[cn] = 1;
        ar[cn++] = 0.5;
        ia[cn] = row;
        ja[cn] = i + 2;
        ar[cn++] = -1.;
        glp_set_row_bnds(lp, row, GLP_UP, 0.0, 0.0);
        ++row;
        //Ai-Di<=0
        ia[cn] = row;
        ja[cn] = i + 1;
        ar[cn++] = 1.;
        ia[cn] = row;
        ja[cn] = i + 3;
        ar[cn++] = -1.;
        glp_set_row_bnds(lp, row, GLP_UP, 0.0, 0.0);
        ++row;
        //T/2-Di<=0
        ia[cn] = row;
        ja[cn] = 1;
        ar[cn++] = 0.5;
        ia[cn] = row;
        ja[cn] = i + 3;
        ar[cn++] = -1.;
        glp_set_row_bnds(lp, row, GLP_UP, 0.0, 0.0);
        ++row;
        //T+af-di<=Dmin
        ia[cn] = row;
        ja[cn] = 1;
        ar[cn++] = 1.;
        ia[cn] = row;
        ja[cn] = j;
        ar[cn++] = 1.;
        ia[cn] = row;
        ja[cn] = i + 2;
        ar[cn++] = -1.;
        glp_set_row_bnds(lp, row, GLP_UP, 0.0, Dmin);
        ++row;
        //-T+Di-Af<=-Dmax
        ia[cn] = row;
        ja[cn] = 1;
        ar[cn++] = -1.;
        ia[cn] = row;
        ja[cn] = i + 3;
        ar[cn++] = 1.;
        ia[cn] = row;
        ja[cn] = j + 1;
        ar[cn++] = -1.;
        glp_set_row_bnds(lp, row, GLP_UP, 0.0, -Dmax);
        ++row;
        //af-Af<=0
        ia[cn] = row;
        ja[cn] = j;
        ar[cn++] = 1.;
        ia[cn] = row;
        ja[cn] = j + 1;
        ar[cn++] = -1.;
        glp_set_row_bnds(lp, row, GLP_UP, 0.0, 0.0);
        ++row;
        //df-Df<=0
        ia[cn] = row;
        ja[cn] = j + 2;
        ar[cn++] = 1.;
        ia[cn] = row;
        ja[cn] = j + 3;
        ar[cn++] = -1.;
        glp_set_row_bnds(lp, row, GLP_UP, 0.0, 0.0);
        ++row;
    }
    glp_set_obj_coef(lp, 1, 1.0);
    // for(int i=1;i<numcof;i++)
    // 	cout<<ia[i]<<" "<<ja[i]<<" "<<ar[i]<<endl;
    glp_load_matrix(lp, numcof, ia, ja, ar);
    auto beginTime = high_resolution_clock::now();
    glp_simplex(lp, NULL);
    auto endTime = high_resolution_clock::now();
    change = endTime - beginTime + change;

    tlp = glp_get_obj_val(lp);
    // cout << "z=" << z << endl;
    glp_delete_prob(lp);
    delete[] ar;
    delete[] ia;
    delete[] ja;
    return duration_cast<nanoseconds>(change);
}
nanoseconds ellLP(const string &fileName, double &tlp, double &wlp)
{
    auto change = high_resolution_clock::now() - high_resolution_clock::now();
    DelayGraph dg;
    double tInitial = 0;
    string fe = extension(fileName);
    if (fe == "out")
        assert(getDelayGraph(fileName, dg, tInitial));
    else if (fe == "rslt")
        assert(getRsltDelayGraph(fileName, dg, tInitial));
    auto _v = boost::num_vertices(dg);
    auto _c = boost::num_edges(dg);
    int numvar = _v + 2;
    int numcon = 3 * _c;
    int numcof = 0; //nums of cof
    boost::graph_traits<DelayGraph>::edge_iterator eibegin, eiend;
    for (tie(eibegin, eiend) = boost::edges(dg); eibegin != eiend; ++eibegin)
    {
        if (boost::source(*eibegin, dg) == boost::target(*eibegin, dg))
            numcof += 4;
        else
            numcof += 10;
    }

    glp_prob *lp;
    int *ia = new int[1 + numcof];
    int *ja = new int[1 + numcof]; //start from 1
    double *ar = new double[1 + numcof];
    double z;
    lp = glp_create_prob();
    glp_set_prob_name(lp, "timing");
    glp_set_obj_dir(lp, GLP_MIN);
    glp_add_rows(lp, numcon);
    glp_add_cols(lp, numvar);
    glp_set_col_bnds(lp, 1, GLP_LO, 0.0, 0.0);
    glp_set_col_bnds(lp, 2, GLP_LO, 0.0, 0.0);
    for (int i = 3; i <= numvar; i++)
    {
        //glp_set_col_name(lp, i);
        glp_set_col_bnds(lp, i, GLP_FR, 0.0, 0.0);
    }
    int row = 1; //rth constraint
    int cn = 1;  //the index of subi/subj

    for (tie(eibegin, eiend) = boost::edges(dg); eibegin != eiend && cn <= numcof && row <= numcon; ++eibegin)
    {
        int i = boost::source(*eibegin, dg) + 3;
        int j = boost::target(*eibegin, dg) + 3;
        double Dmin = dg[*eibegin].minDelay;
        double Dmax = dg[*eibegin].maxDelay;
        if (i == j)
        {
            //-T<=-Dmax
            ia[cn] = row;
            ja[cn] = 1;
            ar[cn++] = -1.;
            glp_set_row_bnds(lp, row, GLP_UP, 0.0, -Dmax);
            ++row;
            //-T-W<=-Dmax
            ia[cn] = row;
            ja[cn] = 1;
            ar[cn++] = -1.;
            ia[cn] = row;
            ja[cn] = 2;
            ar[cn++] = -1.;
            glp_set_row_bnds(lp, row, GLP_UP, 0.0, -Dmax);
            ++row;
            //W<=Dmin
            ia[cn] = row;
            ja[cn] = 2;
            ar[cn++] = 1.;
            glp_set_row_bnds(lp, row, GLP_UP, 0.0, Dmin);
            ++row;
        }
        else
        {
            //-T+ti-tj<=-Dmax
            ia[cn] = row;
            ja[cn] = 1;
            ar[cn++] = -1.;
            ia[cn] = row;
            ja[cn] = i;
            ar[cn++] = 1.;
            ia[cn] = row;
            ja[cn] = j;
            ar[cn++] = -1.;
            glp_set_row_bnds(lp, row, GLP_UP, 0.0, -Dmax);
            ++row;
            //-T-W+ti-tj<=-Dmax
            ia[cn] = row;
            ja[cn] = 1;
            ar[cn++] = -1.;
            ia[cn] = row;
            ja[cn] = 2;
            ar[cn++] = -1.;
            ia[cn] = row;
            ja[cn] = i;
            ar[cn++] = 1.;
            ia[cn] = row;
            ja[cn] = j;
            ar[cn++] = -1.;
            glp_set_row_bnds(lp, row, GLP_UP, 0.0, -Dmax);
            ++row;
            //W-ti+tj<=Dmin
            ia[cn] = row;
            ja[cn] = 2;
            ar[cn++] = 1.;
            ia[cn] = row;
            ja[cn] = i;
            ar[cn++] = -1.;
            ia[cn] = row;
            ja[cn] = j;
            ar[cn++] = 1.;
            glp_set_row_bnds(lp, row, GLP_UP, 0.0, Dmin);
            ++row;
        }
    }
    glp_set_obj_coef(lp, 1, 1.0);
    glp_set_obj_coef(lp, 2, 1.0);
    // for(int i=1;i<numcof;i++)
    // 	cout<<ia[i]<<" "<<ja[i]<<" "<<ar[i]<<endl;
    glp_load_matrix(lp, numcof, ia, ja, ar);
    auto beginTime = high_resolution_clock::now();
    glp_simplex(lp, NULL);
    auto endTime = high_resolution_clock::now();
    change = endTime - beginTime + change;

    z = glp_get_obj_val(lp);
    tlp = glp_get_col_prim(lp, 1);
    wlp = glp_get_col_prim(lp, 2);

    glp_delete_prob(lp);
    delete[] ar;
    delete[] ia;
    delete[] ja;
    return duration_cast<nanoseconds>(change);
}
