// Note this file is written in C i.e. one can rename it to .c and use it for anything.

#include "voronoi.h"
#include <stdlib.h>
#include <string.h>
#if defined(__cplusplus)
#include <cstdio>
#endif
#define JC_VORONOI_IMPLEMENTATION
#include "jc_voronoi.h"
#include "jc_voronoi_clip.h"
 
define_carray(int)


#define dassert(COND, ...) if(!(COND)) {printf("%s, %d", __FILE__, __LINE__); abort();}

static VriPoint as_point(jcv_point p) {
    VriPoint point = {p.x, p.y};
	return point;
}

// From the jc_voronoi example
static void _relax_points(const jcv_diagram* diagram, jcv_point* points) {
    const jcv_site *sites = jcv_diagram_get_sites(diagram);
    for(int i = 0; i < diagram->numsites; ++i) {
        
        const jcv_site *site = &sites[i];
        jcv_point sum = site->p;
        int count = 1;

        const jcv_graphedge *edge = site->edges;

        while(edge) {
            sum.x += edge->pos[0].x;
            sum.y += edge->pos[0].y;
            ++count;
            edge = edge->next;
        }

        points[site->index].x = sum.x / (jcv_real)count;
        points[site->index].y = sum.y / (jcv_real)count;
    }
}

RawGraph make_raw_voronoi_random_points(int width, int height, int count, VriPoint *points, int num_relax, int seed) {
    if(count == 0) return (RawGraph){};
    srand(seed);

    // jcv_point *points = (jcv_point *)malloc(sizeof(jcv_point)*count);
    
    int pointoffset = 10;

    for(int i = 0; i < count; ++i) {
        points[i].x = (float)(pointoffset + rand() % (width-2*pointoffset));
        points[i].y = (float)(pointoffset + rand() % (height-2*pointoffset));
    }    
    return make_raw_voronoi_from_points(count, num_relax, points);
}

RawGraph make_raw_voronoi_random(int width, int height, int count, int num_relax, int seed) {
    if(count == 0) return (RawGraph){};

    jcv_point *points = (jcv_point *)malloc(sizeof(jcv_point)*count);
    return make_raw_voronoi_random_points(width, height, count, points, num_relax, seed);
}


VriGraph make_voronoi_random(int width, int height, int count, int num_relax, int seed) {
    RawGraph r = make_raw_voronoi_random(width, height, count, num_relax, seed);
    VriGraph g = make_voronoi_graph(&r);    
    g.width = width;
    g.height = height;    
    free_raw_graph(&r);
    return g;
}
VriGraph make_voronoi_random_points(int width, int height, int count, VriPoint *points, int num_relax, int seed) {
    RawGraph r = make_raw_voronoi_random_points(width, height, count, points, num_relax, seed);
    VriGraph g = make_voronoi_graph(&r);    
    g.width = width;
    g.height = height;    
    free_raw_graph(&r);
    return g;
}


VriGraph make_voronoi_from_points(int count, int num_relax, VriPoint *points) {
    RawGraph r = make_raw_voronoi_from_points(count, num_relax, points);
    VriGraph g = make_voronoi_graph(&r);    
    soft_free_raw_graph(&r);
    return g;    
}
RawGraph make_raw_voronoi_from_points(int count, int num_relax, VriPoint *points) {
    // jcv_clipping_polygon polygon;
    // polygon.num_points = 0;
    // polygon.points = 0;

    jcv_clipper* clipper = 0; // stays 0
    // printf("%d, %d, %d, %d", width, height, count, num_relax);
    jcv_rect *rect = 0;
    for(int i = 0; i < num_relax; ++i ) {
        
        jcv_diagram diagram;
        memset(&diagram, 0, sizeof(jcv_diagram));
        jcv_diagram_generate(count, (const jcv_point *)points, rect, clipper, &diagram);

        _relax_points(&diagram, points);

        jcv_diagram_free( &diagram );
    }
    
    jcv_diagram diagram;
    
    memset(&diagram, 0, sizeof(jcv_diagram));
    jcv_diagram_generate(count, (const jcv_point*)points, rect, clipper, &diagram);    
    RawGraph ret = {diagram, points, count};
    return ret;
}

VriGraph make_voronoi_graph(RawGraph *r) {
    const jcv_site* sites = jcv_diagram_get_sites(&r->diagram);
    VriGraph graph;
    graph.node_count = r->diagram.numsites;
    graph.nodes = (VriNode *)malloc(sizeof(VriNode) * graph.node_count);
    for(int i = 0; i < r->diagram.numsites; ++i ) {
        const jcv_site *site = &sites[i];
        VriNode *node = &graph.nodes[site->index];
        node->edg = new_int_carray(0, 10);
        node->point = as_point(site->p);
        node->index = site->index;
        
        int count = 0;
        int neighbor_count = 0;
        const jcv_graphedge *e = site->edges;
        while(e) {
            count += 1;
            // cells on the edge of the field don't have a neighbour but a graph edge still exists
            if(e->neighbor) {
                neighbor_count += 1;
            }
            e = e->next;            
        }
        
        // @todo?        
        node->adj_count = neighbor_count;
        node->tri_count = count;
        int _size = sizeof(int) * node->adj_count;
        node->adj = (int *)        malloc(_size);
        node->tri = (VriTriangle *)malloc(sizeof(VriTriangle) * node->tri_count);
        // printf("%d", node->adj_count); fflush(stdout);
        
        int adj_i = 0;
        int tri_i = 0;
        e = site->edges;
        while(e) {            
            // dassert(i+5<node->adj_count);
            if(e->neighbor) {
                node->adj[adj_i] = e->neighbor->index;
                adj_i += 1;
            }
            VriPoint p0 = as_point(e->pos[0]);
            VriPoint p1 = as_point(e->pos[1]);
            node->tri[tri_i] = (VriTriangle){p0, node->point, p1};
            e = e->next;            
            tri_i += 1;
        }
    }
    const jcv_edge *edge = jcv_diagram_get_edges(&r->diagram);
    int edge_count = 0;
    
    while(edge) {
        edge_count += 1;
        edge = jcv_diagram_get_next_edge(edge);        
    }
    graph.edge_count = edge_count;
    graph.edges = (VriEdge *)malloc(sizeof(VriEdge) * graph.edge_count);
    int i = 0;
    edge = jcv_diagram_get_edges(&r->diagram);
    while(edge) {
        VriPoint p0 = as_point(edge->pos[0]);
        VriPoint p1 = as_point(edge->pos[1]);
        int node_a = -1, node_b = -1;
        if(edge->sites[0]) {
            node_a = edge->sites[0]->index;
            Edges *edg = &graph.nodes[node_a].edg;
            int_carray_add(edg, &i);
        }
        if(edge->sites[1]) {
            node_b = edge->sites[1]->index;
            Edges *edg = &graph.nodes[node_b].edg;
            int_carray_add(edg, &i);
        }
        graph.edges[i] = (VriEdge){p0, p1, node_a, node_b};
        edge = jcv_diagram_get_next_edge(edge);
        
        i += 1;
    }
    return graph;
}

void free_raw_graph(RawGraph *r) {
    jcv_diagram_free(&r->diagram);
    free(r->points);
}
void soft_free_raw_graph(RawGraph *r) {
    jcv_diagram_free(&r->diagram);
    // free(r->points); // points might come from caller
}
static void free_node(VriNode *node) {
    free(node->adj);
    free(node->tri);
    free(node->edg.data);
}
void free_vri_graph(VriGraph *g) {
    for(int i = 0; i < g->node_count; i+=1) {
        free_node(&g->nodes[i]);
    }
    free(g->edges);
    free(g->nodes);
}