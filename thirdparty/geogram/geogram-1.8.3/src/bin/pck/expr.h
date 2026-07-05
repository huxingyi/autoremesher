#ifndef H__PCK_EXPR__H
#define H__PCK_EXPR__H

#include <geogram/basic/counted.h>
#include <geogram/basic/smart_pointer.h>
#include <geogram/basic/memory.h>

namespace GEO {

    /**************************************************************************/
    
    class Expr : public Counted {
    public:
	enum Type { CONSTANT, SCALAR, VECTOR, SIGN };
	virtual ~Expr();
	virtual Type type() const = 0;
	virtual index_t dim() const = 0;
	virtual std::string name() const;
	virtual std::string to_string() const = 0;
	virtual bool atomic() const = 0;
    };

    typedef SmartPointer<Expr> Expr_var;
    
    /**************************************************************************/
    
    class Constant : public Expr {
    public:
	Constant(double value) : value_(value) {
	}
	Type type() const override;
	index_t dim() const override;
	std::string to_string() const override;
	bool atomic() const override;
	double value() const {
	    return value_;
	}
    private:
	double value_;
    };

    /**************************************************************************/
    
    class Variable : public Expr {
    public:
	Variable(Type type) : type_(type) {
	}
	Type type() const override;
	bool atomic() const override;	
    private:
	Type type_;
    };

    /**************************************************************************/
    
    class ScalarVar : public Variable {
    public:
	ScalarVar() : Variable(SCALAR) {
	}
	index_t dim() const override;
	std::string to_string() const override;	
    };

    /**************************************************************************/
    
    class VectorVar : public Variable {
    public:
	VectorVar(
	    index_t dim = index_t(-1)
	) :
	    Variable(VECTOR),
	    dim_(dim) {
	}
	index_t dim() const override;
	std::string to_string() const override;	
	static index_t default_dim() { return default_dim_; }
	static void set_default_dim(index_t dim) { default_dim_ = dim; }
    private:
	index_t dim_;
	static index_t default_dim_;
    };

    /**************************************************************************/

    class VectorComponent : public Expr {
    public:
	VectorComponent(
	    Expr* vector, index_t comp
	) : vector_(vector), component_(comp) {
	}
	Expr* vector() const {
	    return vector_;
	}
	index_t component() const {
	    return component_;
	}
	std::string to_string() const override;
	Type type() const override;
	index_t dim() const override;
	bool atomic() const override;
    private:
	SmartPointer<Expr> vector_;
	index_t component_;
    };
    
    /**************************************************************************/
    
    class SignVar : public Variable {
    public:
	SignVar() : Variable(SIGN) { }
	index_t dim() const override;
	std::string to_string() const override;
    };
    
    /**************************************************************************/

    class Sum : public Expr {
    public:
	Sum(Type type=SCALAR, index_t dim=1) :
	    type_(type),
	    dim_(dim) {
	}
	Type type() const override;
	index_t dim() const override;
	std::string to_string() const override;
	bool atomic() const override;
	
	index_t nb_terms() const {
	    return terms_.size();
	}

	Expr* ith_term(index_t i) const {
	    return terms_[i];
	}

	void add_term(Expr* term);
	
    private:
	Type type_;
	index_t dim_;
	GEO::vector<Expr_var> terms_;
    };
    
    /**************************************************************************/

    class Product : public Expr {
    public:
	Product(Type type=SCALAR, index_t dim=1) :
	    type_(type),
	    dim_(dim) {
	}
	Type type() const override;
	index_t dim() const override;
	std::string to_string() const override;
	bool atomic() const override;
	
	index_t nb_factors() const {
	    return factors_.size();
	}

	Expr* ith_factor(index_t i) const {
	    return factors_[i];
	}

	void mult_factor(Expr* factor);
	
    private:
	Type type_;
	index_t dim_;
	GEO::vector<Expr_var> factors_;
    };

    /**************************************************************************/

    class Pow : public Expr {
    public:
	Pow(Expr* arg, index_t exponent) :
	    type_(arg->type()),
	    dim_(arg->dim()),
	    arg_(arg),
	    exponent_(exponent)	{
	}
	Type type() const override;
	index_t dim() const override;
	Expr* arg() const {
	    return arg_;
	}
	index_t exponent() const {
	    return exponent_;
	}
	std::string to_string() const override;
	bool atomic() const override;
	
    private:
	Type type_;
	index_t dim_;
	Expr_var arg_;
	index_t exponent_;
    };

    /**************************************************************************/
}

#endif
