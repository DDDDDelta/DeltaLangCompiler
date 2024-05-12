#include "decl.hpp"

#include <vector>

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
        return found() && isinstance<VarDecl>(decl);
    }

    bool is_function() const {
        return found() && isinstance<FunctionDecl>(decl);
    }

    bool is_type() const {
        return found() && isinstance<TypeDecl>(decl);
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
        cleanup_ptrs(global_vardecls);
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

private:
    std::vector<VarDecl*> global_vardecls;
    std::vector<
};

