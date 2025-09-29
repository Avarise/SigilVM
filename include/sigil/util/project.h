#pragma once

#include <sigil/status.h>
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

    status_t load(const std::filesystem::path& path);
    status_t save() const;
    status_t create(const std::string& name, const std::filesystem::path& root_dir);

    const project_metadata_t& meta() const { return metadata; }

private:
    project_metadata_t metadata;
};

class project_manager_t {
public:
    // Root directory where all projects live (usually ~/Projects/Sigil)
    explicit project_manager_t(std::filesystem::path root);

    status_t discover_projects(std::vector<project_metadata_t>& out);
    status_t create_project(const std::string& name, const std::string& author);
    status_t delete_project(const std::string& name);
    status_t open_project(const std::string& name, project_t& out_project);

private:
    std::filesystem::path root_dir;
};

} // namespace sigil
