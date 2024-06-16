#ifndef IMP_TYPECHECKER
#define IMP_TYPECHECKER

#include <unordered_map>

#include "imp.hh"
#include "type_visitor.hh"
#include "../environment.hh"

using namespace std;

class ImpTypeChecker : public TypeVisitor {
public:
  ImpTypeChecker();
private:
  Environment<ImpType> env;
  ImpType booltype;
  ImpType inttype;
  int sp, max_sp;
  void sp_incr(int n);
  void sp_decr(int n);

public:
  int typecheck(Program*); // Retorne el sp_max
  void visit(Program*);
  void visit(Body*);
  void visit(VarDecList*);
  void visit(VarDec*);
  void visit(StatementList*);
  void visit(AssignStatement*);
  void visit(PrintStatement*);
  void visit(IfStatement*);
  void visit(WhileStatement*);
  void visit(DoWhileStatement*);
  // void visit(ForStatement*);
  
  ImpType visit(BinaryExp* e);
  ImpType visit(NumberExp* e);
  ImpType visit(IdExp* e);
  ImpType visit(ParenthExp* e);
  ImpType visit(CondExp* e);
  ImpType visit(UnaryExp* e);
  ImpType visit(BoolExp* e);
};


#endif

