// =============================================================================
// visitor.cpp — Implementación de TypeCheckerVisitor y GenCodeVisitor
// =============================================================================

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include "ast.h"
#include "visitor.h"

// =============================================================================
// Despacho del patrón Visitor (accept en cada nodo del AST)
// =============================================================================

int BinaryExp::accept(Visitor* v) { return v->visit(this); }
int NumberExp::accept(Visitor* v) { return v->visit(this); }
int IdExp::accept(Visitor* v)     { return v->visit(this); }
int Program::accept(Visitor* v)   { return v->visit(this); }
int PrintStm::accept(Visitor* v)  { return v->visit(this); }
int AssignStm::accept(Visitor* v) { return v->visit(this); }
int IfStm::accept(Visitor* v)     { return v->visit(this); }
int WhileStm::accept(Visitor* v)  { return v->visit(this); }
int Body::accept(Visitor* v)      { return v->visit(this); }
int VarDec::accept(Visitor* v)    { return v->visit(this); }
int FcallExp::accept(Visitor* v)  { return v->visit(this); }
int FunDec::accept(Visitor* v)    { return v->visit(this); }
int ReturnStm::accept(Visitor* v) { return v->visit(this); }
int BreakStm::accept(Visitor *visitor) { return visitor->visit(this);}
int SwitchStm::accept(Visitor *v) { return v->visit(this);}
int DoWhileStm::accept(Visitor *v) { return v->visit(this);}
int CaseStm::accept(Visitor *v) { return v->visit(this);}
int UnaryExp::accept(Visitor *visitor) { return visitor->visit(this);}




// =============================================================================
// TypeCheckerVisitor — Análisis semántico
// =============================================================================
// Responsabilidades:
//   1. Registrar las funciones y su aridad antes de analizar los cuerpos.
//   2. Verificar que cada variable usada haya sido declarada en el scope.
//   3. Verificar que cada función llamada exista y reciba el número correcto
//      de argumentos.
//   4. Contar las variables locales de cada función para que GenCodeVisitor
//      pueda reservar el espacio correcto en el stack frame.
// =============================================================================

// -----------------------------------------------------------------------------
// TypeChecker — punto de entrada
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::TypeChecker(Program* program) {
    // Primera pasada: registrar todas las funciones y su aridad.
    // Esto permite llamadas hacia adelante (funciones que se usan antes de
    // ser definidas en el orden textual del fuente).
    for (auto fd : program->fdlist) {
        funAridad[fd->nombre] = static_cast<int>(fd->Pnombres.size());
    }

    // Segunda pasada: analizar cada función
    for (auto fd : program->fdlist) {
        fd->accept(this);
    }

    return 0;
}

// -----------------------------------------------------------------------------
// visit(FunDec) — analiza una declaración de función
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::visit(FunDec* fd) {
    funcionActual = fd->nombre;
    locales       = 0;
    int parametros = static_cast<int>(fd->Pnombres.size());

    // Abrir scope de la función
    entorno.add_level();

    // Registrar los parámetros como variables del scope actual
    for (auto& nombre : fd->Pnombres)
        entorno.add_var(nombre, 0);

    // Analizar el cuerpo
    fd->cuerpo->accept(this);

    // Cerrar scope
    entorno.remove_level();

    // Guardar el tamaño total del frame (parámetros + locales)
    funcontador[fd->nombre] = parametros + locales;
    return 0;
}

// -----------------------------------------------------------------------------
// visit(Body) — analiza declaraciones y sentencias del cuerpo
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::visit(Body* body) {
    entorno.add_level();

    for (auto dec : body->declarations)
        dec->accept(this);
    for (auto stm : body->StmList)
        stm->accept(this);

    entorno.remove_level();
    return 0;
}

// -----------------------------------------------------------------------------
// visit(VarDec) — registra las variables declaradas y cuenta los locales
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::visit(VarDec* vd) {
    for (auto& nombre : vd->vars) {
        if (entorno.check(nombre)) {
            std::cerr << "[TypeChecker] Advertencia: la variable '"
                      << nombre << "' ya fue declarada en este scope"
                      << " (en función '" << funcionActual << "').\n";
        }
        entorno.add_var(nombre, 0);
        locales++;
    }
    return 0;
}

// -----------------------------------------------------------------------------
// visit(IdExp) — verifica que la variable esté declarada
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::visit(IdExp* exp) {
    if (!entorno.check(exp->value)) {
        throw std::runtime_error(
            "[TypeChecker] Variable no declarada: '" + exp->value +
            "' usada en la función '" + funcionActual + "'"
        );
    }
    return 0;
}

// -----------------------------------------------------------------------------
// visit(AssignStm) — verifica variable y evalúa expresión
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::visit(AssignStm* stm) {
    if (!entorno.check(stm->id)) {
        throw std::runtime_error(
            "[TypeChecker] Variable no declarada: '" + stm->id +
            "' asignada en la función '" + funcionActual + "'"
        );
    }
    stm->e->accept(this);
    return 0;
}

// -----------------------------------------------------------------------------
// visit(FcallExp) — verifica existencia de la función y aridad
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::visit(FcallExp* fcall) {
    // Verificar que la función exista
    if (funAridad.find(fcall->nombre) == funAridad.end()) {
        throw std::runtime_error(
            "[TypeChecker] Función no definida: '" + fcall->nombre +
            "' llamada en '" + funcionActual + "'"
        );
    }

    // Verificar número de argumentos
    int esperados = funAridad[fcall->nombre];
    int recibidos = static_cast<int>(fcall->argumentos.size());
    if (recibidos != esperados) {
        throw std::runtime_error(
            "[TypeChecker] La función '" + fcall->nombre +
            "' espera " + std::to_string(esperados) +
            " argumento(s), pero se pasaron " + std::to_string(recibidos)
        );
    }

    // Verificar cada argumento
    for (auto arg : fcall->argumentos)
        arg->accept(this);

    return 0;
}

// -----------------------------------------------------------------------------
// visit(IfStm) — cuenta máximo de locales entre ramas
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::visit(IfStm* stm) {
    stm->condition->accept(this);

    int base = locales;

    // Rama then
    locales = 0;
    stm->then->accept(this);
    int maxLocales = locales;

    // Rama else (opcional)
    if (stm->els) {
        locales = 0;
        stm->els->accept(this);
        maxLocales = std::max(maxLocales, locales);
    }

    // El frame necesita el máximo de ambas ramas
    locales = base + maxLocales;
    return 0;
}

// -----------------------------------------------------------------------------
// visit(WhileStm)
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::visit(WhileStm* stm) {
    stm->condition->accept(this);
    stm->b->accept(this);
    return 0;
}

// -----------------------------------------------------------------------------
// visit(PrintStm)
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::visit(PrintStm* stm) {
    stm->e->accept(this);
    return 0;
}

// -----------------------------------------------------------------------------
// visit(ReturnStm)
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::visit(ReturnStm* r) {
    r->e->accept(this);
    return 0;
}

// -----------------------------------------------------------------------------
// Nodos que no requieren verificación semántica adicional
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::visit(BinaryExp* exp) {
    exp->left->accept(this);
    exp->right->accept(this);
    return 0;
}

int TypeCheckerVisitor::visit(NumberExp* exp) { return 0; }
int TypeCheckerVisitor::visit(Program* p)     { return 0; }

int TypeCheckerVisitor::visit(UnaryExp* e) {
    if (e && e->e) e->e->accept(this);
    return 0;
}

int TypeCheckerVisitor::visit(BreakStm* s) {
    // Nothing to check here; semantics of 'break' handled in codegen
    return 0;
}

int TypeCheckerVisitor::visit(DoWhileStm* s) {
    s->b->accept(this);
    if (s->condition) s->condition->accept(this);
    return 0;
}

int TypeCheckerVisitor::visit(SwitchStm* s) {
    if (s->e) s->e->accept(this);
    for (auto c : s->cases) {
        if (c->value) c->value->accept(this);
        if (c->body) c->body->accept(this);
    }
    return 0;
}

int TypeCheckerVisitor::visit(CaseStm* c) {
    if (c->value) c->value->accept(this);
    if (c->body) c->body->accept(this);
    return 0;
}

// =============================================================================
// GenCodeVisitor — Generación de código ensamblador x86-64 (AT&T syntax)
// =============================================================================
// Convenciones usadas:
//   · Registros de argumentos: %rdi, %rsi, %rdx, %rcx, %r8, %r9
//   · Resultado de expresiones en %rax
//   · Variables locales: offsets negativos desde %rbp
//   · Variables globales: símbolos en .data con acceso RIP-relativo
//   · printf para print: formato en print_fmt ("%ld \n")
// =============================================================================

// -----------------------------------------------------------------------------
// generar — punto de entrada de la generación
// -----------------------------------------------------------------------------

int GenCodeVisitor::generar(Program* program) {
    tipos.TypeChecker(program);
    funcontador = tipos.funcontador;
    program->accept(this);
    return 0;
}

// -----------------------------------------------------------------------------
// visit(Program)
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(Program* program) {
    // Sección de datos
    out << ".data\n";
    out << "print_fmt: .string \"%ld \\n\"\n";

    // Recolectar nombres de variables globales
    for (auto dec : program->vdlist)
        dec->accept(this);

    // Emitir las etiquetas .quad para las globales
    for (auto& [var, _] : memoriaGlobal)
        out << var << ": .quad 0\n";

    // Sección de texto (código)
    out << "\n.text\n";

    for (auto fd : program->fdlist)
        fd->accept(this);

    // Marca el stack como no ejecutable (requerido por el linker moderno)
    out << "\n.section .note.GNU-stack,\"\",@progbits\n";
    return 0;
}

// -----------------------------------------------------------------------------
// visit(VarDec) — registra variables en memoria (global o local)
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(VarDec* stm) {
    for (auto& var : stm->vars) {
        if (!entornoFuncion) {
            memoriaGlobal[var] = true;
        } else {
            memoria[var] = offset;
            offset -= 8;
        }
    }
    return 0;
}

// -----------------------------------------------------------------------------
// visit(NumberExp) — carga un inmediato en %rax
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(NumberExp* exp) {
    out << " movq $" << exp->value << ", %rax\n";
    return 0;
}

// -----------------------------------------------------------------------------
// visit(IdExp) — carga el valor de una variable en %rax
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(IdExp* exp) {
    if (memoriaGlobal.count(exp->value))
        out << " movq " << exp->value << "(%rip), %rax\n";
    else
        out << " movq " << memoria[exp->value] << "(%rbp), %rax\n";
    return 0;
}

// -----------------------------------------------------------------------------
// visit(BinaryExp) — evalúa left y right, aplica operador
// Convención: left en %rax, right en %rcx
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(BinaryExp* exp) {
    exp->left->accept(this);
    out << " pushq %rax\n";
    exp->right->accept(this);
    out << " movq %rax, %rcx\n";
    out << " popq %rax\n";

    switch (exp->op) {
        case PLUS_OP:
            out << " addq %rcx, %rax\n";
            break;
        case MINUS_OP:
            out << " subq %rcx, %rax\n";
            break;
        case MUL_OP:
            out << " imulq %rcx, %rax\n";
            break;
        case DIV_OP:
            // División entera con signo: idivq usa %rdx:%rax / %rcx
            out << " cqto\n";           // sign-extend %rax → %rdx:%rax
            out << " idivq %rcx\n";     // cociente en %rax
            break;
        case POW_OP:
            // Simple pow: repeated multiplication (ineficiente)
            // Implement as result = 1; for(i=0;i<right;i++) result *= left;
            out << " movq %rax, %r11\n"; // base in r11
            out << " movq %rcx, %r12\n"; // exp in r12
            out << " movq $1, %rax\n";   // result
            out << " testq %r12, %r12\n";
            out << " jz .pow_done_" << labelcont << "\n";
            out << ".pow_loop_" << labelcont << ":\n";
            out << " imulq %r11, %rax\n";
            out << " decq %r12\n";
            out << " jnz .pow_loop_" << labelcont << "\n";
            out << ".pow_done_" << labelcont << ":\n";
            labelcont++;
            break;
        case LE_OP:
            out << " cmpq %rcx, %rax\n";
            out << " movq $0, %rax\n";
            out << " setl %al\n";
            out << " movzbq %al, %rax\n";
            break;
        case GE_OP:
            out << " cmpq %rcx, %rax\n";
            out << " movq $0, %rax\n";
            out << " setg %al\n";
            out << " movzbq %al, %rax\n";
            break;
        case LEQ_OP:
            out << " cmpq %rcx, %rax\n";
            out << " movq $0, %rax\n";
            out << " setle %al\n";
            out << " movzbq %al, %rax\n";
            break;
        case GEQ_OP:
            out << " cmpq %rcx, %rax\n";
            out << " movq $0, %rax\n";
            out << " setge %al\n";
            out << " movzbq %al, %rax\n";
            break;
        case EQ_OP:
            out << " cmpq %rcx, %rax\n";
            out << " movq $0, %rax\n";
            out << " sete %al\n";
            out << " movzbq %al, %rax\n";
            break;
        case NEQ_OP:
            out << " cmpq %rcx, %rax\n";
            out << " movq $0, %rax\n";
            out << " setne %al\n";
            out << " movzbq %al, %rax\n";
            break;
        case AND_OP:
            // left in %rax, right in %rcx
            out << " andq %rcx, %rax\n";
            break;
        case OR_OP:
            out << " orq %rcx, %rax\n";
            break;
    }
    return 0;
}

// -----------------------------------------------------------------------------
// visit(UnaryExp) — negación lógica
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(UnaryExp* e) {
    e->e->accept(this);

    int id = labelcont++;
    switch (e->op) {
        case NOT_OP:
            out << " cmpq $0, %rax\n";
            out << " je not_true_" << id << "\n";
            out << " movq $0, %rax\n";
            out << " jmp not_end_" << id << "\n";
            out << "not_true_" << id << ":\n";
            out << " movq $1, %rax\n";
            out << "not_end_" << id << ":\n";
            break;
        default:
            out << "xDDDDDDDDDDDDDDd" << "\n";
    }
    return 0;
}

// -----------------------------------------------------------------------------
// visit(BreakStm)
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(BreakStm* s) {
    if (breaktargets.empty()) {
        throw std::runtime_error("[GenCode] 'break' fuera de un contexto de bucle/switch");
    }
    std::string lbl = breaktargets.back();
    out << " jmp " << lbl << "\n";
    return 0;
}

// -----------------------------------------------------------------------------
// visit(DoWhileStm)
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(DoWhileStm* s) {
    int lbl = labelcont++;
    std::string endDoWhileLabel = "endwhile_" + std::to_string(lbl);
    // marcar destino de break
    breaktargets.push_back(endDoWhileLabel);

    out << "dowhile_" << lbl << ":\n";
    s->b->accept(this);
    s->condition->accept(this);
    out << " cmpq $0, %rax\n";
    out << " jne dowhile_" << lbl << "\n";
    out << endDoWhileLabel << ":\n";

    breaktargets.pop_back();
    return 0;
}

// -----------------------------------------------------------------------------
// visit(SwitchStm) and visit(CaseStm)
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(SwitchStm* s) {
    std::cerr << "[GenCode] visit SwitchStm\n";
    int swlbl = labelcont++;
    std::string endSwitchLabel = "endswitch_" + std::to_string(swlbl);

    // Evaluar expresión switch y guardarla en %r10
    s->e->accept(this);
    out << " movq %rax, %r10\n";

    // default node
    CaseStm* defaultNode = nullptr;

    for (auto c : s->cases) {
        if (c->value != nullptr) {
            NumberExp* numExp = dynamic_cast<NumberExp*>(c->value);
            int val = numExp ? numExp->value : 0;

            c->value->accept(this);
            out << " cmpq %rax, %r10\n";
            out << " je case_" << swlbl << "_" << val << "\n";
        } else {
            defaultNode = c;
        }
    }

    if (defaultNode != nullptr) {
        out << " jmp default_" << swlbl << "\n";
    } else {
        out << " jmp " << endSwitchLabel << "\n";
    }

    breaktargets.push_back(endSwitchLabel);
    for (auto c:s->cases) {
        if (c->value != nullptr) {
            NumberExp* numExp = dynamic_cast<NumberExp*>(c->value);
            int val = numExp ? numExp->value : 0;

            out << "case_" << swlbl << "_" << val << ":\n";
            c->body->accept(this);
            out << " jmp " << endSwitchLabel << "\n";
        }
    }
    out << "default_" << swlbl << ":\n";
    if (defaultNode != nullptr) {
        defaultNode->body->accept(this);
    }

    // Etiqueta de salida oficial del switch
    out << endSwitchLabel << ":\n";

    // Quitamos el objetivo del break de la pila
    breaktargets.pop_back();
    return 0;
}

int GenCodeVisitor::visit(CaseStm* c) {
    return 0;
}

// -----------------------------------------------------------------------------
// visit(AssignStm) — evalúa expresión y almacena resultado
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(AssignStm* stm) {
    stm->e->accept(this);
    if (memoriaGlobal.count(stm->id))
        out << " movq %rax, " << stm->id << "(%rip)\n";
    else
        out << " movq %rax, " << memoria[stm->id] << "(%rbp)\n";
    return 0;
}

// -----------------------------------------------------------------------------
// visit(PrintStm) — imprime un entero con printf
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(PrintStm* stm) {
    stm->e->accept(this);
    out << " movq %rax, %rsi\n";
    out << " leaq print_fmt(%rip), %rdi\n";
    out << " movq $0, %rax\n";
    out << " call printf@PLT\n";
    return 0;
}

// -----------------------------------------------------------------------------
// visit(Body) — procesa declaraciones y sentencias
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(Body* b) {
    for (auto dec : b->declarations)
        dec->accept(this);
    for (auto stm : b->StmList)
        stm->accept(this);
    return 0;
}

// -----------------------------------------------------------------------------
// visit(IfStm) — emite bloque if-then-else con etiquetas únicas
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(IfStm* stm) {
    int lbl = labelcont++;
    stm->condition->accept(this);
    out << " cmpq $0, %rax\n";
    out << " je else_" << lbl << "\n";
    stm->then->accept(this);
    out << " jmp endif_" << lbl << "\n";
    out << "else_" << lbl << ":\n";
    if (stm->els) stm->els->accept(this);
    out << "endif_" << lbl << ":\n";
    return 0;
}

// -----------------------------------------------------------------------------
// visit(WhileStm) — emite bucle while con etiquetas únicas
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(WhileStm* stm) {
    int lbl = labelcont++;
    // marcar destino de break
    std::string endWhileLabel = "endwhile_" + std::to_string(lbl);
    breaktargets.push_back(endWhileLabel);

    out << "while_" << lbl << ":\n";
    stm->condition->accept(this);
    out << " cmpq $0, %rax\n";
    out << " je " << endWhileLabel << "\n";
    stm->b->accept(this);
    out << " jmp while_" << lbl << "\n";
    out << endWhileLabel << ":\n";

    breaktargets.pop_back();
    return 0;
}

// -----------------------------------------------------------------------------
// visit(ReturnStm) — salta al epílogo de la función
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(ReturnStm* stm) {
    stm->e->accept(this);
    out << " jmp .end_" << nombreFuncion << "\n";
    return 0;
}

// -----------------------------------------------------------------------------
// visit(FunDec) — emite prólogo, cuerpo y epílogo de una función
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(FunDec* f) {
    entornoFuncion = true;
    memoria.clear();
    offset        = -8;
    nombreFuncion = f->nombre;

    const std::vector<std::string> argRegs = {
        "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"
    };

    // ---- Prólogo ----
    out << "\n.globl " << f->nombre << "\n";
    out << f->nombre << ":\n";
    out << " pushq %rbp\n";
    out << " movq %rsp, %rbp\n";
    out << " subq $" << funcontador[f->nombre] * 8 << ", %rsp\n";

    // Guardar parámetros en el frame local
    int nParams = static_cast<int>(f->Pnombres.size());
    for (int i = 0; i < nParams; i++) {
        memoria[f->Pnombres[i]] = offset;
        out << " movq " << argRegs[i] << ", " << offset << "(%rbp)\n";
        offset -= 8;
    }

    // Registrar variables locales declaradas (ajusta 'offset' y 'memoria')
    for (auto dec : f->cuerpo->declarations)
        dec->accept(this);

    // ---- Cuerpo ----
    for (auto stm : f->cuerpo->StmList)
        stm->accept(this);

    // ---- Epílogo ----
    out << ".end_" << f->nombre << ":\n";
    out << " leave\n";
    out << " ret\n";

    entornoFuncion = false;
    return 0;
}

// -----------------------------------------------------------------------------
// visit(FcallExp) — emite una llamada a función
// Argumentos en registros según la ABI System V x86-64
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(FcallExp* exp) {
    const std::vector<std::string> argRegs = {
        "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"
    };

    int nArgs = static_cast<int>(exp->argumentos.size());
    for (int i = 0; i < nArgs; i++) {
        exp->argumentos[i]->accept(this);
        out << " movq %rax, " << argRegs[i] << "\n";
    }
    out << " call " << exp->nombre << "\n";
    return 0;
}
