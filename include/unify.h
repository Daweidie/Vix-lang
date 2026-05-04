#ifndef VIX_UNIFY_H
#define VIX_UNIFY_H

#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "type.h"

class Unifier {
public:
	Unifier() : next_var_id_(1) {}

	TypePtr fresh() {
		return Type::make_var(next_var_id_++);
	}

	TypePtr apply(const TypePtr& t) {
		if (!t) {
			return t;
		}

		switch (t->kind) {
			case TypeKind::Var: {
				auto it = subst_.find(t->data.var.id);
				if (it == subst_.end()) {
					return t;
				}
				TypePtr resolved = apply(it->second);
				it->second = resolved;
				return resolved;
			}
			case TypeKind::Ptr:
				return Type::make_ptr(apply(t->data.ptr.pointee));
			case TypeKind::Array:
				return Type::make_array(apply(t->data.array.element));
			case TypeKind::FixedArray:
				return Type::make_fixed_array(apply(t->data.fixed_array.element), t->data.fixed_array.size);
			case TypeKind::App: {
				std::vector<TypePtr> args;
				args.reserve(t->data.app.args.size());
				for (const auto& arg : t->data.app.args) {
					args.push_back(apply(arg));
				}
				return Type::make_app(apply(t->data.app.ctor), std::move(args));
			}
			case TypeKind::Fn: {
				std::vector<TypePtr> params;
				params.reserve(t->data.fn.params.size());
				for (const auto& param : t->data.fn.params) {
					params.push_back(apply(param));
				}
				return Type::make_fn(std::move(params), apply(t->data.fn.ret),
									 t->data.fn.generic_param_ids, t->data.fn.vararg);
			}
			case TypeKind::Tuple: {
				std::vector<TypePtr> elems;
				elems.reserve(t->data.tuple.elements.size());
				for (const auto& elem : t->data.tuple.elements) {
					elems.push_back(apply(elem));
				}
				return Type::make_tuple(std::move(elems));
			}
			default:
				return t;
		}
	}

	void unify(const TypePtr& lhs, const TypePtr& rhs) {
		TypePtr a = apply(lhs);
		TypePtr b = apply(rhs);
		if (!a || !b) {
			throw std::runtime_error("cannot unify null types");
		}

		if (a->kind == TypeKind::Var) {
			bind_var(a->data.var.id, b);
			return;
		}
		if (b->kind == TypeKind::Var) {
			bind_var(b->data.var.id, a);
			return;
		}

		if (a->kind != b->kind) {
			throw std::runtime_error("type mismatch: " + pretty(a) + " vs " + pretty(b));
		}

		switch (a->kind) {
			case TypeKind::Ptr:
				unify(a->data.ptr.pointee, b->data.ptr.pointee);
				break;
			case TypeKind::Array:
				unify(a->data.array.element, b->data.array.element);
				break;
			case TypeKind::FixedArray:
				if (a->data.fixed_array.size != b->data.fixed_array.size) {
					throw std::runtime_error("fixed array size mismatch");
				}
				unify(a->data.fixed_array.element, b->data.fixed_array.element);
				break;
			case TypeKind::Struct:
				if (a->data.struct_data.name != b->data.struct_data.name) {
					throw std::runtime_error("struct mismatch: " + a->data.struct_data.name + " vs " + b->data.struct_data.name);
				}
				break;
			case TypeKind::App:
				unify(a->data.app.ctor, b->data.app.ctor);
				if (a->data.app.args.size() != b->data.app.args.size()) {
					throw std::runtime_error("type application arity mismatch");
				}
				for (size_t i = 0; i < a->data.app.args.size(); ++i) {
					unify(a->data.app.args[i], b->data.app.args[i]);
				}
				break;
			case TypeKind::Fn:
				if (a->data.fn.params.size() != b->data.fn.params.size()) {
					throw std::runtime_error("function arity mismatch");
				}
				for (size_t i = 0; i < a->data.fn.params.size(); ++i) {
					unify(a->data.fn.params[i], b->data.fn.params[i]);
				}
				unify(a->data.fn.ret, b->data.fn.ret);
				break;
			case TypeKind::Tuple:
				if (a->data.tuple.elements.size() != b->data.tuple.elements.size()) {
					throw std::runtime_error("tuple arity mismatch");
				}
				for (size_t i = 0; i < a->data.tuple.elements.size(); ++i) {
					unify(a->data.tuple.elements[i], b->data.tuple.elements[i]);
				}
				break;
			default:
				break;
		}
	}

	std::string pretty(const TypePtr& t) {
		if (!t) {
			return "<null>";
		}
		TypePtr a = apply(t);
		switch (a->kind) {
			case TypeKind::Void:
				return "Void";
			case TypeKind::I8:
				return "I8";
			case TypeKind::I32:
				return "I32";
			case TypeKind::I64:
				return "I64";
			case TypeKind::F32:
				return "F32";
			case TypeKind::F64:
				return "F64";
			case TypeKind::Bool:
				return "Bool";
			case TypeKind::String:
				return "String";
			case TypeKind::Ptr:
				return "Ptr[" + pretty(a->data.ptr.pointee) + "]";
			case TypeKind::Struct:
				return a->data.struct_data.name;
			case TypeKind::Array:
				return "Array[" + pretty(a->data.array.element) + "]";
			case TypeKind::FixedArray: {
				std::ostringstream os;
				os << "FixedArray[" << pretty(a->data.fixed_array.element) << "," << a->data.fixed_array.size << "]";
				return os.str();
			}
			case TypeKind::Var: {
				std::ostringstream os;
				os << "T" << a->data.var.id;
				return os.str();
			}
			case TypeKind::App: {
				std::ostringstream os;
				os << pretty(a->data.app.ctor) << "[";
				for (size_t i = 0; i < a->data.app.args.size(); ++i) {
					if (i) {
						os << ",";
					}
					os << pretty(a->data.app.args[i]);
				}
				os << "]";
				return os.str();
			}
			case TypeKind::Fn: {
				std::ostringstream os;
				os << "Fn(";
				for (size_t i = 0; i < a->data.fn.params.size(); ++i) {
					if (i) {
						os << ",";
					}
					os << pretty(a->data.fn.params[i]);
				}
				os << ")->" << pretty(a->data.fn.ret);
				return os.str();
			}
			case TypeKind::Tuple: {
				std::ostringstream os;
				os << "(";
				for (size_t i = 0; i < a->data.tuple.elements.size(); ++i) {
					if (i) {
						os << ",";
					}
					os << pretty(a->data.tuple.elements[i]);
				}
				os << ")";
				return os.str();
			}
		}
		return "<unknown>";
	}

private:
	int next_var_id_;
	std::unordered_map<int, TypePtr> subst_;

	bool occurs(int var_id, const TypePtr& t) {
		TypePtr a = apply(t);
		if (!a) {
			return false;
		}
		if (a->kind == TypeKind::Var) {
			return a->data.var.id == var_id;
		}
		switch (a->kind) {
			case TypeKind::Ptr:
				return occurs(var_id, a->data.ptr.pointee);
			case TypeKind::Array:
				return occurs(var_id, a->data.array.element);
			case TypeKind::FixedArray:
				return occurs(var_id, a->data.fixed_array.element);
			case TypeKind::App:
				if (occurs(var_id, a->data.app.ctor)) {
					return true;
				}
				for (const auto& arg : a->data.app.args) {
					if (occurs(var_id, arg)) {
						return true;
					}
				}
				return false;
			case TypeKind::Fn:
				for (const auto& param : a->data.fn.params) {
					if (occurs(var_id, param)) {
						return true;
					}
				}
				return occurs(var_id, a->data.fn.ret);
			case TypeKind::Tuple:
				for (const auto& elem : a->data.tuple.elements) {
					if (occurs(var_id, elem)) {
						return true;
					}
				}
				return false;
			default:
				return false;
		}
	}

	void bind_var(int var_id, const TypePtr& t) {
		TypePtr a = apply(t);
		if (a->kind == TypeKind::Var && a->data.var.id == var_id) {
			return;
		}
		if (occurs(var_id, a)) {
			throw std::runtime_error("occurs check failed");
		}
		subst_[var_id] = a;
	}
};

#endif
