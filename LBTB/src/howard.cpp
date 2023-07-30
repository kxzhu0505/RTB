#include <queue>
#include "howard.hpp"

double Howard::getRatio()
{
    return _r;
}
vector<Edge> Howard::getCycle()
{
    return _minRatioCycle;
}
tuple<double, Vertex> Howard::findRatio()
{
    vector<size_t> visited(_n+1, -1);
    double r = _r;
    Vertex handle = boost::graph_traits<Graph>::null_vertex();
    VertexIterator vi, viend;
    for (tie(vi, viend) = boost::vertices(_g); vi != viend; ++vi)
    {
        if (visited[_g[*vi].nodeIndex] != -1)
            continue;
        Vertex u = *vi;
        do
        {
            visited[_g[u].nodeIndex] = _g[*vi].nodeIndex;
            Edge e = _su[_g[u].nodeIndex];
            u = boost::target(e, _g);
        } while (visited[_g[u].nodeIndex] == -1);
        if (visited[_g[u].nodeIndex] != _g[*vi].nodeIndex)
            continue;
        Vertex x = u;
        double sum = 0, len = 0;
        do
        {
            Edge e = _su[_g[x].nodeIndex];
            sum += _g[e].weight;
            len += _g[e].delay;
            x = boost::target(e, _g);
        } while (x != u);
        if (r > sum / len)
        {
            r = sum / len;
            handle = u;
        }
    }
    return {r,handle};
}
void Howard::run()
{
    //init _su , _dist
    boost::graph_traits<Graph>::edge_iterator ei, eiend;
    for (tie(ei, eiend) = boost::edges(_g); ei != eiend; ++ei)
    {
        size_t i = _g[source(*ei, _g)].nodeIndex;
        if (_g[*ei].weight - _r * _g[*ei].delay < _dist[i])
        {
            _dist[i] = _g[*ei].weight - _r * _g[*ei].delay;
            _su[i] = *ei;
        }
    }
    bool changed = true;
    Vertex handle;
    while (changed)
    {
        tie(_r, handle) = findRatio();
        if (handle != boost::graph_traits<Graph>::null_vertex())
        {
            queue<Vertex> que;
            InEdgeIterator inei, ineiend;
            for (tie(inei, ineiend) = boost::in_edges(handle, _g); inei != ineiend; ++inei)
            {
                Vertex source = boost::source(*inei, _g);
                if (_su[_g[source].nodeIndex] == *inei)
                    que.push(source);
            }
            vector<bool> visited(_n, false);
            while (!que.empty())
            {
                Vertex u = que.front();
                que.pop();
                size_t i = _g[u].nodeIndex;
                visited[i] = true;
                Edge e = _su[i];
                Vertex v = boost::target(e, _g);
                _dist[i] = _dist[_g[v].nodeIndex] + _g[e].weight - _r * _g[e].delay;
                for (tie(inei, ineiend) = boost::in_edges(u, _g); inei != ineiend; ++inei)
                {
                    Vertex source = boost::source(*inei, _g);
                    if (!visited[_g[source].nodeIndex] && _su[_g[source].nodeIndex] == *inei)
                        que.push(source);
                }
            }
            //save the cycle
            if (_minRatioCycle.size() != 0)
                _minRatioCycle.clear();
            Vertex u = handle;
            do
            {
                Edge e = _su[_g[u].nodeIndex];
                u = boost::target(e, _g);
            } while (u != handle);
        }
        changed = false;
        for (tie(ei, eiend) = boost::edges(_g); ei != eiend; ++ei)
        {
            size_t i = _g[boost::source(*ei, _g)].nodeIndex,
                   j = _g[boost::target(*ei, _g)].nodeIndex;
            if (_dist[i] > _dist[j] + _g[*ei].weight - _g[*ei].delay * _r + _eps)
            {
                _dist[i] = _dist[j] + _g[*ei].weight - _g[*ei].delay * _r;
                _su[i] = *ei;
                changed = true;
            }
        }
    }
}
