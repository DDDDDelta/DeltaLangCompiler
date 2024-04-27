#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <cassert>
#include <unordered_set>
#include <compare>

#include "llvm/Support/Casting.h"

class BasicTypeManager;

class BasicType {
public:
    static BasicType* void_ty;

public:
    explicit BasicType(std::string name) : name(std::move(name)) {}
    BasicType(const BasicType&) = delete;
    BasicType(BasicType&&) = delete;

public:
    std::string_view repr() const {
        return name;
    }

    friend bool operator ==(const BasicType& lhs, const BasicType& rhs) = default;

private:
    std::string name;
};

inline std::strong_ordering operator <=>(const BasicType& lhs, const BasicType& rhs) {
    return lhs.repr() <=> rhs.repr();
}

// TODO: refactor needed since BasicType will be polymorphic
class BasicTypeManager {
public:
    BasicTypeManager();

    bool has_type(BasicType* ty) const {
        return table.find(*ty) != table.end();
    }

    bool has_type_name(std::string_view name) const {
        BasicType tempty((std::string)name);
        return has_type(&tempty);
    }

    BasicType* register_type(std::string_view name) {
        return const_cast<BasicType*>(&*table.emplace((std::string)name).first);
    }

    BasicType* get_type_name(std::string_view name) {
        return const_cast<BasicType*>(&*table.find((BasicType)(std::string)name));
    }

private:
    std::unordered_set<BasicType> table;
};



class Type {
private:
    class QualType;
    class PtrType;
    class ConcreteType;

public:
    Type() : type(new ConcreteType(BasicType::void_ty, false)), conctype((ConcreteType*)type) {}

    Type(BasicType* basictype, bool constness = false) : 
        type(new ConcreteType(basictype, constness)), conctype((ConcreteType*)type) {}

    Type(const Type& other) : compoundlv(other.compoundlv) {
        conctype = new ConcreteType(other.conctype->basic_ty(), other.conctype->constness());
        type = other.type->copy(conctype);
    }

    Type(Type&& other) : 
        type(other.type), conctype(other.conctype), compoundlv(other.compoundlv) {
        other.type = nullptr;
        other.conctype = nullptr;
    }

    Type& operator =(const Type& rhs) {
        if (&rhs == this) {
            return *this;
        }

        Type newty = rhs;
        swap(newty);
        return *this;
    }

    Type& operator =(Type&& rhs) {
        if (&rhs == this) {
            return *this;
        }

        swap(rhs);
        return *this;
    }

    void swap(Type& other) {
        std::swap(type, other.type);
        std::swap(conctype, other.conctype);
        std::swap(compoundlv, other.compoundlv);
    }

    ~Type() { delete type; }

public:
    std::string repr() const {
        return type->repr();
    }

    bool constness() const {
        return type->constness();
    }

    bool is_compound_ty() const {
        return llvm::isa<CompoundType>(type);
    }

    bool is_ptr_ty() const {
        return llvm::isa<PtrType>(type);
    }

    void remove_ptr() {
        assert(is_ptr_ty());
        auto* garbage = type;

        type = ((CompoundType*)type)->release();
        compoundlv--;

        delete garbage;
    }

    void add_ptr(bool constness = false) {
        type = new PtrType(type, constness);
        compoundlv++;
    }

    BasicType* get_basic_ty() const {
        return conctype->basic_ty();
    }

    void set_basic_ty(BasicType* type) {
        conctype->reset(type);
    }

    std::size_t get_compound_lv() const {
        return compoundlv;
    }

    friend bool operator ==(const Type& lhs, const Type& rhs) {
        if (*lhs.conctype->basic_ty() != *rhs.conctype->basic_ty())
            return false;
        
        // we only have ptrty currently
        return lhs.compoundlv == rhs.compoundlv;
    }

private:
    class QualType {
    public:
        QualType(bool constness) : is_const(constness) {}
        QualType(const QualType&) = delete;
        QualType(QualType&&) = delete;
        virtual ~QualType() = default;
        
        std::string repr() const {
            return get_name() + (is_const ? " const" : "");
        }

        bool constness() const {
            return is_const;
        }

        [[nodiscard]] 
        virtual QualType* copy(ConcreteType* concty) const = 0;

    protected:
        virtual std::string get_name() const = 0;
        friend class PtrType;

    private:
        bool is_const;
    };

    class CompoundType : public QualType {
    public:
        CompoundType(QualType* type, bool constness) : QualType(constness), type_under(type) {}
        ~CompoundType() override {
            delete type_under;
        }

        QualType* release() noexcept {
            QualType* ret = type_under;
            type_under = nullptr;
            return ret;
        }

    protected:
        QualType* type_under;
    };

    class PtrType : public CompoundType {
    public:
        PtrType(QualType* type, bool constness) : CompoundType(type, constness) {
            assert(type != nullptr);
        }
        ~PtrType() override = default;

        [[nodiscard]] QualType* copy(ConcreteType* concty) const override {
            return new PtrType(type_under->copy(concty), constness());
        }

    protected:
        std::string get_name() const override { return "*" + type_under->get_name(); }
    };

    class ConcreteType : public QualType {
    public:
        ConcreteType(BasicType* type, bool constness) : QualType(constness), type(type) {}
        ~ConcreteType() override = default;

        void reset(BasicType* newty) {
            assert(newty != nullptr);

            type = newty;
        }

        BasicType* basic_ty() const {
            return type;
        }

        [[nodiscard]] QualType* copy(ConcreteType* concty) const override {
            return concty;
        }

    protected:
        std::string get_name() const override { return (std::string)type->repr(); }

    private:
        BasicType* type; // do not take ownership
    };

private:
    QualType* type;
    ConcreteType* conctype;
    std::size_t compoundlv = 0;
};
