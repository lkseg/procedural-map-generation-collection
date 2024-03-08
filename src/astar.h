#pragma once
#include "util.h"
#include "linalg.h"
typedef i32 iastar;

struct Astar_Node {
    iastar uid = -1;
    Array<Astar_Node *> adj = {};
    Vec2 point;
    bool disabled = false;
};

struct Astar {
    static const iastar INVALID_INDEX = -1;
    Array<Astar_Node> nodes; 
    void resize_example(isize);
    Astar &operator=(Astar &&) = default;
    Astar &operator=(Astar &) = delete;
};
void connect(Astar &, iastar, iastar);

inline f32 heuristic(Vec2 a, Vec2 b) {    
    return distance(a, b);
}

template<bool allow_disabled_end = false>
Array<iastar> get_path(Astar &self, iastar istart, iastar iend, bool include_start = false) {
    assert(self.nodes.size() > 0, "astar not initialized");
    if(istart == iend) {
        return {};
    }
    Array<f32> costs(self.nodes.size());
    Array<f32> total(self.nodes.size());
    Array<iastar> from(self.nodes.size());
    

    Ordered_Set<Pair<f32, iastar>> queue;

    ForRange(i, 0, costs.size()) {
        from[i] = self.INVALID_INDEX;
        total[i] = I32_MAX;
        costs[i] = I32_MAX;
    }
    costs[istart] = 0;
    total[istart] = heuristic(self.nodes[istart].point, self.nodes[iend].point);
    queue.insert({total[istart], istart});    

    while(queue.size() > 0) {
        auto ia = queue.begin()->second;

        if(ia == iend) break;
        // no path exists
        if(ia != istart && from[ia] == self.INVALID_INDEX) {
            return {};
        }        
        queue.erase(queue.begin());
        auto &a = self.nodes[ia];
        for(auto &b: a.adj) {
            if(b->disabled) continue;            
            auto ib = b->uid;
            
            f32 cost = costs[ia] + distance(a.point, b->point);
            if(cost >= costs[ib]) continue;
            costs[ib] = cost;            
            from[ib] = ia;

            auto it = queue.find({total[ib], ib});            
            total[ib] = cost + heuristic(b->point, self.nodes[iend].point);
            if(it != queue.end()) {            
            // println("delete exisiting ", start, " -> ", end);
                queue.erase(it);
            }        
            queue.insert({total[ib], ib});   
        }
    }
    
    if(from[iend] == self.INVALID_INDEX) return {};

    Array<iastar> path;
    for(int i = iend; i != istart; i = from[i]) {
        path.push_back(i);
    }
    if(include_start) {
        path.push_back(istart);
    }
    return path;
}


