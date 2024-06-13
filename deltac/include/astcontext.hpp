#pragma once

#include "decl.hpp"

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
        return decl == nullptr;
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
    ASTContext() = default;
    ASTContext(const ASTContext&) = delete;
    ASTContext(ASTContext&&) = delete;

    ~ASTContext() {
        util::cleanup_ptrs(global_vardecls.begin(), global_vardecls.end());
    }

    void register_global_vardecl(VarDecl* vardecl) {
        assert(vardecl != nullptr);
        
        if (vardecl->has_body()) {
            vardecl->reset_expr();
        }

        global_vardecls.push_back(vardecl);
    }

    LookupResult lookup_decl_with_id(std::string_view id) const {
        assert(!id.empty());

        for (VarDecl* vardecl : global_vardecls) {
            if (vardecl->get_identifier() == id) {
                return vardecl;
            }
        }

        return nullptr;
    }

public:
    QualType get_i32_ty(bool is_const = false) const;
    QualType get_int_ty_from_size(std::uint32_t bitwidth, bool is_signed, bool is_const = false) const;
    QualType get_bool_ty(bool is_const = false) const;
    QualType get_void_ty() const;
    QualType get_void_ptr_ty(bool is_const = false) const;

private:
    static BuiltinType builtin_types[];

    static BuiltinType* get_builtin_type(BuiltinType::Kind kind) {
        return &builtin_types[kind];
    }

private:
    std::vector<VarDecl*> global_vardecls;
    std::vector<FuncDecl*> global_funcdecls;
};

} // namespace deltac
