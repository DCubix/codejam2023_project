#include "de_asset_manager.h"

#include "external/smol/smol_canvas.h"

#include "de_game_logic.h"
#include "de_game.h"

#define INVALID (size_t)(-1)

de_asset_manager_t de_assets = { 0 };

static uint32_t fnv32_hash(const char* str, size_t len) {
    unsigned char* s = (unsigned char*)str;	/* unsigned string */

    /* See the FNV parameters at www.isthe.com/chongo/tech/comp/fnv/#FNV-param */
    const uint32_t FNV_32_PRIME = 0x01000193; /* 16777619 */

    uint32_t h = 0x811c9dc5; /* 2166136261 */
    while (len--) {
        /* xor the bottom with the current octet */
        h ^= *s++;
        /* multiply by the 32 bit FNV magic prime mod 2^32 */
        h *= FNV_32_PRIME;
    }

    return h;
}

void de_assets_load_mesh(const char* name, smol_xml_t* xml, smol_xml_node_t* node) {
    if (de_assets_has_mesh(name)) return;

    de_mesh_t mesh = de_mesh_load(xml, node);
    uint32_t hash = fnv32_hash(name, strlen(name));
    smol_vector_push(&de_assets.mesh_hashes, hash);
    smol_vector_push(&de_assets.meshes, mesh);
}

void de_assets_load_texture(const char* name, const char* file_name) {
    if (de_assets_has_texture(name)) return;

    smol_image_t img = smol_load_image_qoi(file_name);
    de_texture_t tex = de_texture_create(img.pixel_data, img.width, img.height);
    
    uint32_t hash = fnv32_hash(name, strlen(name));
    smol_vector_push(&de_assets.texture_hashes, hash);
    smol_vector_push(&de_assets.textures, tex);

    smol_image_destroy(&img);
}

void de_assets_load_shader(const char* name, const char* vs_file_name, const char* fs_file_name) {
    if (de_assets_has_shader(name)) return;

    size_t buf_len;
    const char* vs = (char*)smol_read_entire_file(vs_file_name, &buf_len);
    const char* fs = (char*)smol_read_entire_file(fs_file_name, &buf_len);

    de_shader_t shader = de_shader_create(vs, fs);

    uint32_t hash = fnv32_hash(name, strlen(name));
    smol_vector_push(&de_assets.shader_hashes, hash);
    smol_vector_push(&de_assets.shaders, shader);

    free(vs);
    free(fs);
}

void de_assets_load_script(const char* name, de_script_t script) {
    if (de_assets_has_script(name)) return;

    uint32_t hash = fnv32_hash(name, strlen(name));
    smol_vector_push(&de_assets.script_hashes, hash);
    smol_vector_push(&de_assets.scripts, script);
}

// this will support only 1 scene in the collada file.
void de_scene_load_collada(de_scene_t* scene, const char* data) {
    de_scene_init(scene);

#define check(c) (isalnum(c) || c == '_' || c == '-')
#define ltrim_if_not(x) do { while (*x && !check(*x)) x++; } while(0)

    smol_xml_t doc = smol_xml_parse(data);

    smol_xml_matcher_t mt = { 0 };
    mt.has_tag = "COLLADA";

    smol_xml_node_t* root = smol_xml_find_one(&doc, &mt);
    SMOL_ASSERT(root != NULL);

    mt.has_tag = "library_visual_scenes";
    smol_xml_node_t* library_visual_scenes = smol_xml_find_one_child(&doc, root, &mt);
    SMOL_ASSERT(library_visual_scenes != NULL);

    mt.has_tag = "library_geometries";
    smol_xml_node_t* library_geometries = smol_xml_find_one_child(&doc, root, &mt);
    SMOL_ASSERT(library_geometries != NULL);

    mt.has_tag = "library_cameras";
    smol_xml_node_t* library_cameras = smol_xml_find_one_child(&doc, root, &mt);
    SMOL_ASSERT(library_cameras != NULL);

    mt.has_tag = "library_materials";
    smol_xml_node_t* library_materials = smol_xml_find_one_child(&doc, root, &mt);
    SMOL_ASSERT(library_materials != NULL);

    mt.has_tag = "visual_scene";
    smol_xml_node_t* visual_scene = smol_xml_find_one_child(&doc, library_visual_scenes, &mt);

    smol_xml_nodeptr_vector_t scene_nodes;
    smol_vector_init(&scene_nodes, 512);

    mt.has_tag = "node";
    mt.has_attr = "type";
    mt.attr_equals = "NODE";
    smol_xml_find_children(&doc, visual_scene, &mt, &scene_nodes);

    smol_vector_each(&scene_nodes, smol_xml_node_t*, nodeptr) {
        smol_xml_node_t* node = *nodeptr;

        // get node name
        char* name = smol_xml_get_attr(node, "name");

        mt.has_tag = "matrix";
        mt.has_attr = "sid";
        mt.attr_equals = "transform";
        smol_xml_node_t* matrix = smol_xml_find_one_child(&doc, node, &mt);
        SMOL_ASSERT(matrix != NULL);

        size_t i = 0;
        smol_m4_t mat = smol_m4_identity();
        char* matrix_data = smol_xml_node_get_body(matrix);
        while (*matrix_data) mat.m[i++] = strtof(matrix_data, &matrix_data);

        de_object_t obj = { 0 };
        de_object_init(&obj);

        smol_m4_decompose(mat, &obj.position, &obj.scale, &obj.rotation);

        obj.id = _de_scene_global_id++;
        obj.parent = DE_INVALID_OBJECT;
        obj.life = -1.0f;
        strcpy(obj.name, name);

        // parse mesh
        mt.has_tag = "instance_geometry";
        mt.has_attr = NULL;
        mt.attr_equals = NULL;
        smol_xml_node_t* instance_geometry = smol_xml_find_one_child(&doc, node, &mt);

        if (instance_geometry) {
            const char* mesh_url = smol_xml_get_attr(instance_geometry, "url");
            ltrim_if_not(mesh_url);

            char* texture_name = NULL;

            mt.has_tag = "geometry";
            mt.has_attr = "id";
            mt.attr_equals = mesh_url;
            smol_xml_node_t* geometry = smol_xml_find_one_child(&doc, library_geometries, &mt);
            SMOL_ASSERT(geometry != NULL);

            mt.has_tag = "mesh";
            mt.has_attr = NULL;
            mt.attr_equals = NULL;
            smol_xml_node_t* mesh = smol_xml_find_one_child(&doc, geometry, &mt);
            SMOL_ASSERT(mesh != NULL);

            mt.has_tag = "triangles";
            smol_xml_node_t* triangles = smol_xml_find_one_child(&doc, mesh, &mt);

            const char* material_name = smol_xml_get_attr(triangles, "material");

            mt.has_tag = "material";
            mt.has_attr = "id";
            mt.attr_equals = material_name;
            smol_xml_node_t* material = smol_xml_find_one_child(&doc, library_materials, &mt);

            texture_name = smol_xml_get_attr(material, "name");

            de_assets_load_mesh(mesh_url, &doc, mesh);

            de_mesh_component_t* mesh_comp = (de_mesh_component_t*)malloc(sizeof(de_mesh_component_t));
            memset(mesh_comp, 0, sizeof(de_mesh_component_t));

            mesh_comp->mesh = de_assets_get_mesh(mesh_url);
            mesh_comp->shader = de_assets_get_shader("default");

            if (texture_name) {
                mesh_comp->textures[0] = de_assets_get_texture(texture_name);
            }

            de_object_add_component(
                &obj, DE_BUILTIN_COMPONENT_MESH, mesh_comp, de_mesh_component_render_func, NULL
            );
        }

        // parse camera
        mt.has_tag = "instance_camera";
        mt.has_attr = NULL;
        mt.attr_equals = NULL;
        smol_xml_node_t* instance_camera = smol_xml_find_one_child(&doc, node, &mt);

        if (instance_camera) {
            const char* url = smol_xml_get_attr(instance_camera, "url");
            ltrim_if_not(url);

            mt.has_tag = "camera";
            mt.has_attr = "id";
            mt.attr_equals = url;
            smol_xml_node_t* camera = smol_xml_find_one_child(&doc, library_cameras, &mt);
            SMOL_ASSERT(camera != NULL);

            mt.has_tag = "optics";
            mt.has_attr = NULL;
            mt.attr_equals = NULL;
            smol_xml_node_t* optics = smol_xml_find_one_child(&doc, camera, &mt);
            SMOL_ASSERT(optics != NULL);

            mt.has_tag = "technique_common";
            mt.has_attr = NULL;
            mt.attr_equals = NULL;
            smol_xml_node_t* technique_common = smol_xml_find_one_child(&doc, optics, &mt);
            SMOL_ASSERT(technique_common != NULL);

            mt.has_tag = "perspective";
            mt.has_attr = NULL;
            mt.attr_equals = NULL;
            smol_xml_node_t* perspective = smol_xml_find_one_child(&doc, technique_common, &mt);
            SMOL_ASSERT(perspective != NULL);

            mt.has_tag = "xfov";
            smol_xml_node_t* xfov = smol_xml_find_one_child(&doc, perspective, &mt);
            SMOL_ASSERT(xfov != NULL);

            mt.has_tag = "aspect_ratio";
            smol_xml_node_t* aspect_ratio = smol_xml_find_one_child(&doc, perspective, &mt);
            SMOL_ASSERT(aspect_ratio != NULL);

            mt.has_tag = "znear";
            smol_xml_node_t* znear = smol_xml_find_one_child(&doc, perspective, &mt);
            SMOL_ASSERT(znear != NULL);

            mt.has_tag = "zfar";
            smol_xml_node_t* zfar = smol_xml_find_one_child(&doc, perspective, &mt);
            SMOL_ASSERT(zfar != NULL);

            de_camera_component_t* cam = (de_camera_component_t*)malloc(sizeof(de_camera_component_t));
            cam->z_near = strtof(smol_xml_node_get_body(znear), NULL);
            cam->z_far = strtof(smol_xml_node_get_body(zfar), NULL);
            cam->aspect = strtof(smol_xml_node_get_body(aspect_ratio), NULL);

            float fovx = SMOL_DEG_TO_RAD(strtof(smol_xml_node_get_body(xfov), NULL));
            float fovy = 2.0f * atanf(tanf(fovx / 2.0f) / cam->aspect);

            cam->fov = fovy;

            de_object_add_component(
                &obj, DE_BUILTIN_COMPONENT_CAMERA, cam, de_camera_component_render_func, NULL
            );
        }

        smol_vector_push(&scene->objects, obj);

        // TODO: child nodes (maybe depth 1)
    }

    smol_xml_free(&doc);
}

void de_assets_load_scene(de_game_t* game, const char* name, const char* file_path) {
    if (de_assets_has_scene(name)) return;
    
    de_scene_t scene;
    de_scene_init(&scene);

    smol_size_t buf_len;
    char* buf = (char*)smol_read_entire_file(file_path, &buf_len);

    smol_xml_t doc = smol_xml_parse(buf);

    smol_xml_matcher_t mt = { 0 };

    mt.has_tag = "level";
    smol_xml_node_t* root = smol_xml_find_one(&doc, &mt);

    const char* collada_file = smol_xml_get_attr(root, "assets");

    // first we need to laod the collada file, it must be in the same dir as this one!
    char* collada = (char*)smol_read_entire_file(collada_file, &buf_len);
    SMOL_ASSERT(collada != NULL);
    de_scene_load_collada(&scene, collada);

    smol_xml_nodeptr_vector_t objects;
    smol_vector_init(&objects, 512);

    mt.has_tag = "object";
    smol_xml_find_children(&doc, root, &mt, &objects);

    smol_vector_each(&objects, smol_xml_node_t*, objptr) {
        smol_xml_node_t* obj = *objptr;

        const char* ref = smol_xml_get_attr(obj, "ref");
        if (!ref) continue;

        de_object_t* scene_obj = de_scene_get_object_by_name(&scene, ref);
        if (!scene_obj) continue;

        const char* tag = smol_xml_get_attr(obj, "tag");
        if (tag) {
            strcpy(scene_obj->tag, tag);
        }

        mt.has_tag = "properties";
        smol_xml_node_t* properties = smol_xml_find_one_child(&doc, obj, &mt);
        if (properties) {
            smol_xml_nodeptr_vector_t props;
            smol_vector_init(&props, 32);

            mt.has_tag = "prop";
            smol_xml_find_children(&doc, properties, &mt, &props);

            smol_vector_each(&props, smol_xml_node_t*, proptr) {
                smol_xml_node_t* prop_node = *proptr;

                const char* type = smol_xml_get_attr(prop_node, "type");
                const char* name = smol_xml_get_attr(prop_node, "name");
                const char* value = smol_xml_get_attr(prop_node, "value");
                SMOL_ASSERT(type != NULL);
                SMOL_ASSERT(name != NULL);

                de_prop_t scene_prop = { 0 };
                strcpy(scene_prop.name, name);

                if (strcmp("STRING", type) == 0) {
                    scene_prop.type = DE_PROP_STRING;
                    if (value) strcpy(scene_prop.str_prop, value);
                }
                else if (strcmp("INT", type) == 0) {
                    scene_prop.type = DE_PROP_FLOAT;
                    if (value) scene_prop.int_prop = atoi(value);
                }
                else if (strcmp("FLOAT", type) == 0) {
                    scene_prop.type = DE_PROP_FLOAT;
                    if (value) scene_prop.flt_prop = strtof(value, NULL);
                }
                else if (strcmp("BOOL", type) == 0) {
                    scene_prop.type = DE_PROP_FLOAT;
                    if (value) scene_prop.int_prop = atoi(value);
                }

                smol_vector_push(&scene_obj->properties, scene_prop);
            }

            smol_vector_free(&props);
        }

        // only load script components...
        mt.has_tag = "components";
        smol_xml_node_t* components = smol_xml_find_one_child(&doc, obj, &mt);
        if (components) {
            smol_xml_nodeptr_vector_t comps;
            smol_vector_init(&comps, 32);

            mt.has_tag = "component";
            mt.has_attr = "type";
            mt.attr_equals = "SCRIPT";
            smol_xml_find_children(&doc, components, &mt, &comps);

            smol_vector_each(&comps, smol_xml_node_t*, comptr) {
                smol_xml_node_t* comp_node = *comptr;

                const char* file_path = smol_xml_get_attr(comp_node, "value");
                SMOL_ASSERT(file_path != NULL);

                de_script_t* script = de_scripting_run_script(game->vm, file_path);
                de_component_t comp = { 0 };
                comp.create_func = de_script_component_create_func;
                comp.update_func = de_script_component_update_func;
                comp.destroy_func = de_script_component_destroy_func;
                comp.free_func = NULL;
                comp.data = script;
                
                smol_vector_push(&scene_obj->components, comp);
            }

            smol_vector_free(&comps);
        }
    }

    smol_vector_free(&objects);

    free(collada);
    free(buf);

    uint32_t hash = fnv32_hash(name, strlen(name));
    smol_vector_push(&de_assets.scene_hashes, hash);
    smol_vector_push(&de_assets.scenes, scene);
}

void de_asset_manager_init() {
    smol_vector_init(&de_assets.meshes, 128);
    smol_vector_init(&de_assets.mesh_hashes, 128);
    smol_vector_init(&de_assets.shaders, 128);
    smol_vector_init(&de_assets.shader_hashes, 128);
    smol_vector_init(&de_assets.textures, 128);
    smol_vector_init(&de_assets.texture_hashes, 128);
    smol_vector_init(&de_assets.scripts, 128);
    smol_vector_init(&de_assets.script_hashes, 128);
    smol_vector_init(&de_assets.scenes, 128);
    smol_vector_init(&de_assets.scene_hashes, 128);
}

void de_asset_manager_free() {
    smol_vector_free(&de_assets.meshes);
    smol_vector_free(&de_assets.mesh_hashes);
    smol_vector_free(&de_assets.shaders);
    smol_vector_free(&de_assets.shader_hashes);
    smol_vector_free(&de_assets.textures);
    smol_vector_free(&de_assets.texture_hashes);
    smol_vector_free(&de_assets.scripts);
    smol_vector_free(&de_assets.script_hashes);
    smol_vector_free(&de_assets.scenes);
    smol_vector_free(&de_assets.scene_hashes);
}

size_t de_asset_get_offset(de_index_array_t* vec, uint32_t hash) {
    smol_vector_iterate(vec, i) {
        if (smol_vector_data(vec)[i] == hash) return i;
    }
    return INVALID;
}

de_mesh_t* de_assets_get_mesh(const char* name) {
    uint32_t hash = fnv32_hash(name, strlen(name));
    size_t i = de_asset_get_offset(&de_assets.mesh_hashes, hash);
    if (i != INVALID) {
        return &smol_vector_at(&de_assets.meshes, i);
    }
    return NULL;
}

de_texture_t* de_assets_get_texture(const char* name) {
    uint32_t hash = fnv32_hash(name, strlen(name));
    size_t i = de_asset_get_offset(&de_assets.texture_hashes, hash);
    if (i != INVALID) {
        return &smol_vector_at(&de_assets.textures, i);
    }
    return NULL;
}

de_shader_t* de_assets_get_shader(const char* name) {
    uint32_t hash = fnv32_hash(name, strlen(name));
    size_t i = de_asset_get_offset(&de_assets.shader_hashes, hash);
    if (i != INVALID) {
        return &smol_vector_at(&de_assets.shaders, i);
    }
    return NULL;
}

de_script_t* de_assets_get_script(const char* name) {
    uint32_t hash = fnv32_hash(name, strlen(name));
    size_t i = de_asset_get_offset(&de_assets.script_hashes, hash);
    if (i != INVALID) {
        return &smol_vector_at(&de_assets.scripts, i);
    }
    return NULL;
}

de_scene_t* de_assets_get_scene(const char* name) {
    uint32_t hash = fnv32_hash(name, strlen(name));
    size_t i = de_asset_get_offset(&de_assets.scene_hashes, hash);
    if (i != INVALID) {
        return &smol_vector_at(&de_assets.scenes, i);
    }
    return NULL;
}

int de_assets_has_mesh(const char* name) {
    uint32_t hash = fnv32_hash(name, strlen(name));
    smol_vector_each(&de_assets.mesh_hashes, uint32_t, h) {
        if (*h == hash) return 1;
    }
    return 0;
}

int de_assets_has_texture(const char* name) {
    uint32_t hash = fnv32_hash(name, strlen(name));
    smol_vector_each(&de_assets.texture_hashes, uint32_t, h) {
        if (*h == hash) return 1;
    }
    return 0;
}

int de_assets_has_shader(const char* name) {
    uint32_t hash = fnv32_hash(name, strlen(name));
    smol_vector_each(&de_assets.shader_hashes, uint32_t, h) {
        if (*h == hash) return 1;
    }
    return 0;
}

int de_assets_has_script(const char* name) {
    uint32_t hash = fnv32_hash(name, strlen(name));
    smol_vector_each(&de_assets.script_hashes, uint32_t, h) {
        if (*h == hash) return 1;
    }
    return 0;
}

int de_assets_has_scene(const char* name) {
    uint32_t hash = fnv32_hash(name, strlen(name));
    smol_vector_each(&de_assets.scene_hashes, uint32_t, h) {
        if (*h == hash) return 1;
    }
    return 0;
}
