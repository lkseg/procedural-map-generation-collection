#include "astar.h"


void Astar::resize_example(isize s) {
    nodes.resize(s);
    ForRange(i, 0, nodes.size()) {
        nodes[i] = Astar_Node{.uid = iastar(i), .adj = {}, .point = {i, 0}};
    }
}

void connect(Astar &self, iastar a, iastar b) {
    self.nodes[a].adj.push_back(&self.nodes[b]);
}