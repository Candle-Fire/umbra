#include "runtime/assembly.h"
#include "spdlog/spdlog.h"

Runtime::Assembly::~Assembly() {

}

bool Runtime::Assembly::HasSymbol(const std::string &name) const {
    if (!this->lib || name.empty())
        return false;
    return Native::locate_symbol(this->lib, name.c_str()) != nullptr;
}
const SH::Path &Runtime::Assembly::GetID() const {
    return this->id;
}
bool Runtime::Assembly::IsLoaded() const {
    return this->lib != nullptr;
}
void Runtime::Assembly::Load() {
    this->lib = Native::open(this->path.c_str());

    if (!this->lib) {
        spdlog::error("Could not load library \"{}\"\n{}", this->path, Native::get_error_description());
        //throw load_error("Could not load library \"" + this->path + "\"\n" + Native::get_error_description());
    }
}
