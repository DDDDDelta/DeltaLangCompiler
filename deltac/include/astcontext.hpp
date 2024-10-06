#pragma once

#include "declaration.hpp"
#include "utils.hpp"

#include <vector>

namespace deltac {

class LookupResult {
public:
    LookupResult(Decl* decl) : decl(decl) {}

    void result_decl(Decl* d) {
        decl = d;
    }

    Decl* result_decl() const {
        return decl;
    }

    bool found() const {
        return decl != nullptr;
    }

    bool is_variable() const {
        return found() && util::isinstance<VarDecl>(decl);
    }

    bool is_function() const {
        return found() && util::isinstance<FunctionDecl>(decl);
    }

    bool is_type() const {
        return found() && util::isinstance<TypeDecl>(decl);
    }

private:
    Decl* decl;
};

class ASTContext {
public:
    ASTContext();
    ASTContext(const ASTContext&) = delete;
    ASTContext(ASTContext&&) = delete;

    ~ASTContext() {
        util::cleanup_ptrs(global_vardecls.begin(), global_vardecls.end());
    }

    void register_toplevel_decl(Decl* decl) {
        assert(decl != nullptr);
        
        if (auto* d = dynamic_cast<VarDecl*>(decl)) {
            top_level_vardecls.push_back(d);
        }
        else if (auto* d = dynamic_cast<FuncDecl*>(decl)) {
            top_level_vardecls.push_back(d);
        }
    }

    LookupResult lookup_decl_with_id(std::string_view id) const {
        assert(!id.empty());

        for (VarDecl* vardecl : top_level_vardecls) {
            if (vardecl->get_identifier() == id) {
                return vardecl;
            }
        }

        return nullptr;
    }

public:
    BuiltinType* get_i32_ty() const;
    BuiltinType* get_int_ty_size(u32 bitwidth, bool is_signed) const;
    BuiltinType* get_bool_ty() const;
    BuiltinType* get_void_ty() const;
    BuiltinType* get_void_ptr_ty() const;

    BuiltinType* get_uint_ty(u32 bitwidth) const {
        return get_int_ty_size(bitwidth, false);
    }

    BuiltinType* get_int_ty(u32 bitwidth) const {
        return get_int_ty_size(bitwidth, true);
    }

    BuiltinType* get_builtin_type(BuiltinType::Kind kind) const {
        // BuiltinType is always const
        return const_cast<BuiltinType*>(&builtin_types[kind]);
    }

private:
    std::vector<BuiltinType> builtin_types;
    std::vector<VarDecl*> top_level_vardecls;
    std::vector<FuncDecl*> top_level_funcdecls;
};

BuiltinType* ASTContext::get_i32_ty() const {
    return get_builtin_type(BuiltinType::I32);
}

BuiltinType* ASTContext::get_bool_ty() const {
    return get_builtin_type(BuiltinType::Bool);
}

} // namespace deltac
