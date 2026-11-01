#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

namespace fs = std::filesystem;

void scan_apps_in_directory(const fs::path& dir_path);
bool is_launchable_app(const fs::path& file_path);

const char* sendAppName(const char *str) {
    return str;
}

int main() {
    const char* xdg_dirs = std::getenv("XDG_DATA_DIRS");

    if (xdg_dirs == nullptr)
        return 1;

    std::istringstream dirs(xdg_dirs);
    std::string base_dir;

    while (std::getline(dirs, base_dir, ':')) {
        if (base_dir.empty())
            continue;

        fs::path app_dir = fs::path(base_dir) / "applications";

        scan_apps_in_directory(app_dir);
    }

    return 0;
}

void scan_apps_in_directory(const fs::path& dir_path) {
    std::error_code ec;
    fs::directory_iterator it(dir_path, ec);

    if (ec)
        return;

    for (const auto& entry : it) {
        const auto filename = entry.path().filename();

        if (filename == "mimeinfo.cache")
            continue;

        std::error_code file_ec;

        if (!entry.is_regular_file(file_ec))
            continue;

        if (is_launchable_app(entry.path()))
            std::cout << filename.string() << '\n';
            sendAppName(filename.string().c_str());
    }
}

bool is_launchable_app(const fs::path& file_path) {
    std::ifstream file(file_path);

    if (!file)
        return false;

    bool is_app = false;
    bool is_hidden = false;
    bool has_exec = false;

    std::string line;

    while (std::getline(file, line)) {
        if (line.starts_with("Type=Application"))
            is_app = true;

        if (line.starts_with("NoDisplay=true"))
            is_hidden = true;

        if (line.starts_with("Exec="))
            has_exec = true;
    }

    return is_app && !is_hidden && has_exec;
}
