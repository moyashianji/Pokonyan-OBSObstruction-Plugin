#pragma once

#include <obs-module.h>
#include <graphics/graphics.h>
#include <graphics/matrix4.h>
#include <graphics/vec3.h>
#include <graphics/vec4.h>
#include <string>
#include <vector>

// Forward declarations
struct Room3DContext;

// Room 3D Source API
void register_room_3d_source();

// Room geometry configuration
struct Room3DConfig {
    // Room dimensions (normalized, scaled by output resolution)
    float room_width = 1.0f;
    float room_height = 1.0f;
    float room_depth = 2.0f;

    // Wireframe appearance
    uint32_t wireframe_color = 0xFF00FF00;  // Green (ABGR format)
    int grid_divisions = 10;

    // Camera/perspective
    float fov = 60.0f;

    // Back wall margin (0.0-0.5)
    float wall_margin = 0.1f;

    // Embedded source name
    std::string embed_source_name;

    // Output dimensions
    uint32_t output_width = 1920;
    uint32_t output_height = 1080;
};

// Vertex structure for wireframe lines
struct WireframeVertex {
    float x, y, z;
};

// Room 3D Context - main data structure for the source
struct Room3DContext {
    obs_source_t* source;

    // Configuration
    Room3DConfig config;

    // Embedded source
    obs_weak_source_t* embed_source;
    gs_texrender_t* texrender;

    // Vertex buffers
    gs_vertbuffer_t* room_edges_vb;      // 12 room edge lines
    gs_vertbuffer_t* floor_grid_vb;      // Floor grid lines
    gs_vertbuffer_t* left_wall_grid_vb;  // Left wall grid
    gs_vertbuffer_t* right_wall_grid_vb; // Right wall grid
    gs_vertbuffer_t* back_wall_quad_vb;  // Textured back wall

    // Vertex counts
    size_t room_edges_count;
    size_t floor_grid_count;
    size_t left_wall_grid_count;
    size_t right_wall_grid_count;

    // Effects (shaders)
    gs_effect_t* wireframe_effect;
    gs_effect_t* texture_effect;

    // Effect parameters
    gs_eparam_t* wireframe_color_param;
    gs_eparam_t* wireframe_viewproj_param;
    gs_eparam_t* texture_image_param;
    gs_eparam_t* texture_viewproj_param;

    // Geometry needs rebuild flag
    bool geometry_dirty;
};

// Geometry generation functions
void room3d_create_geometry(Room3DContext* ctx);
void room3d_destroy_geometry(Room3DContext* ctx);
void room3d_rebuild_geometry(Room3DContext* ctx);

// Vertex buffer creation helpers
gs_vertbuffer_t* room3d_create_room_edges(const Room3DConfig& config, size_t* out_count);
gs_vertbuffer_t* room3d_create_floor_grid(const Room3DConfig& config, size_t* out_count);
gs_vertbuffer_t* room3d_create_wall_grid(const Room3DConfig& config, bool left_wall, size_t* out_count);
gs_vertbuffer_t* room3d_create_back_wall_quad(const Room3DConfig& config);

// Rendering functions
void room3d_render_wireframe(Room3DContext* ctx);
void room3d_render_back_wall(Room3DContext* ctx);
void room3d_capture_embed_source(Room3DContext* ctx);

// Utility
void room3d_vec4_from_rgba(struct vec4* out, uint32_t rgba);
