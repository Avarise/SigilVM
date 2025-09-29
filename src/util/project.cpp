#include "sigil/status.h"
#include <sigil/util/project.h>
#include <fstream>
#include <iostream>
#include <cstdlib>  // for getenv

namespace fs = std::filesystem;
namespace sigil {

// Helper: expand "~" to user home
static fs::path expand_home(const fs::path& path) {
    if (!path.empty() && path.string().starts_with("~")) {
        const char* home = std::getenv("HOME");
        if (home)
            return fs::path(home) / path.string().substr(1);
    }
    return path;
}

// Helper: generate default project root (~/Projects/Sigil)
static fs::path default_project_root() {
    const char* home = std::getenv("HOME");
    if (!home)
        return fs::path("./Projects");
    return fs::path(home) / "Projects";
}

// ============================================================================
// project_t implementation
// ============================================================================

status_t project_t::create(const std::string& name, const fs::path& root_dir) {
    if (name.empty())
        return VM_ARG_INVALID;

    fs::path project_root = root_dir / name;
    if (fs::exists(project_root))
        return VM_ALREADY_EXISTS;

    try {
        fs::create_directories(project_root / "assets");
        fs::create_directories(project_root / "scenes");
        fs::create_directories(project_root / "scripts");
        fs::create_directories(project_root / "build");
    } catch (...) {
        return VM_IO_ERROR;
    }

    metadata.name = name;
    metadata.author = std::getenv("USER") ? std::getenv("USER") : "unknown";
    metadata.version = "0.1.0";
    metadata.description = "New Sigil project";
    metadata.root_path = project_root;
    metadata.assets_path = project_root / "assets";
    metadata.config_path = project_root / "configs";
    metadata.build_path = project_root / "build";

    // Create configs folder
    fs::create_directories(metadata.config_path);

    // Write JSON metadata
    fs::path json_path = project_root / "project.json";
    std::ofstream ofs(json_path);
    if (!ofs.is_open())
        return VM_IO_ERROR;

    ofs << "{\n"
        << "  \"name\": \"" << metadata.name << "\",\n"
        << "  \"author\": \"" << metadata.author << "\",\n"
        << "  \"version\": \"" << metadata.version << "\",\n"
        << "  \"description\": \"" << metadata.description << "\",\n"
        << "  \"assets_path\": \"" << metadata.assets_path.filename().string() << "\",\n"
        << "  \"config_path\": \"" << metadata.config_path.filename().string() << "\",\n"
        << "  \"build_path\": \"" << metadata.build_path.filename().string() << "\"\n"
        << "}\n";

    ofs.close();
    return VM_OK;
}

status_t project_t::load(const fs::path& path) {
    fs::path json_path = path / "project.json";
    if (!fs::exists(json_path))
        return VM_RESOURCE_MISSING;

    std::ifstream ifs(json_path);
    if (!ifs.is_open())
        return VM_IO_ERROR;

    std::string key, value, line;
    while (std::getline(ifs, line)) {
        if (line.find("\"name\"") != std::string::npos)
            metadata.name = line.substr(line.find(":") + 2);
        else if (line.find("\"author\"") != std::string::npos)
            metadata.author = line.substr(line.find(":") + 2);
        else if (line.find("\"version\"") != std::string::npos)
            metadata.version = line.substr(line.find(":") + 2);
        else if (line.find("\"description\"") != std::string::npos)
            metadata.description = line.substr(line.find(":") + 2);
    }

    metadata.root_path = path;
    metadata.assets_path = path / "assets";
    metadata.config_path = path / "configs";
    metadata.build_path = path / "build";
    return VM_OK;
}

status_t project_t::save() const {
    if (metadata.name.empty())
        return VM_ARG_INVALID;

    fs::path json_path = metadata.root_path / "project.json";
    std::ofstream ofs(json_path);
    if (!ofs.is_open())
        return VM_IO_ERROR;

    ofs << "{\n"
        << "  \"name\": \"" << metadata.name << "\",\n"
        << "  \"author\": \"" << metadata.author << "\",\n"
        << "  \"version\": \"" << metadata.version << "\",\n"
        << "  \"description\": \"" << metadata.description << "\",\n"
        << "  \"assets_path\": \"" << metadata.assets_path.filename().string() << "\",\n"
        << "  \"config_path\": \"" << metadata.config_path.filename().string() << "\",\n"
        << "  \"build_path\": \"" << metadata.build_path.filename().string() << "\"\n"
        << "}\n";

    ofs.close();
    return VM_OK;
}

// ============================================================================
// project_manager_t implementation
// ============================================================================

project_manager_t::project_manager_t(fs::path root)
    : root_dir(expand_home(root.empty() ? default_project_root() : root))
{
    try {
        fs::create_directories(root_dir);
    } catch (...) {
        std::cerr << "Warning: cannot create project root at " << root_dir << "\n";
    }
}

status_t project_manager_t::discover_projects(std::vector<project_metadata_t>& out) {
    out.clear();
    if (!fs::exists(root_dir))
        return VM_RESOURCE_MISSING;

    for (auto& entry : fs::directory_iterator(root_dir)) {
        if (!entry.is_directory())
            continue;

        fs::path proj_json = entry.path() / "project.json";
        if (fs::exists(proj_json)) {
            project_t temp;
            if (temp.load(entry.path()) == VM_OK)
                out.push_back(temp.meta());
        }
    }
    return out.empty() ? VM_RESOURCE_MISSING : VM_OK;
}

status_t project_manager_t::create_project(const std::string& name, const std::string& author) {
    if (name.empty())
        return VM_ARG_INVALID;

    project_t proj;
    status_t st = proj.create(name, root_dir);
    if (st == VM_OK) {
        // Optionally override author if provided
        if (!author.empty()) {
            project_metadata_t meta = proj.meta();
            meta.author = author;
            proj.save();
        }
    }
    return st;
}

status_t project_manager_t::delete_project(const std::string& name) {
    fs::path proj_path = root_dir / name;
    if (!fs::exists(proj_path))
        return VM_RESOURCE_MISSING;

    try {
        fs::remove_all(proj_path);
    } catch (...) {
        return VM_IO_ERROR;
    }
    return VM_OK;
}

status_t project_manager_t::open_project(const std::string& name, project_t& out_project) {
    fs::path proj_path = root_dir / name;
    if (!fs::exists(proj_path))
        return VM_RESOURCE_MISSING;

    return out_project.load(proj_path);
}

} // namespace sigil
