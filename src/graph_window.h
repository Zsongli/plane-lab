#pragma once
#include "graph.h"

typedef struct {
	Graph graph;

	bool dockspace_initialized;
	ImGuiID dockspace_id;
	
	String window_name;
	String graph_window_name;
	String properties_window_name;
	String selector_window_name;

	bool should_close;
} GraphWindow;

bool graph_window_new(GraphWindow* this);
void graph_window_delete(GraphWindow* this);
void graph_window_draw(GraphWindow* this);
void graph_window_setup_docking_layout(GraphWindow* this);
bool graph_window_load_from_file(GraphWindow* this, char* path);
