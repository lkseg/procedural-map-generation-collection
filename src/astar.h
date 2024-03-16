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

Array<iastar> get_path(Astar &self, iastar istart, iastar iend, bool include_start = false);




