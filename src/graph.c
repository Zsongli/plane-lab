#include "graph.h"
#include <imgui_macros.h>
#include <string.h>
#include <debugmalloc.h>

void graph_new(Graph* this) {
	this->selected_shape = NULL;
	linked_list_new(&this->shapes);

	string_new(&this->working_file, NULL);
}

void graph_delete(Graph* this) {
	string_delete(&this->working_file);

	for (LinkedListNode* iter = this->shapes.head; iter != NULL; iter = iter->next) {
		Shape* shape = iter->value;
		shape->vtable->delete(shape);
		free(shape);
	}
	linked_list_delete(&this->shapes);
}

bool graph_add_default_shape(Graph* this, ShapeType type) {
	Buffer shape_buffer;
	if (!buffer_new(&shape_buffer, sizeof(Shape))) return false;
	if (!shape_new_default_from_type(type, &shape_buffer)) return false;

	Shape* shape = shape_buffer.data; // won't get freed unless we free the buffer, which we don't, we manually free the pointer later
	if (!linked_list_push_back(&this->shapes, shape)) {
		shape->vtable->delete(shape);
		buffer_delete(&shape_buffer);
		return false;
	}

	printf("Added new default shape of type %s\n", shape_type_to_string(type));

	return true;
}

void graph_remove_shape(Graph* this, Shape* shape) {

	printf("Removing shape with label: %s\n", (char*)shape->label.data);

	if (shape == this->shapes.head->value) linked_list_remove_head(&this->shapes);
	else linked_list_remove_after(linked_list_find_preceding(&this->shapes, shape));

	shape->vtable->delete(shape);
	free(shape);
}

static const uint8_t HEADER_MAGIC[] = "PLAB";
static const uint16_t FILE_VERSION = 1;
#pragma pack(push, 1)
typedef struct {
	uint8_t magic[IM_ARRAYSIZE(HEADER_MAGIC) - 1];
	uint16_t version;
	uint64_t shape_count;
} SerializedGraphHeader;
#pragma pack(pop)

bool graph_serialize(Graph* this, Buffer* out_data) {
	SerializedGraphHeader header = {
		.version = FILE_VERSION,
		.shape_count = linked_list_count(&this->shapes)
	};
	strncpy_s(header.magic, IM_ARRAYSIZE(HEADER_MAGIC) - 1, HEADER_MAGIC, IM_ARRAYSIZE(HEADER_MAGIC) - 1);

	if (!buffer_push_back(out_data, &header, sizeof(SerializedGraphHeader))) return false;

	for (LinkedListNode* iter = this->shapes.head; iter != NULL; iter = iter->next) {
		Shape* shape = iter->value;

		Buffer shape_data;
		if (!buffer_new(&shape_data, 0)) return false;
		if (!shape->vtable->serialize(shape, &shape_data)) {
			buffer_delete(&shape_data);
			return false;
		}

		if (!buffer_push_back(out_data, shape_data.data, shape_data.size)) return false;
		buffer_delete(&shape_data);
	}

	return true;
}

bool graph_save_to_file(Graph* this, char* path) {
	FILE* f = fopen(path, "wb");
	if (!f) return false;

	Buffer serialized_graph;
	if (!buffer_new(&serialized_graph, 1024)) goto fail_buffer;
	if (!graph_serialize(this, &serialized_graph)) goto fail_serialize;

	fwrite(serialized_graph.data, 1, serialized_graph.size, f);

	buffer_delete(&serialized_graph);
	fclose(f);

	printf("Saved graph to file: %s\n", path);
	
	return true;

fail_serialize:
	buffer_delete(&serialized_graph);
fail_buffer:
	fclose(f);
	return false;
}

bool graph_deserialize(Graph* this, Buffer* in_data) {

	SerializedGraphHeader header;
	if (!buffer_consume(in_data, sizeof(SerializedGraphHeader), &header)) return false;

	if (strncmp((char*)header.magic, (char*)HEADER_MAGIC, IM_ARRAYSIZE(HEADER_MAGIC) - 1) != 0) return false;
	if (header.version != FILE_VERSION) return false;

	for (size_t i = 0; i < header.shape_count; i++) {
		Buffer shape;
		if (!buffer_new(&shape, 0)) return false;
		if (!shape_deserialize_from_file_entry(in_data, &shape)) goto fail_deserialize;

		Shape* shape_ptr = shape.data; // won't get freed unless we free the buffer, which we don't, we manually free the pointer later
		if (!linked_list_push_back(&this->shapes, shape_ptr)) goto fail_push_back;

		continue;

	fail_push_back:
		shape_ptr->vtable->delete(shape_ptr);
	fail_deserialize:
		buffer_delete(&shape);
		LinkedListNode* iter = this->shapes.head;
		while (iter != NULL) {

			Shape* shape = iter->value;
			shape->vtable->delete(shape);
			free(shape);

			iter = iter->next;
			linked_list_remove_head(&this->shapes);
		}
		return false;
	}

	return true;
}

bool graph_load_from_file(Graph* this, char* path) {
	FILE* f = fopen(path, "rb");
	if (!f) return false;

	fseek(f, 0, SEEK_END);
	long file_size = ftell(f);
	fseek(f, 0, SEEK_SET);

	assert(file_size >= 0);

	Buffer file_data;
	if (!buffer_new(&file_data, (size_t)file_size)) goto fail_file;
	fread(file_data.data, 1, (size_t)file_size, f);
	file_data.size = (size_t)file_size;
	if (!graph_deserialize(this, &file_data)) goto fail_read;
	if (!string_set(&this->working_file, path)) goto fail_read;

	buffer_delete(&file_data);
	fclose(f);

	printf("Loaded graph from file: %s\n", path);
	
	return true;

fail_read:
	buffer_delete(&file_data);
fail_file:
	fclose(f);
	return false;
}

void graph_plot_shapes(Graph* this) {
	for (LinkedListNode* iter = this->shapes.head; iter != NULL; iter = iter->next) {
		Shape* shape = iter->value;

		igPushID_Ptr(shape);
		shape_plot(shape, shape == this->selected_shape);
		igPopID();
	}
}
