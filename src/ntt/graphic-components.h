#pragma once
#include "ntt.h"
#include "../core/math.h"
#include <unordered_map>

namespace sigil {
    namespace ntt {
        /* Light component */
        struct light_component_t {
            sigil::math::v3_t position;
            uint8_t brightness; // 0-255 0% - 100%
            int field_of_view; // in degrees
        };

        extern struct light_component_t_container_t {
            std::vector<light_component_t> objects;
            std::unordered_map<ntt::entity_t, uint32_t> entity_map;
            std::mutex mtx;
        } light_component_store;

        int add_light_component(ntt::entity_t id);
        light_component_t* get_light_component(uint32_t offset);
        light_component_t* entity_to_light_component(ntt::entity_t id);
        
        /* Material component */
        struct material_component_t {
            sigil::asset_t *asset_data;
            sigil::math::v3_t position;

            material_component_t() {
                asset_data = NULL;
                position.x = 0.0f;
                position.y = 0.0f;
                position.z = 0.0f;
            }
        };

        extern struct material_component_container_t {
            std::vector<material_component_t> objects;
            std::unordered_map<ntt::entity_t, uint32_t> entity_map;
            std::mutex mtx;
        } material_component_store;

        material_component_t* get_material_component(uint32_t offset);
        material_component_t* entity_to_mesh_component(ntt::entity_t id);
        int add_mesh_component(sigil::ntt::entity_t id, sigil::asset_t asset);
        
        /* Texture component */
        struct texture_component_t {
            sigil::asset_t *data;
            texture_component_t() {
                data = NULL;
            }
        };

        extern struct texture_component_container_t {
            std::vector<texture_component_t> objects;
            std::unordered_map<ntt::entity_t, uint32_t> entity_map;
            std::mutex mtx;
        } texture_component_store;

        int add_texture_component(ntt::entity_t id);
        texture_component_t* get_texture_component(uint32_t offset);
        texture_component_t* entity_to_texture_component(ntt::entity_t id);
        
        /* Static mesh component */
        struct static_mesh_component_t {
            sigil::asset_t *asset_data;
            sigil::math::v3_t position;

            static_mesh_component_t() {
                asset_data = NULL;
                position.x = 0.0f;
                position.y = 0.0f;
                position.z = 0.0f;
            }
        };

        extern struct static_mesh_component_container_t {
            std::vector<static_mesh_component_t> objects;
            std::unordered_map<ntt::entity_t, uint32_t> entity_map;
            std::mutex mtx;
        } static_mesh_component_store;

        static_mesh_component_t* get_static_mesh_component(uint32_t offset);
        static_mesh_component_t* entity_to_static_mesh_component(ntt::entity_t id);
        int add_static_mesh_component(sigil::ntt::entity_t id, sigil::asset_t asset);

        /* Sprite component */
        struct sprite_component_t {
            sigil::asset_t *data;
            sprite_component_t() {
                
            }
        };

        extern struct sprite_component_container_t {
            std::vector<sprite_component_t> objects;
            std::unordered_map<ntt::entity_t, uint32_t> entity_map;
            std::mutex mtx;
        } sprite_component_store;

        int add_sprite_component(ntt::entity_t id);
        sprite_component_t* get_sprite_component(uint32_t offset);
        sprite_component_t* entity_to_sprite_component(ntt::entity_t id);

        /* Dynamic mesh component */
        struct dynamic_mesh_component_t {
            sigil::asset_t *asset_data;
            sigil::math::v3_t position;
            sigil::math::transform3d transform;

            dynamic_mesh_component_t() {
                asset_data = NULL;
                position.x = 0.0f;
                position.y = 0.0f;
                position.z = 0.0f;
                transform.rotation.r = 0.0f;
                transform.rotation.x = 0.0f;
                transform.rotation.y = 0.0f;
                transform.rotation.z = 0.0f;
                transform.translation.x = 0.0f;
                transform.translation.y = 0.0f;
                transform.translation.z = 0.0f;
                transform.scale.x = 1.0f;
                transform.scale.y = 1.0f;
                transform.scale.z = 1.0f;
            }
        };

        extern struct dynamic_mesh_component_t_container_t {
            std::vector<dynamic_mesh_component_t> objects;
            std::unordered_map<ntt::entity_t, uint32_t> entity_map;
            std::mutex mtx;
        } dynamic_mesh_component_store;

        int add_dynamic_mesh_component(sigil::ntt::entity_t id, sigil::asset_t asset);
        dynamic_mesh_component_t* get_dynamic_mesh_component(uint32_t offset);
        dynamic_mesh_component_t* entity_to_dynamic_mesh_component(ntt::entity_t id);

        /* Camera component */
        struct camera_component_t {
            sigil::math::v3_t position;
            sigil::math::transform3d transform;
            uint8_t detail_level; // 0-255 0% - 100%
            // TODO: compose in camera from external lib or implement it
        };

        extern struct camera_component_t_container_t {
            std::vector<camera_component_t> objects;
            std::unordered_map<ntt::entity_t, uint32_t> entity_map;
            std::mutex mtx;
        } camera_component_store;

        int add_camera_component(ntt::entity_t id);
        inline camera_component_t* get_camera_component(uint32_t offset);
        inline camera_component_t* entity_to_camera_component(ntt::entity_t id);
    }
}