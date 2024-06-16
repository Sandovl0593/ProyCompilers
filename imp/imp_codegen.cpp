#include "imp_codegen.hh"

void ImpCodeGen::codegen(string label, string instr) {
  if (label !=  nolabel)
    code << label << ": ";
  code << instr << endl;
}

void ImpCodeGen::codegen(string label, string instr, int arg) {
  if (label !=  nolabel)
    code << label << ": ";
  code << instr << " " << arg << endl;
}

void ImpCodeGen::codegen(string label, string instr, string jmplabel) {
  if (label !=  nolabel)
    code << label << ": ";
  code << instr << " " << jmplabel << endl;
}

string ImpCodeGen::next_label() {
  string l = "L";
  string n = to_string(current_label++);
  l.append(n);
  return l;
}

void ImpCodeGen::codegen(Program* p, string outfname) {
  checker = new ImpTypeChecker();
  nolabel = "";
  current_label = 0;
  siguiente_direccion = 1;
  mem_locals = checker->typecheck(p);
  codegen(nolabel,"alloc",this->mem_locals);
  p->accept(this);
  ofstream outfile;
  outfile.open(outfname);
  outfile << code.str();
  outfile.close();
  cout << "Memoria variables locales: " << mem_locals << endl;
  
  return;
}

int ImpCodeGen::visit(Program* p) {
  p->body->accept(this);
  return 0;
}

int ImpCodeGen::visit(Body * b) {
  direcciones.add_level();
  b->var_decs->accept(this);
  b->slist->accept(this);
  direcciones.remove_level();
  return 0;
}

int ImpCodeGen::visit(VarDecList* s) {
  list<VarDec*>::iterator it;
  for (it = s->vdlist.begin(); it != s->vdlist.end(); ++it) {
    (*it)->accept(this);
  }  
  return 0;
}

int ImpCodeGen::visit(VarDec* vd) {
  list<string>::iterator it;
  int prev_direcciones = 0;
  for (it = vd->vars.begin(); it != vd->vars.end(); ++it){
    direcciones.add_var(*it, siguiente_direccion++); ++prev_direcciones;
  }
  return 0;
}

int ImpCodeGen::visit(StatementList* s) {
  list<Stm*>::iterator it;
  for (it = s->slist.begin(); it != s->slist.end(); ++it) {
    (*it)->accept(this);
  }
  return 0;
}

int ImpCodeGen::visit(AssignStatement* s) {
  s->rhs->accept(this);
  codegen(nolabel,"store",direcciones.lookup(s->id));
  return 0;
}

int ImpCodeGen::visit(PrintStatement* s) {
  s->e->accept(this);
  code << "print" << endl;
  return 0;
}

int ImpCodeGen::visit(IfStatement* s) {
  string l1 = next_label();
  string l2 = next_label();

  s->cond->accept(this);
  codegen(nolabel,"jmpz",l1);
  s->tbody->accept(this);
  codegen(nolabel,"goto",l2);
  codegen(l1,"skip");
  if (s->fbody!=NULL) {
    s->fbody->accept(this);
  }
  codegen(l2,"skip");

  return 0;
}

int ImpCodeGen::visit(WhileStatement* s) {
  string l1 = next_label();
  string l2 = next_label();

  in_Loop = true;
  loop_repeat_label = l1;
  loop_finish_label = l2;
  
  codegen(l1,"skip");
  s->cond->accept(this);
  codegen(nolabel,"jmpz",l2);
  s->body->accept(this);
  codegen(nolabel,"goto",l1);
  codegen(l2,"skip");
  
  in_Loop = false;

  return 0;
}

int ImpCodeGen::visit(DoWhileStatement* s) {
  string l1 = next_label();
  string l2 = next_label();

  codegen(l1,"skip");
  s->body->accept(this);
  s->cond->accept(this);
  codegen(nolabel,"jmpz",l2);
  codegen(nolabel,"goto",l1);
  codegen(l2,"skip");

  return 0;
}

int ImpCodeGen::visit(BinaryExp* e) {
  e->left->accept(this);
  e->right->accept(this);
  string op = "";
  switch(e->op) {
    case PLUS: op =  "add"; break;
    case MINUS: op = "sub"; break;
    case MULT:  op = "mul"; break;
    case DIV:  op = "div"; break;
    case LT:  op = "lt"; break;
    case LTEQ: op = "le"; break;
    case EQ:  op = "eq"; break;
    case GT:  op = "gt"; break;
    case GTEQ: op = "ge"; break;
    case AND: op = "and"; break;
    case OR: op = "or"; break;
    default: cout << "binop " << Exp::binopToString(e->op) << " not implemented" << endl;
  }
  codegen(nolabel, op);
  return 0;
}

int ImpCodeGen::visit(UnaryExp* e) {
  e->e->accept(this);
  if (e->op == NEG) {
    codegen(nolabel,"push",0);
    codegen(nolabel,"swap");
    codegen(nolabel,"sub");
  } else {
    string l1 = next_label();
    string l2 = next_label();
    codegen(nolabel,"jmpz",l1);
    codegen(nolabel,"push", 0);
    codegen(nolabel,"goto", l2);
    codegen(l1,"skip", 0);
    codegen(nolabel,"push", 1);
    codegen(l2,"skip");
  }
  return 0;
}

int ImpCodeGen::visit(NumberExp* e) {
  codegen(nolabel,"push",e->value);
  return 0;
}

int ImpCodeGen::visit(IdExp* e) {
  codegen(nolabel,"load",direcciones.lookup(e->id));
  return 0;
}

int ImpCodeGen::visit(ParenthExp* ep) {
  ep->e->accept(this);
  return 0;
}

int ImpCodeGen::visit(CondExp* e) {
  string l1 = next_label();
  string l2 = next_label();
 
  e->cond->accept(this);
  codegen(nolabel, "jmpz", l1);
  e->etrue->accept(this);
  codegen(nolabel, "goto", l2);
  codegen(l1,"skip");
  e->efalse->accept(this);
  codegen(l2, "skip");
  return 0;
}

int ImpCodeGen::visit(BoolExp* e) {
  codegen(nolabel,"push",e->value?1:0);
  return 0;
}