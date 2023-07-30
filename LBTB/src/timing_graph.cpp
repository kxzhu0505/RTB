#include <boost/graph/adjacency_list.hpp>
#include <fstream>
#include <map>
#include <vector>

#include "timing_graph.hpp"
#include "strOperation.hpp"

using namespace std;

void generateSimpleGraph(Graph &g)
{
    g.clear();
    vector<Vertex> vertices(3);
    for (size_t i = 0; i < 3; ++i)
    {
        NodeProperties np = {i};
        vertices[i] = add_vertex(np, g);
    }
    ArcProperties ap01 = {true, 0., 6.0};
    add_edge(vertices[0], vertices[1], ap01, g);
    ArcProperties ap10 = {false, 0., 4.0};
    add_edge(vertices[1], vertices[0], ap10, g);
    ArcProperties ap_01 = {false, 0., 3.0};
    add_edge(vertices[0], vertices[1], ap_01, g);
    ArcProperties ap_10 = {true, 0., 5.0};
    add_edge(vertices[1], vertices[0], ap_10, g);
    ArcProperties ap12 = {true, 0., 4.0};
    add_edge(vertices[1], vertices[2], ap12, g);
    ArcProperties ap21 = {false, 0., 2.0};
    add_edge(vertices[2], vertices[1], ap21, g);
}

bool getDelayGraph(const string &fileName, DelayGraph &dg, double &tInitial)
{
    // reset all the output data
    dg.clear();
    map<string, size_t> nodeIdxMap;
    tInitial = 0;
    map<string, size_t>::iterator iter = nodeIdxMap.end();
    size_t cntNodes = 0;
    int from = -1, to = -1;
    double maxDelay, minDelay;
    ifstream infile(fileName);
    if (!infile)
    {
        cout << "Error in reading file " << fileName << endl;
        return false;
    }
    string textLine;
    vector<string> words;
    // Initialize the PI/PO vertex.
    nodeIdxMap.insert(pair<string, size_t>("INPUT", cntNodes));
    nodeIdxMap.insert(pair<string, size_t>("OUTPUT", cntNodes));
    NodeIdx nidx = {cntNodes};
    boost::add_vertex(nidx, dg);
    cntNodes++;
    while (infile)
    {
        getline(infile, textLine);
        words.clear();
        splitString(textLine, " \t", words);
        if ((words.size() > 1 && words[0] == "Network") || words.size() < 4 ||
            (words[2] == "max_delay" && words[3] == "min_delay"))
            continue;
        iter = nodeIdxMap.find(words[0]);
        if (iter == nodeIdxMap.end())
        {
            nodeIdxMap.insert(pair<string, size_t>(words[0], cntNodes));
            from = cntNodes;
            NodeIdx nidx = {cntNodes};
            boost::add_vertex(nidx, dg);
            cntNodes++;
        }
        else
        {
            from = iter->second;
        }

        iter = nodeIdxMap.find(words[1]);
        if (iter == nodeIdxMap.end())
        {
            nodeIdxMap.insert(pair<string, size_t>(words[1], cntNodes));
            to = cntNodes;
            NodeIdx nidx = {cntNodes};
            boost::add_vertex(nidx, dg);
            cntNodes++;
        }
        else
        {
            to = iter->second;
        }

        maxDelay = stof(words[2]);
        minDelay = stof(words[3]);
        assert(maxDelay >= 0);
        assert(minDelay >= 0);
        boost::graph_traits<DelayGraph>::edge_descriptor e;
        bool flag = false;
        tie(e, flag) = boost::edge(from, to, dg);
        if (!flag)
        { // the edge doesn't exist
            EdgeDelay ed = {maxDelay, minDelay};
            boost::add_edge(from, to, ed, dg);
        }
        else
        {
            if (maxDelay > dg[e].maxDelay)
                dg[e].maxDelay = maxDelay;
            if (minDelay < dg[e].minDelay)
                dg[e].minDelay = minDelay;
        }
    } // end of reading file

    boost::graph_traits<DelayGraph>::edge_iterator eidg, eienddg;
    for (tie(eidg, eienddg) = boost::edges(dg); eidg != eienddg; ++eidg)
    {
        //debug
        //cout << source(*eidg, dg) << " -> " << target(*eidg, dg)<<":"<<dg[*eidg].maxDelay<<"--"<<dg[*eidg].minDelay<<endl;
        if (dg[*eidg].maxDelay > tInitial)
            tInitial = dg[*eidg].maxDelay;
    }
    return true;
}

bool getRsltDelayGraph(const string &fileName, DelayGraph &dg, double &tInitial)
{

    // reset all the output data
    dg.clear();
    tInitial = 0;
    map<string, size_t> nodeIdxMap;
    map<string, size_t>::iterator iter;
    size_t cntNodes = 0;
    int from = -1, to = -1;
    double maxDelay, minDelay;
    ifstream infile(fileName);
    if (!infile)
    {
        cout << "Error in reading file " << fileName << endl;
        return false;
    }
    string textLine;
    vector<string> words;
    string instancefrom, instanceto;
    NodeIdx nidx = {cntNodes};
    boost::add_vertex(nidx, dg);
    nodeIdxMap.insert(pair<string, size_t>("IO", cntNodes));
    cntNodes++;

    while (infile)
    {
        getline(infile, textLine);
        splitString(textLine, " \t", words);
        if (words.size() < 4)
            continue;
        if (words[0].find("DFF") == string::npos)
            instancefrom = "IO";
        else
            instancefrom = words[0];
        iter = nodeIdxMap.find(instancefrom);
        if (iter == nodeIdxMap.end())
        {
            nodeIdxMap[instancefrom] = from = cntNodes;
            NodeIdx nidx = {cntNodes};
            boost::add_vertex(nidx, dg);
            ++cntNodes;
        }
        else
        {
            from = iter->second;
        }
        if (words[1].find("DFF") == string::npos)
            instanceto = "IO";
        else
            instanceto = words[1];
        iter = nodeIdxMap.find(instanceto);
        if (iter == nodeIdxMap.end())
        {
            nodeIdxMap[instanceto] = to = cntNodes;
            NodeIdx nidx = {cntNodes};
            add_vertex(nidx, dg);
            ++cntNodes;
        }
        else
        {
            to = iter->second;
        }

        minDelay = stof(words[2]);
        maxDelay = stof(words[3]);
        assert(minDelay <= maxDelay);
        assert(maxDelay >= 0 && minDelay >= 0);
        Edge e;
        bool flag = false;
        tie(e, flag) = boost::edge(from, to, dg);
        if (!flag)
        { // the edge doesn't exist
            EdgeDelay ed = {maxDelay, minDelay};
            boost::add_edge(from, to, ed, dg);
        }
        else
        {
            if (maxDelay > dg[e].maxDelay)
                dg[e].maxDelay = maxDelay;
            if (minDelay < dg[e].minDelay)
                dg[e].minDelay = minDelay;
        }
    } // end of reading file

    boost::graph_traits<DelayGraph>::edge_iterator eidg, eienddg;
    for (tie(eidg, eienddg) = boost::edges(dg); eidg != eienddg; ++eidg)
    {
        if (dg[*eidg].maxDelay > tInitial)
            tInitial = dg[*eidg].maxDelay;
    }

    return true;
}

void getMcrGraph(const string &fileName, Graph &g, double &tInitial)
{
    DelayGraph dg;
    string fe = extension(fileName);
    if (fe == "out")
        assert(getDelayGraph(fileName, dg, tInitial));
    else if (fe == "rslt")
        assert(getRsltDelayGraph(fileName, dg, tInitial));
    // add the vertices to output grpah
    map<size_t, vector<Vertex>> toSubNode; //a node in dg will split into four node:a,A,d,D
    //reference node
    NodeProperties n_ref = {0};
    Vertex ref = boost::add_vertex(n_ref, g);
    boost::graph_traits<DelayGraph>::vertex_iterator vi, viend;
    size_t numNode = 1;
    for (tie(vi, viend) = boost::vertices(dg); vi != viend; ++vi)
    {
        size_t idx = dg[*vi].idx;
        NodeProperties np_a = {numNode};
        Vertex va = boost::add_vertex(np_a, g);
        toSubNode[idx].push_back(va);
        ++numNode;
        NodeProperties np_A = {numNode};
        Vertex vA = boost::add_vertex(np_A, g);
        toSubNode[idx].push_back(vA);
        ++numNode;
        NodeProperties np_d = {numNode};
        Vertex vd = boost::add_vertex(np_d, g);
        toSubNode[idx].push_back(vd);
        ++numNode;
        NodeProperties np_D = {numNode};
        Vertex vD = boost::add_vertex(np_D, g);
        toSubNode[idx].push_back(vD);
        ++numNode;
    }
    // add edges to output graph
    boost::graph_traits<DelayGraph>::edge_iterator eidg, eienddg;
    for (tie(eidg, eienddg) = boost::edges(dg); eidg != eienddg; ++eidg)
    {
        Vertex u = boost::source(*eidg, dg), v = boost::target(*eidg, dg);
        size_t uid = dg[u].idx, vid = dg[v].idx;
        assert((toSubNode[uid].size() == 4 && toSubNode[vid].size() == 4));
        ArcProperties va_r = {0, 0, 0};
        boost::add_edge(toSubNode[vid][0], ref, va_r, g);
        ArcProperties r_vA = {0, tInitial, 2};
        boost::add_edge(ref, toSubNode[vid][1], r_vA, g);
        ArcProperties ud_ua = {0, 0, 0};
        boost::add_edge(toSubNode[uid][2], toSubNode[uid][0], ud_ua, g);
        ArcProperties ud_ref = {0, -0.5 * tInitial, -1};
        boost::add_edge(toSubNode[uid][2], ref, ud_ref, g);
        ArcProperties uD_uA = {0, 0, 0};
        boost::add_edge(toSubNode[uid][3], toSubNode[uid][1], uD_uA, g);
        ArcProperties uD_ref = {0, -0.5 * tInitial, -1};
        boost::add_edge(toSubNode[uid][3], ref, uD_ref, g);
        ArcProperties ud_va = {0, dg[*eidg].minDelay - tInitial, -2};
        boost::add_edge(toSubNode[uid][2], toSubNode[vid][0], ud_va, g);
        ArcProperties vA_uD = {0, tInitial - dg[*eidg].maxDelay, 2};
        boost::add_edge(toSubNode[vid][1], toSubNode[uid][3], vA_uD, g);
        ArcProperties vA_va = {0, 0, 0};
        boost::add_edge(toSubNode[vid][1], toSubNode[vid][0], vA_va, g);
        ArcProperties vD_vd = {0, 0, 0};
        boost::add_edge(toSubNode[vid][3], toSubNode[vid][2], vD_vd, g);
    }
}
void getEllGraph(const string &fileName, Graph &g, double &tInitial)
{
    DelayGraph dg;
    string fe = extension(fileName);
    if (fe == "out")
        assert(getDelayGraph(fileName, dg, tInitial));
    else if (fe == "rslt")
        assert(getRsltDelayGraph(fileName, dg, tInitial));
    size_t nVertices = boost::num_vertices(dg);
    vector<Vertex> vertices(nVertices);
    for (size_t i = 0; i < nVertices; ++i)
    {
        NodeProperties np = {i};
        vertices[i] = boost::add_vertex(np, g);
    }
    // add edges to output graph
    boost::graph_traits<DelayGraph>::edge_iterator eidg, eienddg;
    for (tie(eidg, eienddg) = boost::edges(dg); eidg != eienddg; ++eidg)
    {
        Vertex u = vertices[boost::source(*eidg, dg)], v = vertices[boost::target(*eidg, dg)];
        ArcProperties apSetup1 = {0, tInitial - dg[*eidg].maxDelay, dg[*eidg].maxDelay};
        boost::add_edge(v, u, apSetup1, g);
        ArcProperties apSetup2 = {1, tInitial - dg[*eidg].maxDelay, dg[*eidg].maxDelay};
        boost::add_edge(v, u, apSetup2, g);
        ArcProperties apHold = {2, dg[*eidg].minDelay, dg[*eidg].minDelay};
        boost::add_edge(u, v, apHold, g);
    }
}
