#pragma once
#include <filesystem>
#include <thaumaturgy/thaumaturgy.h>
#include <string>
#include <vector>

namespace sigil::desktop {

/**
 * @brief 
 * List all .yaml templates from /usr/share/sigilvm/themes
 * (optionally from directly from repo when prototyping)
 * @param out 
 * Vector to be filled with found names
 * @return ::thaumaturgy::yield
 */
::thaumaturgy::yield list_templates(std::vector<std::string> &out);

/**
 * @brief 
 * List all built themes in ~/.local/share/sigilvm/themes
 * @param out
 * Vector to be filled with found names 
 * @return ::thaumaturgy::yield
 */
::thaumaturgy::yield list_themes(std::vector<std::string> &out);

/**
 * @brief Arrange templates and a <name>.yaml theme config
 * into a theme located in ~/.local/share/sigilvm/themes/<name>
 * @param name 
 * @return ::thaumaturgy::yield
 */
::thaumaturgy::yield build_theme(std::string name);

/**
 * @brief Deploy a theme from ~/.local to ~/.config
 * 
 * @param name 
 * @return ::thaumaturgy::yield
 */
::thaumaturgy::yield deploy_theme(std::string name);


/**
 * @brief Deploy a theme from ~/.local to ~/.config
 * 
 * @param name 
 * @return ::thaumaturgy::yield
 */
::thaumaturgy::yield deploy_theme_from_path(std::filesystem::path theme_dir);


/**
 * @brief 
 * Create or update ~/.local/share/sigilvm/themes/manifest.yaml
 * with hash values generated during theme build step.
 * Used to detect if generated themes were modified by user.
 * @return ::thaumaturgy::yield
 * VM_RESOURCE_MISSING if there are no themes found in the ~/.local
 * VM_OK on success
 */
::thaumaturgy::yield update_theme_manifest();


/**
 * @brief 
 * Attempt to reset every desktop component
 * @return ::thaumaturgy::yield
 */
::thaumaturgy::yield reload_components();


} // namespace sigil::desktop