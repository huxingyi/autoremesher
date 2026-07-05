#ifndef PRETTY_PRINT_VISITOR_H
#define PRETTY_PRINT_VISITOR_H

#include <FPG/Visitor.h>
#include <map>
#include <string>
#include <ostream>

struct Prettyprint_visitor : public Visitor {
    typedef	std::map< AST::Node*, std::string > Annotation_map;

    std::ostream    &out;
    Annotation_map   annotation_map;
    unsigned int     indent_count;

    Prettyprint_visitor(std::ostream& out);
    Prettyprint_visitor(std::ostream& out, const Annotation_map& annotation_map);

    virtual void visit( AST::LiteralExpression*     );
    virtual void visit( AST::IdentifierExpression*  );
    virtual void visit( AST::UnaryExpression*       );
    virtual void visit( AST::BinaryExpression*      );
    virtual void visit( AST::ConditionalExpression* );
    virtual void visit( AST::AssignmentExpression*  );
    virtual void visit( AST::FunctionCall*          );
    virtual void visit( AST::UnaryFunction*         );
    virtual void visit( AST::EmptyStatement*        );
    virtual void visit( AST::ExpressionStatement*   );
    virtual void visit( AST::ConditionalStatement*  );
    virtual void visit( AST::Return*                );
    virtual void visit( AST::StatementList*         );
    virtual void visit( AST::VariableDeclaration*   );
    virtual void visit( AST::CompoundStatement*     );
    virtual void visit( AST::FunctionDefinition*    );
    virtual void visit( AST::PlainText*             );
    virtual void visit( AST::PlainTextExpression*   );
    virtual void visit( AST::TranslationUnit*       );


    void dumpIndent();
    void indent();
    void unindent();
    void printIndented( AST::Node *node );

    struct Output_rep {
        AST::Node           *node;
        Prettyprint_visitor *visitor;

        Output_rep( AST::Node *node, Prettyprint_visitor *visitor )
          : node(node), visitor(visitor)
        {}

        void operator()() {
            node->accept(visitor) ;
        }
    };

    Output_rep  oformat( AST::Node *node );

};


#endif
