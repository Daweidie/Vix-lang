#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <cstdlib>

bool iamroot() {
    return getuid() == 0;
}

std::string osrelease() {
    std::ifstream file("/etc/os-release");
    if (!file.is_open()) return "";

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        size_t eq_pos = line.find('=');
        if (eq_pos == std::string::npos) continue;

        std::string k = line.substr(0, eq_pos);
        if (k != "ID") continue;

        std::string v = line.substr(eq_pos + 1);
        if (v.size() >= 2 && v.front() == '"' && v.back() == '"')
            v = v.substr(1, v.size() - 2);

        return v;
    }
    return "";
}

int main() {
    std::string distro_id = osrelease();

    if (distro_id.empty()) {
        std::cerr << "\033[31mCannot detect the distribution ID. Contact the repository owner or script author Cero0xA672@outlook.com ask for help.\033[0m\n";
        return 1;
    }

    std::cout << "\033[34mYour device's distribution ID: " << distro_id << "\033[0m" << std::endl;

    std::string Install;
    if (distro_id == "ubuntu" || distro_id == "debian" || distro_id == "linuxmint" || distro_id == "pop" || distro_id == "kali") {
        Install = "apt install -y gcc flex bison llvm clang-18 libclang-18-dev git";
    } else if (distro_id == "arch" || distro_id == "manjaro" || distro_id == "endeavouros") {
        Install = "pacman -S --noconfirm gcc flex bison llvm clang git";
    } else if (distro_id == "fedora") {
        Install = "dnf install -y gcc flex bison llvm clang git";
    } else if (distro_id == "rhel" || distro_id == "centos" || distro_id == "rocky" || distro_id == "almalinux") {
        Install = "dnf install -y gcc flex bison llvm clang git";
    } else if (distro_id == "opensuse" || distro_id == "opensuse-leap" || distro_id == "opensuse-tumbleweed" || distro_id == "suse") {
        Install = "zypper install -y gcc flex bison llvm clang git";
    } else if (distro_id == "alpine") {
        Install = "apk add gcc flex bison llvm clang git";
    } else if (distro_id == "gentoo") {
        Install = "emerge -v gcc flex bison llvm clang git";
    } else if (distro_id == "void") {
        Install = "xbps-install -y gcc flex bison llvm clang git";
    } else {
        std::cerr << "\033[31mUnsupported distribution: " << distro_id << "\033[0m" << std::endl;
        return 1;
    }

    std::string cmd;
    if (iamroot()) {
        cmd = Install;
    } else {
        cmd = "sudo " + Install;
    }

    std::cout << "\033[32mExecuting: " << cmd << "\033[0m" << std::endl;
    int ret = system(cmd.c_str());
    if (ret != 0) {
        std::cerr << "\033[31mInstallation failed with exit code " << ret << "\033[0m" << std::endl;
        return 1;
    }

    std::cout << "\033[32mInstallation successful!\033[0m" << std::endl;
    return 0;
}
