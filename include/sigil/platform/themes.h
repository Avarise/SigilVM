#pragma once
#include <sigil/status.h>
#include <string>
#include <vector>

namespace sigil::util {

/**
 * @brief 
 * List all .yaml templates from /usr/share/sigilvm/themes
 * (optionally from directly from repo when prototyping)
 * @param out 
 * Vector to be filled with found names
 * @return status_t 
 */
status_t list_templates(std::vector<std::string> &out);

/**
 * @brief 
 * List all built themes in ~/.local/share/sigilvm/themes
 * @param out
 * Vector to be filled with found names 
 * @return status_t 
 */
status_t list_themes(std::vector<std::string> &out);

/**
 * @brief Arrange templates and a <name>.yaml theme config
 * into a theme located in ~/.local/share/sigilvm/themes/<name>
 * @param name 
 * @return status_t 
 */
status_t build_theme(std::string name);

/**
 * @brief Deploy a theme from ~/.local to ~/.config
 * 
 * @param name 
 * @return status_t 
 */
status_t deploy_theme(std::string name);

/**
 * @brief 
 * Create or update ~/.local/share/sigilvm/themes/manifest.yaml
 * with hash values generated during theme build step.
 * Used to detect if generated themes were modified by user.
 * @return status_t 
 * VM_RESOURCE_MISSING if there are no themes found in the ~/.local
 * VM_OK on success
 */
status_t update_theme_manifest();


/**
 * @brief 
 * Attempt to reset every desktop component
 * @return status_t 
 */
status_t reload_desktop_components();


} // namespace sigil::util