#include "game-components.h"
#include "general-components.h"
#include "graphic-components.h"

sigil::ntt::pawn_component_container_t sigil::ntt::pawn_component_store;
sigil::ntt::name_component_container_t sigil::ntt::name_component_store;
sigil::ntt::script_component_container_t sigil::ntt::script_component_store;
sigil::ntt::controller_component_t_container_t sigil::ntt::controller_component_store;
sigil::ntt::light_component_t_container_t sigil::ntt::light_component_store;
sigil::ntt::material_component_container_t sigil::ntt::material_component_store;
sigil::ntt::dynamic_mesh_component_t_container_t sigil::ntt::dynamic_mesh_component_store;
sigil::ntt::static_mesh_component_container_t sigil::ntt::static_mesh_component_store;
sigil::ntt::camera_component_t_container_t sigil::ntt::camera_component_store;
sigil::ntt::sprite_component_container_t sigil::ntt::sprite_component_store;
sigil::ntt::texture_component_container_t sigil::ntt::texture_component_store;

sigil::ntt::pawn_component_t* sigil::ntt::get_pawn_component(uint32_t offset) {
    return &pawn_component_store.objects[offset];
}

sigil::ntt::pawn_component_t* sigil::ntt::entity_to_pawn_component(ntt::entity_t id) {
    uint32_t offset = pawn_component_store.entity_map[id];
    return get_pawn_component(offset);
}

int sigil::ntt::add_pawn_component(ntt::entity_t id) {
    pawn_component_store.mtx.lock();
    pawn_component_store.objects.push_back(pawn_component_t());
    pawn_component_store.entity_map[id] = pawn_component_store.objects.size() - 1;
    pawn_component_store.mtx.unlock();
    return 0;
}

sigil::ntt::name_component_t* sigil::ntt::get_name_component(uint32_t offset) {
    return &name_component_store.objects[offset];
}

sigil::ntt::name_component_t* sigil::ntt::entity_to_name_component(ntt::entity_t id) {
    uint32_t offset = name_component_store.entity_map[id];
    return get_name_component(offset);
}

int sigil::ntt::add_name_component(ntt::entity_t id, std::string name) {
    name_component_store.mtx.lock();
    name_component_store.objects.push_back(name_component_t(name));
    name_component_store.entity_map[id] = name_component_store.objects.size() - 1;
    name_component_store.mtx.unlock();
    return 0;
}

sigil::ntt::script_component_t* sigil::ntt::get_script_component(uint32_t offset) {
    return &script_component_store.objects[offset];
}

sigil::ntt::script_component_t* sigil::ntt::entity_to_script_component(ntt::entity_t id) {
    uint32_t offset = script_component_store.entity_map[id];
    return get_script_component(offset);
}

int sigil::ntt::add_script_component(ntt::entity_t id) {
    script_component_store.mtx.lock();
    script_component_store.objects.push_back(script_component_t());
    script_component_store.entity_map[id] = script_component_store.objects.size() - 1;
    script_component_store.mtx.unlock();
    return 0;
}

int sigil::ntt::add_controller_component(ntt::entity_t id) {
    controller_component_store.mtx.lock();
    controller_component_store.objects.push_back(controller_component_t());
    controller_component_store.entity_map[id] = controller_component_store.objects.size() - 1;
    controller_component_store.mtx.unlock();
    return 0;
}

sigil::ntt::controller_component_t* sigil::ntt::get_controller_component(uint32_t offset) {
    return &controller_component_store.objects[offset];
}

sigil::ntt::controller_component_t* sigil::ntt::entity_to_controller_component(ntt::entity_t id) {
    uint32_t offset = controller_component_store.entity_map[id];
    return get_controller_component(offset);
}

sigil::ntt::light_component_t* sigil::ntt::get_light_component(uint32_t offset) {
    return &light_component_store.objects[offset];
}

sigil::ntt::light_component_t* sigil::ntt::entity_to_light_component(ntt::entity_t id) {
    uint32_t offset = light_component_store.entity_map[id];
    return get_light_component(offset);
}

int sigil::ntt::add_light_component(ntt::entity_t id) {
    light_component_store.mtx.lock();
    light_component_store.objects.push_back(light_component_t());
    light_component_store.entity_map[id] = light_component_store.objects.size() - 1;
    light_component_store.mtx.unlock();
    return 0;
}

sigil::ntt::material_component_t* sigil::ntt::get_material_component(uint32_t offset) {
    return &material_component_store.objects[offset];
}

sigil::ntt::material_component_t* sigil::ntt::entity_to_mesh_component(ntt::entity_t id) {
    uint32_t offset = material_component_store.entity_map[id];
    return get_material_component(offset);
}

int sigil::ntt::add_mesh_component(sigil::ntt::entity_t id, sigil::asset_t asset) {
    material_component_store.mtx.lock();
    material_component_store.objects.push_back(material_component_t());
    material_component_store.entity_map[id] = material_component_store.objects.size() - 1;
    material_component_store.mtx.unlock();
    return 0;
}

sigil::ntt::texture_component_t* sigil::ntt::get_texture_component(uint32_t offset) {
    return &texture_component_store.objects[offset];
}

sigil::ntt::texture_component_t* sigil::ntt::entity_to_texture_component(ntt::entity_t id) {
    uint32_t offset = texture_component_store.entity_map[id];
    return get_texture_component(offset);
}

int sigil::ntt::add_texture_component(ntt::entity_t id) {
    texture_component_store.mtx.lock();
    texture_component_store.objects.push_back(texture_component_t());
    texture_component_store.entity_map[id] = texture_component_store.objects.size() - 1;
    texture_component_store.mtx.unlock();
    return 0;
}

sigil::ntt::static_mesh_component_t* sigil::ntt::get_static_mesh_component(uint32_t offset) {
    return &static_mesh_component_store.objects[offset];
}

sigil::ntt::static_mesh_component_t* sigil::ntt::entity_to_static_mesh_component(ntt::entity_t id) {
    uint32_t offset = static_mesh_component_store.entity_map[id];
    return get_static_mesh_component(offset);
}

int sigil::ntt::add_static_mesh_component(sigil::ntt::entity_t id, sigil::asset_t asset) {
    static_mesh_component_store.mtx.lock();
    static_mesh_component_store.objects.push_back(static_mesh_component_t());
    static_mesh_component_store.entity_map[id] = static_mesh_component_store.objects.size() - 1;
    static_mesh_component_store.mtx.unlock();
    return 0;
}

sigil::ntt::sprite_component_t* sigil::ntt::get_sprite_component(uint32_t offset) {
    return &sprite_component_store.objects[offset];
}

sigil::ntt::sprite_component_t* sigil::ntt::entity_to_sprite_component(ntt::entity_t id) {
    uint32_t offset = sprite_component_store.entity_map[id];
    return get_sprite_component(offset);
}

int sigil::ntt::add_sprite_component(ntt::entity_t id) {
    sprite_component_store.mtx.lock();
    sprite_component_store.objects.push_back(sprite_component_t());
    sprite_component_store.entity_map[id] = sprite_component_store.objects.size() - 1;
    sprite_component_store.mtx.unlock();
    return 0;
}

sigil::ntt::dynamic_mesh_component_t* sigil::ntt::get_dynamic_mesh_component(uint32_t offset) {
    return &dynamic_mesh_component_store.objects[offset];
}

sigil::ntt::dynamic_mesh_component_t* sigil::ntt::entity_to_dynamic_mesh_component(ntt::entity_t id) {
    uint32_t offset = dynamic_mesh_component_store.entity_map[id];
    return get_dynamic_mesh_component(offset);
}

int sigil::ntt::add_dynamic_mesh_component(sigil::ntt::entity_t id, sigil::asset_t asset) {
    dynamic_mesh_component_store.mtx.lock();
    dynamic_mesh_component_store.objects.push_back(dynamic_mesh_component_t());
    dynamic_mesh_component_store.entity_map[id] = dynamic_mesh_component_store.objects.size() - 1;
    dynamic_mesh_component_store.mtx.unlock();
    return 0;
}

sigil::ntt::camera_component_t* sigil::ntt::get_camera_component(uint32_t offset) {
    return &camera_component_store.objects[offset];
}

sigil::ntt::camera_component_t* sigil::ntt::entity_to_camera_component(ntt::entity_t id) {
    uint32_t offset = camera_component_store.entity_map[id];
    return get_camera_component(offset);
}

int sigil::ntt::add_camera_component(ntt::entity_t id) {
    camera_component_store.mtx.lock();
    camera_component_store.objects.push_back(camera_component_t());
    camera_component_store.entity_map[id] = camera_component_store.objects.size() - 1;
    camera_component_store.mtx.unlock();
    return 0;
}



