#include "YTO.hpp"
#include <queue>

using namespace boost;

double YTO::getRatio()
{
    return _r;
}
double YTO::findArcKey(const Edge &e)
{
    size_t u = _g[source(e, _g)].nodeIndex,
           v = _g[target(e, _g)].nodeIndex;
    double delta_t = _transit[u] + _g[e].delay - _transit[v];
    if (delta_t > 0)
    {
        double delta_d = _dist[u] + _g[e].weight - _dist[v];
        return delta_d / delta_t;
    }
    else
        return numeric_limits<double>::infinity();
}

bool YTO::inSubTree(size_t v, size_t u)
{
    boost::graph_traits<ShortestPathTree>::in_edge_iterator inei, ineiend;
    if (u == v)
        return true;
    tie(inei, ineiend) = boost::in_edges(u, _Tp);
    while (inei != ineiend)
    {
        SPTVertex v_Spt = boost::source(*inei, _Tp);
        if (v_Spt == v)
            return true;
        tie(inei, ineiend) = boost::in_edges(v_Spt, _Tp);
    }
    return false;
}

void YTO::run()
{
    EdgeIterator ei, eiend;
    VertexIterator vi, viend;

    _pq.insert(numeric_limits<double>::infinity(), Vertex());
    //Initialize nodekey,arckey
    for (tie(ei, eiend) = boost::edges(_g); ei != eiend; ++ei)
    {
        double ak = findArcKey(*ei);
        size_t v = _g[boost::target(*ei, _g)].nodeIndex;
        if (ak < _arcKey[v])
        {
            _nodeKey[v] = *ei;
            _arcKey[v] = ak;
        }
    }
    //Initialize priority queue
    for (tie(vi, viend) = boost::vertices(_g); vi != viend; ++vi)
    {
        _items[_g[*vi].nodeIndex] = _pq.insert(_arcKey[_g[*vi].nodeIndex], *vi);
    }
    // vector<Vertex> vts(_n);
    // size_t cnt = 0;
    // for (tie(vi, viend) = vertices(_g); vi != viend; ++vi)
    // {
    //     vts[cnt++] = *vi;
    // }
    pq_item item;
    size_t u, v;
    Edge e;
    while (true)
    {
        item = _pq.find_min();
        _r = _pq.prio(item);
        if (_r == numeric_limits<double>::infinity())
            return;
        v = _g[_pq.inf(item)].nodeIndex;
        e = _nodeKey[v];
        u = _g[boost::source(e, _g)].nodeIndex;
        if (inSubTree(v, u))
            return;
        //Update shortest path tree rooted by v
        double delta_t = _transit[u] + _g[e].delay - _transit[v];
        double delta_d = _dist[u] + _g[e].weight - _dist[v];
        queue<SPTVertex> nodes_in_Tpv;
        nodes_in_Tpv.push(v);
        while (!nodes_in_Tpv.empty())
        {
            SPTVertex v_Spt = nodes_in_Tpv.front();
            nodes_in_Tpv.pop();
            _dist[v_Spt] += delta_d;
            _transit[v_Spt] += delta_t;
            boost::graph_traits<ShortestPathTree>::out_edge_iterator oei, oeiend;
            for (tie(oei, oeiend) = boost::out_edges(v_Spt, _Tp); oei != oeiend; ++oei)
            {
                nodes_in_Tpv.push(target(*oei, _Tp));
            }
        }
        boost::graph_traits<ShortestPathTree>::in_edge_iterator ineispt, ineiendspt;
        tie(ineispt, ineiendspt) = boost::in_edges(v, _Tp);
        if (ineispt != ineiendspt)
            boost::remove_edge(boost::source(*ineispt, _Tp), v, _Tp);
        boost::add_edge(u, v, _Tp);

        // update the arc keys of the edges entering the subtree rooted at v and those related node keys
        // note that the arc keys need to be re-calculated because of the changed of shortest path tree.
        nodes_in_Tpv.push(v);
        while (!nodes_in_Tpv.empty())
        {
            SPTVertex v_Spt = nodes_in_Tpv.front();
            nodes_in_Tpv.pop();
            InEdgeIterator inei, ineiend;
            _arcKey[v_Spt] = numeric_limits<double>::infinity();
            double ak = numeric_limits<double>::infinity();
            for (tie(inei, ineiend) = boost::in_edges(v_Spt, _g); inei != ineiend; ++inei)
            {
                ak = findArcKey(*inei);
                if (ak < _arcKey[v_Spt])
                {
                    _arcKey[v_Spt] = ak;
                    _nodeKey[v_Spt] = *inei;
                }
            }
            _pq.del_item(_items[v_Spt]);
            _items[v_Spt] = _pq.insert(_arcKey[v_Spt],v_Spt);
            boost::graph_traits<ShortestPathTree>::out_edge_iterator oeispt, oeiendspt;
            for (tie(oeispt, oeiendspt) = out_edges(v_Spt, _Tp); oeispt != oeiendspt; ++oeispt)
                nodes_in_Tpv.push(boost::target(*oeispt, _Tp));
        }

        // update the arc keys of the edges leaving the subtree rooted at v and those related node keys
        nodes_in_Tpv.push(v);
        while (!nodes_in_Tpv.empty())
        {
            SPTVertex v_Spt = nodes_in_Tpv.front();
            nodes_in_Tpv.pop();
            OutEdgeIterator oei, oeiend;
            for (tie(oei, oeiend) = boost::out_edges(v_Spt, _g); oei != oeiend; ++oei)
            {
                double ak = findArcKey(*oei);
                size_t y = _g[boost::target(*oei, _g)].nodeIndex;
                if (ak < _arcKey[y])
                {
                    _arcKey[y] = ak;
                    _nodeKey[y] = *oei;
                    _pq.decrease_p(_items[y],ak);
                }
            }
            boost::graph_traits<ShortestPathTree>::out_edge_iterator oeispt, oeiendspt;
            for (tie(oeispt, oeiendspt) = out_edges(v_Spt, _Tp); oeispt != oeiendspt; ++oeispt)
                nodes_in_Tpv.push(boost::target(*oeispt, _Tp));
        }
    }
}