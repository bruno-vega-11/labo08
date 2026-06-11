#ifndef AST_H
#define AST_H

// =============================================================================
// ast.h — Definición del Árbol de Sintaxis Abstracta (AST)
// =============================================================================
// Jerarquía de nodos:
//
//   Exp (abstracta)
//     ├── NumberExp   — literal entero
//     ├── IdExp       — variable
//     ├── BinaryExp   — operación binaria (+, -, *, /, **, <)
//     └── FcallExp    — llamada a función
//
//   Stm (abstracta)
//     ├── AssignStm   — asignación: ID = Exp
//     ├── PrintStm    — print(Exp)
//     ├── ReturnStm   — return(Exp)
//     ├── IfStm       — if CE then Body [else Body] endif
//     └── WhileStm    — while CE do Body endwhile
//
//   VarDec            — var tipo var1, var2, ...
//   Body              — VarDec* Stm*
//   FunDec            — fun tipo nombre(params) Body endfun
//   Program           — VarDec* FunDec*
// =============================================================================

#include <list>
#include <ostream>
#include <string>
#include <vector>

class Visitor;
class VarDec;
class Body;

// =============================================================================
// Operadores binarios
// =============================================================================

enum BinaryOp {
    PLUS_OP,    // +
    MINUS_OP,   // -
    MUL_OP,     // *
    DIV_OP,     // /
    POW_OP,     // **
    LE_OP,      // <
    GE_OP,      // >
    LEQ_OP,     // <=
    GEQ_OP,     // >=
    EQ_OP,      // ==
    NEQ_OP,     // !=
    AND_OP,     // and
    OR_OP,      // or
};

enum UnaryOp {
    NOT_OP      // !
};

// =============================================================================
// Expresiones
// =============================================================================

class Exp {
public:
    virtual int  accept(Visitor* visitor) = 0;
    virtual ~Exp() = 0;
    static std::string binopToChar(BinaryOp op);
};

// ---- Expresión binaria ----
class BinaryExp : public Exp {
public:
    Exp*     left;
    Exp*     right;
    BinaryOp op;
    BinaryExp(Exp* l, Exp* r, BinaryOp op);
    int  accept(Visitor* visitor) override;
    ~BinaryExp();
};

class UnaryExp: public Exp {
public:
    Exp* e;
    UnaryOp op;
    UnaryExp(Exp* _e,UnaryOp _op):e(_e),op(_op){};
    int accept(Visitor *visitor) override;
    ~UnaryExp(){};
};


// ---- Literal numérico entero ----
class NumberExp : public Exp {
public:
    int value;
    NumberExp(int v);
    int  accept(Visitor* visitor) override;
    ~NumberExp();
};

// ---- Identificador (variable) ----
class IdExp : public Exp {
public:
    std::string value;
    IdExp(const std::string& v);
    int  accept(Visitor* visitor) override;
    ~IdExp();
};

// ---- Llamada a función ----
class FcallExp : public Exp {
public:
    std::string          nombre;
    std::vector<Exp*>    argumentos;
    FcallExp() = default;
    int  accept(Visitor* visitor) override;
    ~FcallExp() = default;
};

// =============================================================================
// Sentencias
// =============================================================================

class Stm {
public:
    virtual int  accept(Visitor* visitor) = 0;
    virtual ~Stm() = 0;
};

// ---- Asignación: ID = Exp ----
class AssignStm : public Stm {
public:
    std::string id;
    Exp*        e;
    AssignStm(const std::string& id, Exp* e);
    int  accept(Visitor* visitor) override;
    ~AssignStm();
};

// ---- Impresión: print(Exp) ----
class PrintStm : public Stm {
public:
    Exp* e;
    PrintStm(Exp* e);
    int  accept(Visitor* visitor) override;
    ~PrintStm();
};

// ---- Retorno: return(Exp) ----
class ReturnStm : public Stm {
public:
    Exp* e;
    ReturnStm() {}
    int  accept(Visitor* visitor) override;
    ~ReturnStm() {}
};

// ---- Condicional: if CE then Body [else Body] endif ----
class IfStm : public Stm {
public:
    Exp*  condition;
    Body* then;
    Body* els;      // Puede ser nullptr si no hay rama else
    IfStm(Exp* condition, Body* then, Body* els);
    int  accept(Visitor* visitor) override;
    ~IfStm() {}
};

// ---- Bucle: while CE do Body endwhile ----
class WhileStm : public Stm {
public:
    Exp*  condition;
    Body* b;
    WhileStm(Exp* condition, Body* b);
    int  accept(Visitor* visitor) override;
    ~WhileStm() {}
};

class BreakStm : public Stm {
public:
    BreakStm(){};
    int accept(Visitor *visitor) override;
    ~BreakStm(){}
};

class DoWhileStm: public Stm {
public:
    Body* b;
    Exp* condition;
    DoWhileStm(Body* b, Exp* c) : b(b), condition(c) {}
    int accept(Visitor* v) override;
    ~DoWhileStm() {}
};

class CaseStm {
public:
    Exp* value;       // nullptr => default
    Body* body;
    int accept(Visitor* v);
    CaseStm(Exp* v, Body* b) : value(v), body(b) {}
};


class SwitchStm : public Stm {
public:
    Exp* e;
    std::vector<CaseStm*> cases;
    SwitchStm(Exp* e) : e(e) {}
    int accept(Visitor* v) override;
    ~SwitchStm() {}
};
// =============================================================================
// Declaraciones y estructura del programa
// =============================================================================

// ---- Declaración de variables: var tipo var1, var2, ... ----
class VarDec {
public:
    std::string            type;
    std::list<std::string> vars;
    VarDec();
    int  accept(Visitor* visitor);
    ~VarDec();
};

// ---- Cuerpo: VarDec* Stm* ----
class Body {
public:
    std::list<VarDec*> declarations;
    std::list<Stm*>    StmList;
    Body();
    int  accept(Visitor* visitor);
    ~Body();
};

// ---- Declaración de función: fun tipo nombre(params) Body endfun ----
class FunDec {
public:
    std::string              nombre;
    std::string              tipo;       // Tipo de retorno
    Body*                    cuerpo;
    std::vector<std::string> Ptipos;     // Tipos de los parámetros
    std::vector<std::string> Pnombres;   // Nombres de los parámetros
    FunDec() = default;
    int  accept(Visitor* visitor);
    ~FunDec() = default;
};

// ---- Programa: VarDec* FunDec* ----
class Program {
public:
    std::list<VarDec*> vdlist;
    std::list<FunDec*> fdlist;
    Program() = default;
    int  accept(Visitor* visitor);
    ~Program() = default;
};

#endif // AST_H
