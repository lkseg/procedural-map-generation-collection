#ifndef VORONOI_H
#define VORONOI_H
/*
    Wrapper around jc_voronoi.
    Creates a directed graph with connected nodes out of the voronoi diagram.
    Written in C such that it can be used as a lib for other languages.
*/

// #include <stdint.h>
#include "jc_voronoi.h"
#include "stdbool.h"
#include "c_basic.h"

typedef struct {
    jcv_diagram diagram;
    jcv_point *points;
    int points_count;
} RawGraph;



/* typedef struct {
    float x;
    float y;
} VriPoint; */

typedef jcv_point VriPoint;



typedef struct {
    VriPoint a;
    VriPoint b;
    int node_a;
    int node_b;
} VriEdge;

typedef struct {
    VriPoint a;
    VriPoint b;
    VriPoint c;
} VriTriangle;



// declare_carray(int)
typedef struct {
    int *data;
    int count;
	int capacity;
} int_CArray;

typedef int_CArray Edges;

typedef struct  {
    int index;
    VriPoint point;

    int *adj;
    int adj_count;
    
    VriTriangle *tri;
    int tri_count;

    Edges edg;
} VriNode;


typedef struct {
    VriNode *nodes;
    int node_count;
    VriEdge *edges;
    int edge_count;
    
    int width;
    int height;
} VriGraph;

#if defined(__cplusplus)
extern "C" {
#endif

RawGraph make_raw_voronoi_random(int, int, int, int, int);
RawGraph make_raw_voronoi_from_points(int, int, VriPoint *);
VriGraph make_voronoi_random(int width, int height, int count, int num_relax, int seed);
VriGraph make_voronoi_random_points(int width, int height, int count, VriPoint *points, int num_relax, int seed);
VriGraph make_voronoi_from_points(int count, int num_relax, VriPoint *);
VriGraph make_voronoi_graph(RawGraph *);
void free_raw_graph(RawGraph *);
void soft_free_raw_graph(RawGraph *);
void free_vri_graph(VriGraph *);

#if defined(__cplusplus)
}
#else
_Static_assert(sizeof(JCV_REAL_TYPE) == 4, "jcv_point should use 4 byte floats since we use it as a typedef for VriPoint");
#endif

#endif // VORONOI_H