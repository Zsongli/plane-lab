#pragma once
#include "data_structures/linked_list.h"
#include "shapes/shape.h"
#include "shapes/shape_types.h"
#include "data_structures/dynamic_buffer.h"

typedef struct {
	String working_file;
	Shape* selected_shape;
	LinkedList shapes;
} Graph;

void graph_new(Graph* this);
void graph_delete(Graph* this);
bool graph_add_default_shape(Graph* this, ShapeType type);
void graph_remove_shape(Graph* this, Shape* shape);
bool graph_serialize(Graph* this, Buffer* out_data);
bool graph_save_to_file(Graph* this, const char* path);
bool graph_load_from_file(Graph* this, const char* path);
void graph_plot_shapes(Graph* this);
