/*
 * Room 3D Source for OBS
 * Creates a 3D wireframe room with an embedded source on the back wall
 */

#include "room-3d-source.hpp"
#include <obs-frontend-api.h>
#include <util/platform.h>
#include <cmath>
#include <cstring>

#define ROOM3D_SOURCE_ID "room_3d_source"
#define ROOM3D_SOURCE_NAME "3D Room Source"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// Utility Functions
// ============================================================================

void room3d_vec4_from_rgba(struct vec4* out, uint32_t rgba)
{
    out->x = (float)((rgba >> 0) & 0xFF) / 255.0f;   // R
    out->y = (float)((rgba >> 8) & 0xFF) / 255.0f;   // G
    out->z = (float)((rgba >> 16) & 0xFF) / 255.0f;  // B
    out->w = (float)((rgba >> 24) & 0xFF) / 255.0f;  // A
}

// ============================================================================
// Geometry Generation - Screen Space (2D with perspective simulation)
// ============================================================================

// Convert 3D point to 2D screen coordinates with perspective
static void project_point(float x3d, float y3d, float z3d,
                          float* x2d, float* y2d,
                          float width, float height, float depth, float fov)
{
    // Simple perspective projection
    // Camera is at z=0, looking towards -z
    float camera_dist = depth * 1.5f;
    float scale = camera_dist / (camera_dist - z3d);

    // Project to screen coordinates (center of screen is 0,0)
    *x2d = x3d * scale * (width / 2.0f) + (width / 2.0f);
    *y2d = -y3d * scale * (height / 2.0f) + (height / 2.0f);  // Flip Y
}

gs_vertbuffer_t* room3d_create_room_edges(const Room3DConfig& config, size_t* out_count)
{
    float w = (float)config.output_width;
    float h = (float)config.output_height;
    float d = config.room_depth;
    float fov = config.fov;

    // Room corners in 3D (normalized -1 to 1 range)
    // Front face (z = 0)
    float fx[4], fy[4];  // Front corners (x, y)
    project_point(-1, -1, 0, &fx[0], &fy[0], w, h, d, fov);  // bottom-left
    project_point( 1, -1, 0, &fx[1], &fy[1], w, h, d, fov);  // bottom-right
    project_point( 1,  1, 0, &fx[2], &fy[2], w, h, d, fov);  // top-right
    project_point(-1,  1, 0, &fx[3], &fy[3], w, h, d, fov);  // top-left

    // Back face (z = -depth)
    float bx[4], by[4];  // Back corners
    project_point(-1, -1, -d, &bx[0], &by[0], w, h, d, fov);
    project_point( 1, -1, -d, &bx[1], &by[1], w, h, d, fov);
    project_point( 1,  1, -d, &bx[2], &by[2], w, h, d, fov);
    project_point(-1,  1, -d, &bx[3], &by[3], w, h, d, fov);

    // 12 edges = 24 vertices
    std::vector<WireframeVertex> vertices;
    vertices.reserve(24);

    // Front face (4 edges)
    vertices.push_back({fx[0], fy[0], 0}); vertices.push_back({fx[1], fy[1], 0});
    vertices.push_back({fx[1], fy[1], 0}); vertices.push_back({fx[2], fy[2], 0});
    vertices.push_back({fx[2], fy[2], 0}); vertices.push_back({fx[3], fy[3], 0});
    vertices.push_back({fx[3], fy[3], 0}); vertices.push_back({fx[0], fy[0], 0});

    // Back face (4 edges)
    vertices.push_back({bx[0], by[0], 0}); vertices.push_back({bx[1], by[1], 0});
    vertices.push_back({bx[1], by[1], 0}); vertices.push_back({bx[2], by[2], 0});
    vertices.push_back({bx[2], by[2], 0}); vertices.push_back({bx[3], by[3], 0});
    vertices.push_back({bx[3], by[3], 0}); vertices.push_back({bx[0], by[0], 0});

    // Connecting edges (4 edges)
    vertices.push_back({fx[0], fy[0], 0}); vertices.push_back({bx[0], by[0], 0});
    vertices.push_back({fx[1], fy[1], 0}); vertices.push_back({bx[1], by[1], 0});
    vertices.push_back({fx[2], fy[2], 0}); vertices.push_back({bx[2], by[2], 0});
    vertices.push_back({fx[3], fy[3], 0}); vertices.push_back({bx[3], by[3], 0});

    *out_count = vertices.size();

    gs_vb_data* vb_data = gs_vbdata_create();
    vb_data->num = vertices.size();
    vb_data->points = (struct vec3*)bmalloc(sizeof(struct vec3) * vertices.size());

    for (size_t i = 0; i < vertices.size(); i++) {
        vec3_set(&vb_data->points[i], vertices[i].x, vertices[i].y, 0.0f);
    }

    return gs_vertexbuffer_create(vb_data, GS_DYNAMIC);
}

gs_vertbuffer_t* room3d_create_floor_grid(const Room3DConfig& config, size_t* out_count)
{
    float w = (float)config.output_width;
    float h = (float)config.output_height;
    float d = config.room_depth;
    float fov = config.fov;
    int divisions = config.grid_divisions;

    std::vector<WireframeVertex> vertices;

    // Floor is at y = -1 (bottom)
    // Lines from front to back
    for (int i = 0; i <= divisions; i++) {
        float t = (float)i / (float)divisions;
        float x = -1.0f + t * 2.0f;

        float x1, y1, x2, y2;
        project_point(x, -1, 0, &x1, &y1, w, h, d, fov);
        project_point(x, -1, -d, &x2, &y2, w, h, d, fov);

        vertices.push_back({x1, y1, 0});
        vertices.push_back({x2, y2, 0});
    }

    // Lines from left to right
    for (int i = 0; i <= divisions; i++) {
        float t = (float)i / (float)divisions;
        float z = -t * d;

        float x1, y1, x2, y2;
        project_point(-1, -1, z, &x1, &y1, w, h, d, fov);
        project_point( 1, -1, z, &x2, &y2, w, h, d, fov);

        vertices.push_back({x1, y1, 0});
        vertices.push_back({x2, y2, 0});
    }

    *out_count = vertices.size();

    gs_vb_data* vb_data = gs_vbdata_create();
    vb_data->num = vertices.size();
    vb_data->points = (struct vec3*)bmalloc(sizeof(struct vec3) * vertices.size());

    for (size_t i = 0; i < vertices.size(); i++) {
        vec3_set(&vb_data->points[i], vertices[i].x, vertices[i].y, 0.0f);
    }

    return gs_vertexbuffer_create(vb_data, GS_DYNAMIC);
}

gs_vertbuffer_t* room3d_create_wall_grid(const Room3DConfig& config, bool left_wall, size_t* out_count)
{
    float w = (float)config.output_width;
    float h = (float)config.output_height;
    float d = config.room_depth;
    float fov = config.fov;
    int divisions = config.grid_divisions;

    std::vector<WireframeVertex> vertices;

    float x = left_wall ? -1.0f : 1.0f;

    // Vertical lines (front to back)
    for (int i = 0; i <= divisions; i++) {
        float t = (float)i / (float)divisions;
        float y = -1.0f + t * 2.0f;

        float x1, y1, x2, y2;
        project_point(x, y, 0, &x1, &y1, w, h, d, fov);
        project_point(x, y, -d, &x2, &y2, w, h, d, fov);

        vertices.push_back({x1, y1, 0});
        vertices.push_back({x2, y2, 0});
    }

    // Horizontal lines (bottom to top at different depths)
    for (int i = 0; i <= divisions; i++) {
        float t = (float)i / (float)divisions;
        float z = -t * d;

        float x1, y1, x2, y2;
        project_point(x, -1, z, &x1, &y1, w, h, d, fov);
        project_point(x,  1, z, &x2, &y2, w, h, d, fov);

        vertices.push_back({x1, y1, 0});
        vertices.push_back({x2, y2, 0});
    }

    *out_count = vertices.size();

    gs_vb_data* vb_data = gs_vbdata_create();
    vb_data->num = vertices.size();
    vb_data->points = (struct vec3*)bmalloc(sizeof(struct vec3) * vertices.size());

    for (size_t i = 0; i < vertices.size(); i++) {
        vec3_set(&vb_data->points[i], vertices[i].x, vertices[i].y, 0.0f);
    }

    return gs_vertexbuffer_create(vb_data, GS_DYNAMIC);
}

gs_vertbuffer_t* room3d_create_back_wall_quad(const Room3DConfig& config)
{
    float w = (float)config.output_width;
    float h = (float)config.output_height;
    float d = config.room_depth;
    float fov = config.fov;
    float margin = config.wall_margin;

    // Back wall with margin
    float m = 1.0f - margin * 2.0f;

    float x[4], y[4];
    project_point(-m, -m, -d, &x[0], &y[0], w, h, d, fov);  // bottom-left
    project_point(-m,  m, -d, &x[1], &y[1], w, h, d, fov);  // top-left
    project_point( m, -m, -d, &x[2], &y[2], w, h, d, fov);  // bottom-right
    project_point( m,  m, -d, &x[3], &y[3], w, h, d, fov);  // top-right

    gs_vb_data* vb_data = gs_vbdata_create();
    vb_data->num = 4;
    vb_data->points = (struct vec3*)bmalloc(sizeof(struct vec3) * 4);
    vb_data->num_tex = 1;
    vb_data->tvarray = (struct gs_tvertarray*)bmalloc(sizeof(struct gs_tvertarray));
    vb_data->tvarray[0].width = 2;
    vb_data->tvarray[0].array = bmalloc(sizeof(struct vec2) * 4);

    struct vec2* uvs = (struct vec2*)vb_data->tvarray[0].array;

    // Triangle strip order: BL, TL, BR, TR
    vec3_set(&vb_data->points[0], x[0], y[0], 0.0f);
    vec2_set(&uvs[0], 0.0f, 1.0f);

    vec3_set(&vb_data->points[1], x[1], y[1], 0.0f);
    vec2_set(&uvs[1], 0.0f, 0.0f);

    vec3_set(&vb_data->points[2], x[2], y[2], 0.0f);
    vec2_set(&uvs[2], 1.0f, 1.0f);

    vec3_set(&vb_data->points[3], x[3], y[3], 0.0f);
    vec2_set(&uvs[3], 1.0f, 0.0f);

    return gs_vertexbuffer_create(vb_data, GS_DYNAMIC);
}

void room3d_create_geometry(Room3DContext* ctx)
{
    obs_enter_graphics();

    ctx->room_edges_vb = room3d_create_room_edges(ctx->config, &ctx->room_edges_count);
    ctx->floor_grid_vb = room3d_create_floor_grid(ctx->config, &ctx->floor_grid_count);
    ctx->left_wall_grid_vb = room3d_create_wall_grid(ctx->config, true, &ctx->left_wall_grid_count);
    ctx->right_wall_grid_vb = room3d_create_wall_grid(ctx->config, false, &ctx->right_wall_grid_count);
    ctx->back_wall_quad_vb = room3d_create_back_wall_quad(ctx->config);

    ctx->geometry_dirty = false;

    obs_leave_graphics();
}

void room3d_destroy_geometry(Room3DContext* ctx)
{
    obs_enter_graphics();

    if (ctx->room_edges_vb) {
        gs_vertexbuffer_destroy(ctx->room_edges_vb);
        ctx->room_edges_vb = nullptr;
    }
    if (ctx->floor_grid_vb) {
        gs_vertexbuffer_destroy(ctx->floor_grid_vb);
        ctx->floor_grid_vb = nullptr;
    }
    if (ctx->left_wall_grid_vb) {
        gs_vertexbuffer_destroy(ctx->left_wall_grid_vb);
        ctx->left_wall_grid_vb = nullptr;
    }
    if (ctx->right_wall_grid_vb) {
        gs_vertexbuffer_destroy(ctx->right_wall_grid_vb);
        ctx->right_wall_grid_vb = nullptr;
    }
    if (ctx->back_wall_quad_vb) {
        gs_vertexbuffer_destroy(ctx->back_wall_quad_vb);
        ctx->back_wall_quad_vb = nullptr;
    }

    obs_leave_graphics();
}

void room3d_rebuild_geometry(Room3DContext* ctx)
{
    room3d_destroy_geometry(ctx);
    room3d_create_geometry(ctx);
}

// ============================================================================
// Rendering Functions
// ============================================================================

void room3d_capture_embed_source(Room3DContext* ctx)
{
    if (!ctx->embed_source)
        return;

    obs_source_t* embed = obs_weak_source_get_source(ctx->embed_source);
    if (!embed)
        return;

    uint32_t width = obs_source_get_width(embed);
    uint32_t height = obs_source_get_height(embed);

    if (width > 0 && height > 0) {
        gs_texrender_reset(ctx->texrender);

        if (gs_texrender_begin(ctx->texrender, width, height)) {
            struct vec4 clear_color;
            vec4_zero(&clear_color);
            gs_clear(GS_CLEAR_COLOR, &clear_color, 0.0f, 0);

            gs_ortho(0.0f, (float)width, 0.0f, (float)height, -100.0f, 100.0f);
            obs_source_video_render(embed);

            gs_texrender_end(ctx->texrender);
        }
    }

    obs_source_release(embed);
}

void room3d_render_wireframe(Room3DContext* ctx)
{
    // Use OBS solid color effect
    gs_effect_t* solid = obs_get_base_effect(OBS_EFFECT_SOLID);
    if (!solid)
        return;

    gs_eparam_t* color_param = gs_effect_get_param_by_name(solid, "color");

    struct vec4 color;
    room3d_vec4_from_rgba(&color, ctx->config.wireframe_color);
    gs_effect_set_vec4(color_param, &color);

    while (gs_effect_loop(solid, "Solid")) {
        // Room edges
        if (ctx->room_edges_vb) {
            gs_load_vertexbuffer(ctx->room_edges_vb);
            gs_load_indexbuffer(nullptr);
            gs_draw(GS_LINES, 0, (uint32_t)ctx->room_edges_count);
        }

        // Floor grid
        if (ctx->floor_grid_vb) {
            gs_load_vertexbuffer(ctx->floor_grid_vb);
            gs_draw(GS_LINES, 0, (uint32_t)ctx->floor_grid_count);
        }

        // Left wall grid
        if (ctx->left_wall_grid_vb) {
            gs_load_vertexbuffer(ctx->left_wall_grid_vb);
            gs_draw(GS_LINES, 0, (uint32_t)ctx->left_wall_grid_count);
        }

        // Right wall grid
        if (ctx->right_wall_grid_vb) {
            gs_load_vertexbuffer(ctx->right_wall_grid_vb);
            gs_draw(GS_LINES, 0, (uint32_t)ctx->right_wall_grid_count);
        }
    }
}

void room3d_render_back_wall(Room3DContext* ctx)
{
    if (!ctx->back_wall_quad_vb)
        return;

    gs_texture_t* tex = gs_texrender_get_texture(ctx->texrender);
    if (!tex)
        return;

    // Use default draw effect
    gs_effect_t* effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);
    if (!effect)
        return;

    gs_eparam_t* image_param = gs_effect_get_param_by_name(effect, "image");
    gs_effect_set_texture(image_param, tex);

    while (gs_effect_loop(effect, "Draw")) {
        gs_load_vertexbuffer(ctx->back_wall_quad_vb);
        gs_load_indexbuffer(nullptr);
        gs_draw(GS_TRISTRIP, 0, 4);
    }
}

// ============================================================================
// OBS Source Callbacks
// ============================================================================

static const char* room_3d_get_name(void* type_data)
{
    UNUSED_PARAMETER(type_data);
    return ROOM3D_SOURCE_NAME;
}

static void* room_3d_create(obs_data_t* settings, obs_source_t* source)
{
    Room3DContext* ctx = new Room3DContext();
    memset(ctx, 0, sizeof(Room3DContext));

    ctx->source = source;
    ctx->geometry_dirty = true;

    // Set default config
    ctx->config.room_width = 1.0f;
    ctx->config.room_height = 1.0f;
    ctx->config.room_depth = 2.0f;
    ctx->config.wireframe_color = 0xFF00FF00;  // Green with full alpha
    ctx->config.grid_divisions = 10;
    ctx->config.fov = 60.0f;
    ctx->config.wall_margin = 0.1f;
    ctx->config.output_width = 1920;
    ctx->config.output_height = 1080;

    // Create texture render target
    obs_enter_graphics();
    ctx->texrender = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
    obs_leave_graphics();

    // Load settings
    ctx->config.room_depth = (float)obs_data_get_double(settings, "room_depth");
    if (ctx->config.room_depth <= 0)
        ctx->config.room_depth = 2.0f;

    ctx->config.wireframe_color = (uint32_t)obs_data_get_int(settings, "wireframe_color");
    if (ctx->config.wireframe_color == 0)
        ctx->config.wireframe_color = 0xFF00FF00;

    ctx->config.grid_divisions = (int)obs_data_get_int(settings, "grid_divisions");
    if (ctx->config.grid_divisions <= 0)
        ctx->config.grid_divisions = 10;

    ctx->config.fov = (float)obs_data_get_double(settings, "fov");
    if (ctx->config.fov <= 0)
        ctx->config.fov = 60.0f;

    ctx->config.wall_margin = (float)obs_data_get_double(settings, "wall_margin");

    ctx->config.output_width = (uint32_t)obs_data_get_int(settings, "output_width");
    if (ctx->config.output_width == 0)
        ctx->config.output_width = 1920;

    ctx->config.output_height = (uint32_t)obs_data_get_int(settings, "output_height");
    if (ctx->config.output_height == 0)
        ctx->config.output_height = 1080;

    const char* embed_name = obs_data_get_string(settings, "embed_source");
    if (embed_name && *embed_name) {
        ctx->config.embed_source_name = embed_name;
        obs_source_t* embed = obs_get_source_by_name(embed_name);
        if (embed) {
            ctx->embed_source = obs_source_get_weak_source(embed);
            obs_source_release(embed);
        }
    }

    // Create geometry
    room3d_create_geometry(ctx);

    blog(LOG_INFO, "[Room3D] Source created: %dx%d, depth=%.1f",
         ctx->config.output_width, ctx->config.output_height, ctx->config.room_depth);

    return ctx;
}

static void room_3d_destroy(void* data)
{
    Room3DContext* ctx = (Room3DContext*)data;

    room3d_destroy_geometry(ctx);

    obs_enter_graphics();

    if (ctx->texrender) {
        gs_texrender_destroy(ctx->texrender);
    }

    obs_leave_graphics();

    if (ctx->embed_source) {
        obs_weak_source_release(ctx->embed_source);
    }

    delete ctx;

    blog(LOG_INFO, "[Room3D] Source destroyed");
}

static void room_3d_update(void* data, obs_data_t* settings)
{
    Room3DContext* ctx = (Room3DContext*)data;

    float new_depth = (float)obs_data_get_double(settings, "room_depth");
    if (new_depth <= 0) new_depth = 2.0f;

    uint32_t new_color = (uint32_t)obs_data_get_int(settings, "wireframe_color");
    if (new_color == 0) new_color = 0xFF00FF00;

    int new_divisions = (int)obs_data_get_int(settings, "grid_divisions");
    if (new_divisions <= 0) new_divisions = 10;

    float new_fov = (float)obs_data_get_double(settings, "fov");
    if (new_fov <= 0) new_fov = 60.0f;

    float new_margin = (float)obs_data_get_double(settings, "wall_margin");

    uint32_t new_width = (uint32_t)obs_data_get_int(settings, "output_width");
    if (new_width == 0) new_width = 1920;

    uint32_t new_height = (uint32_t)obs_data_get_int(settings, "output_height");
    if (new_height == 0) new_height = 1080;

    // Check if geometry needs rebuild
    bool needs_rebuild = (new_depth != ctx->config.room_depth ||
                          new_divisions != ctx->config.grid_divisions ||
                          new_margin != ctx->config.wall_margin ||
                          new_width != ctx->config.output_width ||
                          new_height != ctx->config.output_height ||
                          new_fov != ctx->config.fov);

    ctx->config.room_depth = new_depth;
    ctx->config.wireframe_color = new_color;
    ctx->config.grid_divisions = new_divisions;
    ctx->config.fov = new_fov;
    ctx->config.wall_margin = new_margin;
    ctx->config.output_width = new_width;
    ctx->config.output_height = new_height;

    // Update embedded source
    const char* embed_name = obs_data_get_string(settings, "embed_source");
    if (embed_name && ctx->config.embed_source_name != embed_name) {
        ctx->config.embed_source_name = embed_name;

        if (ctx->embed_source) {
            obs_weak_source_release(ctx->embed_source);
            ctx->embed_source = nullptr;
        }

        if (*embed_name) {
            obs_source_t* embed = obs_get_source_by_name(embed_name);
            if (embed) {
                ctx->embed_source = obs_source_get_weak_source(embed);
                obs_source_release(embed);
            }
        }
    }

    if (needs_rebuild) {
        room3d_rebuild_geometry(ctx);
        blog(LOG_INFO, "[Room3D] Geometry rebuilt");
    }
}

static uint32_t room_3d_get_width(void* data)
{
    Room3DContext* ctx = (Room3DContext*)data;
    return ctx->config.output_width;
}

static uint32_t room_3d_get_height(void* data)
{
    Room3DContext* ctx = (Room3DContext*)data;
    return ctx->config.output_height;
}

static void room_3d_video_render(void* data, gs_effect_t* effect)
{
    UNUSED_PARAMETER(effect);
    Room3DContext* ctx = (Room3DContext*)data;

    if (!ctx)
        return;

    // Capture embedded source to texture
    room3d_capture_embed_source(ctx);

    uint32_t width = ctx->config.output_width;
    uint32_t height = ctx->config.output_height;

    // Set up orthographic projection for 2D rendering
    gs_ortho(0.0f, (float)width, 0.0f, (float)height, -1.0f, 1.0f);

    // Enable blending
    gs_enable_blending(true);
    gs_blend_function(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA);

    // Render back wall texture first (behind wireframe)
    room3d_render_back_wall(ctx);

    // Render wireframe on top
    room3d_render_wireframe(ctx);
}

static void room_3d_get_defaults(obs_data_t* settings)
{
    obs_data_set_default_double(settings, "room_depth", 2.0);
    obs_data_set_default_int(settings, "wireframe_color", 0xFF00FF00);
    obs_data_set_default_int(settings, "grid_divisions", 10);
    obs_data_set_default_double(settings, "fov", 60.0);
    obs_data_set_default_double(settings, "wall_margin", 0.1);
    obs_data_set_default_int(settings, "output_width", 1920);
    obs_data_set_default_int(settings, "output_height", 1080);
    obs_data_set_default_string(settings, "embed_source", "");
}

static bool add_source_to_list(void* data, obs_source_t* source)
{
    obs_property_t* list = (obs_property_t*)data;

    uint32_t flags = obs_source_get_output_flags(source);
    if (!(flags & OBS_SOURCE_VIDEO))
        return true;

    const char* name = obs_source_get_name(source);
    obs_property_list_add_string(list, name, name);

    return true;
}

static obs_properties_t* room_3d_get_properties(void* data)
{
    UNUSED_PARAMETER(data);

    obs_properties_t* props = obs_properties_create();

    // Embedded source selection
    obs_property_t* source_list = obs_properties_add_list(props, "embed_source",
        "Embedded Source", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
    obs_property_list_add_string(source_list, "(None)", "");
    obs_enum_sources(add_source_to_list, source_list);

    // Output resolution
    obs_properties_add_int(props, "output_width", "Output Width", 640, 7680, 1);
    obs_properties_add_int(props, "output_height", "Output Height", 480, 4320, 1);

    // Room geometry
    obs_properties_add_float_slider(props, "room_depth", "Room Depth", 0.5, 10.0, 0.1);
    obs_properties_add_float_slider(props, "wall_margin", "Wall Margin", 0.0, 0.4, 0.01);

    // Wireframe style
    obs_properties_add_color(props, "wireframe_color", "Wireframe Color");
    obs_properties_add_int_slider(props, "grid_divisions", "Grid Divisions", 2, 20, 1);

    // Camera
    obs_properties_add_float_slider(props, "fov", "Field of View", 30.0, 120.0, 1.0);

    return props;
}

// Source info structure - initialized in register function for C++17 compatibility
static struct obs_source_info room_3d_source_info;

// ============================================================================
// Registration
// ============================================================================

void register_room_3d_source()
{
    // Initialize source info structure (C++17 compatible)
    memset(&room_3d_source_info, 0, sizeof(room_3d_source_info));
    room_3d_source_info.id = ROOM3D_SOURCE_ID;
    room_3d_source_info.type = OBS_SOURCE_TYPE_INPUT;
    room_3d_source_info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW;
    room_3d_source_info.get_name = room_3d_get_name;
    room_3d_source_info.create = room_3d_create;
    room_3d_source_info.destroy = room_3d_destroy;
    room_3d_source_info.update = room_3d_update;
    room_3d_source_info.video_render = room_3d_video_render;
    room_3d_source_info.get_width = room_3d_get_width;
    room_3d_source_info.get_height = room_3d_get_height;
    room_3d_source_info.get_properties = room_3d_get_properties;
    room_3d_source_info.get_defaults = room_3d_get_defaults;

    obs_register_source(&room_3d_source_info);
    blog(LOG_INFO, "[Room3D] 3D Room Source registered");
}
