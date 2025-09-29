#pragma once

#include <thaumaturgy/thaumaturgy.h>
#include <filesystem>
#include <string>
#include <vector>

namespace sigil {

struct project_metadata_t {
    std::string name;
    std::string author;
    std::string version;
    std::string description;
    std::filesystem::path root_path;
    std::filesystem::path assets_path;
    std::filesystem::path config_path;
    std::filesystem::path build_path;
};

class project_t {
public:
    project_t() = default;

    ::thaumaturgy::yield load(const std::filesystem::path& path);
    ::thaumaturgy::yield save() const;
    ::thaumaturgy::yield create(const std::string& name, const std::filesystem::path& root_dir);

    const project_metadata_t& meta() const { return metadata; }

private:
    project_metadata_t metadata;
};

class project_manager_t {
public:
    // Root directory where all projects live (usually ~/Projects/Sigil)
    explicit project_manager_t(std::filesystem::path root);

    ::thaumaturgy::yield discover_projects(std::vector<project_metadata_t>& out);
    ::thaumaturgy::yield create_project(const std::string& name, const std::string& author);
    ::thaumaturgy::yield delete_project(const std::string& name);
    ::thaumaturgy::yield open_project(const std::string& name, project_t& out_project);

private:
    std::filesystem::path root_dir;
};

} // namespace sigil
