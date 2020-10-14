#include "ast.hpp"
#include "symbol_table.hpp"
#include "errors.hpp"
#include "types.hpp"
#include "name_analysis.hpp"
#include "type_analysis.hpp"
#include <iostream>

namespace holeyc{

TypeAnalysis * TypeAnalysis::build(NameAnalysis * nameAnalysis){
	//To emphasize that type analysis depends on name analysis
	// being complete, a name analysis must be supplied for 
	// type analysis to be performed.
	TypeAnalysis * typeAnalysis = new TypeAnalysis();
	auto ast = nameAnalysis->ast;	
	typeAnalysis->ast = ast;

	ast->typeAnalysis(typeAnalysis);
	if (typeAnalysis->hasError){
		return nullptr;
	}

	return typeAnalysis;

}

void ProgramNode::typeAnalysis(TypeAnalysis * ta){

	//pass the TypeAnalysis down throughout
	// the entire tree, getting the types for
	// each element in turn and adding them
	// to the ta object's hashMap
	for (auto global : *myGlobals){
		global->typeAnalysis(ta);
	}

	//The type of the program node will never
	// be needed. We can just set it to VOID
	//(Alternatively, we could make our type 
	// be error if the DeclListNode is an error)
	ta->nodeType(this, BasicType::produce(VOID));
}

void FnDeclNode::typeAnalysis(TypeAnalysis * ta){

	//HINT: you might want to change the signature for
	// typeAnalysis on FnBodyNode to take a second
	// argument which is the type of the current 
	// function. This will help you to know at a 
	// return statement whether the return type matches
	// the current function

	//Note: this function may need extra code

	for (auto stmt : *myBody){
		stmt->typeAnalysis(ta);
	}
}

void StmtNode::typeAnalysis(TypeAnalysis * ta){
	TODO("Implement me in the subclass");
}

void AssignStmtNode::typeAnalysis(TypeAnalysis * ta){
	myExp->typeAnalysis(ta);

	//It can be a bit of a pain to write 
	// "const DataType *" everywhere, so here
	// the use of auto is used instead to tell the
	// compiler to figure out what the subType variable
	// should be
	auto subType = ta->nodeType(myExp);

	// As error returns null if subType is NOT an error type
	// otherwise, it returns the subType itself
	// nullptr casts to boolean false
	if (subType->asError()){
		ta->nodeType(this, subType);
	} else {
		// if error occurs then set AssignStmtNode nodeType to VOID?
		ta->nodeType(this, BasicType::produce(VOID));
	}
}

void ExpNode::typeAnalysis(TypeAnalysis * ta){
	//TODO("Override me in the subclass");
}

void AssignExpNode::typeAnalysis(TypeAnalysis * ta){
	//TODO: Note that this function is incomplete. 
	// and needs additional code

	//Do typeAnalysis on the subexpressions
	myDst->typeAnalysis(ta);
	mySrc->typeAnalysis(ta);

	const DataType * tgtType = ta->nodeType(myDst);
	const DataType * srcType = ta->nodeType(mySrc);

	//While incomplete, this gives you one case for 
	// assignment: if the types are exactly the same
	// it is usually ok to do the assignment. One
	// exception is that if both types are function
	// names, it should fail type analysis
	if (tgtType == srcType){
		if (tgtType->validVarType() == false) { //call false for void and fn types
			//call error
		}
		else {
			ta->nodeType(this, tgtType);
		}
		return;
	}
	// TODO will need to adapt this later but for now set AssignExpNode node to to error and don't report if tgtType or srcType is error
	if(tgtType->asError()!=nullptr || srcType->asError()!=nullptr ){
	    ta->nodeType(this, ErrorType::produce());
	    return;
	}
	
	//Some functions are already defined for you to
	// report type errors. Note that these functions
	// also tell the typeAnalysis object that the
	// analysis has failed, meaning that main.cpp
	// will print "Type check failed" at the end
	ta->badAssignOpr(this->line(), this->col());

	//Note that reporting an error does not set the
	// type of the current node, so setting the node
	// type must be done
	ta->nodeType(this, ErrorType::produce());
}

void DeclNode::typeAnalysis(TypeAnalysis * ta){
	//TODO("Override me in the subclass");
}

void VarDeclNode::typeAnalysis(TypeAnalysis * ta){
	// VarDecls always pass type analysis, since they 
	// are never used in an expression. You may choose
	// to type them void (like this), as discussed in class
	ta->nodeType(this, BasicType::produce(VOID));
}

void IDNode::typeAnalysis(TypeAnalysis * ta){
	// IDs never fail type analysis and always
	// yield the type of their symbol (which
	// depends on their definition)
	ta->nodeType(this, this->getSymbol()->getDataType());
}

void IntLitNode::typeAnalysis(TypeAnalysis * ta){
	// IntLits never fail their type analysis and always
	// yield the type INT
	ta->nodeType(this, BasicType::produce(INT));
}

void CharLitNode::typeAnalysis(TypeAnalysis * ta){
	ta->nodeType(this, BasicType::produce(CHAR));
}

void FalseNode::typeAnalysis(TypeAnalysis * ta){
	ta->nodeType(this, BasicType::produce(BOOL));
}

void TrueNode::typeAnalysis(TypeAnalysis * ta){
	ta->nodeType(this, BasicType::produce(BOOL));
}

void StrLitNode::typeAnalysis(TypeAnalysis * ta){
    ta->nodeType(this, PtrType::produce(BasicType::produce(CHAR),1));
}

void DivideNode::typeAnalysis(TypeAnalysis * ta){
	//Do typeAnalysis on the subexpressions
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	// constant containing the type returned from type analysis on both expressions
	const DataType * exp1 = ta->nodeType(myExp1);
	const DataType * exp2 = ta->nodeType(myExp2);

	// base case is you dont throw an error if the types are compatible for division i.e. both are integers 
	// checks that both expression types are the same and both are int type
	if (exp1 == exp2 && exp1->isInt()){
		ta->nodeType(this, exp1);
	}
	// case where exp1 & exp2 are errors
	else if(exp1->asError()!=nullptr && exp2->asError()!=nullptr ){
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp1 is an error
	else if(exp1->asError()!=nullptr && exp2->asError()==nullptr ){
	    ta->badMathOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp2 is an error
	else if(exp1->asError()==nullptr && exp2->asError()!=nullptr ){
	    ta->badMathOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}

	// if both are not int throw Arithmetic operator applied to incompatible operands
	else if(!exp1->isInt() && !exp2->isInt()){
	    ta->badMathOpd(myExp1->line(), myExp1->col());
	    ta->badMathOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// first expression is not an int
	// throw Arithmetic operator applied to invalid operand
	else if(!exp1->isInt() && exp2->isInt() ){
	    ta->badMathOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// second expression is not an int
	// throw Arithmetic operator applied to invalid operand
	else if(exp1->isInt() && !exp2->isInt()){
	    ta->badMathOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
}

void TimesNode::typeAnalysis(TypeAnalysis * ta){
	//Do typeAnalysis on the subexpressions
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	// constant containing the type returned from type analysis on both expressions
	const DataType * exp1 = ta->nodeType(myExp1);
	const DataType * exp2 = ta->nodeType(myExp2);

	// base case is you dont throw an error if the types are compatible for division i.e. both are integers 
	// checks that both expression types are the same and both are int type
	if (exp1 == exp2 && exp1->isInt()){
		ta->nodeType(this, exp1);
	}
	// case where exp1 & exp2 are errors
	else if(exp1->asError()!=nullptr && exp2->asError()!=nullptr ){
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp1 is an error
	else if(exp1->asError()!=nullptr && exp2->asError()==nullptr ){
	    ta->badMathOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp2 is an error
	else if(exp1->asError()==nullptr && exp2->asError()!=nullptr ){
	    ta->badMathOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}

	// if both are not int throw Arithmetic operator applied to incompatible operands
	else if(!exp1->isInt() && !exp2->isInt()){
	    ta->badMathOpd(myExp1->line(), myExp1->col());
	    ta->badMathOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// first expression is not an int
	// throw Arithmetic operator applied to invalid operand
	else if(!exp1->isInt() && exp2->isInt() ){
	    ta->badMathOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// second expression is not an int
	// throw Arithmetic operator applied to invalid operand
	else if(exp1->isInt() && !exp2->isInt()){
	    ta->badMathOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
}

void MinusNode::typeAnalysis(TypeAnalysis * ta){
	//Do typeAnalysis on the subexpressions
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	// constant containing the type returned from type analysis on both expressions
	const DataType * exp1 = ta->nodeType(myExp1);
	const DataType * exp2 = ta->nodeType(myExp2);

	// base case is you dont throw an error if the types are compatible for division i.e. both are integers 
	// checks that both expression types are the same and both are int type
	if (exp1 == exp2 && exp1->isInt()){
		ta->nodeType(this, exp1);
	}
	// case where exp1 & exp2 are errors
	else if(exp1->asError()!=nullptr && exp2->asError()!=nullptr ){
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp1 is an error
	else if(exp1->asError()!=nullptr && exp2->asError()==nullptr ){
	    ta->badMathOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp2 is an error
	else if(exp1->asError()==nullptr && exp2->asError()!=nullptr ){
	    ta->badMathOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}

	// if both are not int throw Arithmetic operator applied to incompatible operands
	else if(!exp1->isInt() && !exp2->isInt()){
	    ta->badMathOpd(myExp1->line(), myExp1->col());
	    ta->badMathOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// first expression is not an int
	// throw Arithmetic operator applied to invalid operand
	else if(!exp1->isInt() && exp2->isInt() ){
	    ta->badMathOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// second expression is not an int
	// throw Arithmetic operator applied to invalid operand
	else if(exp1->isInt() && !exp2->isInt()){
	    ta->badMathOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
}

void PlusNode::typeAnalysis(TypeAnalysis * ta){
	//Do typeAnalysis on the subexpressions
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	// constant containing the type returned from type analysis on both expressions
	const DataType * exp1 = ta->nodeType(myExp1);
	const DataType * exp2 = ta->nodeType(myExp2);

	// base case is you dont throw an error if the types are compatible for division i.e. both are integers 
	// checks that both expression types are the same and both are int type
	if (exp1 == exp2 && exp1->isInt()){
		ta->nodeType(this, exp1);
	}
	// case where exp1 & exp2 are errors
	else if(exp1->asError()!=nullptr && exp2->asError()!=nullptr ){
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp1 is an error
	else if(exp1->asError()!=nullptr && exp2->asError()==nullptr ){
	    ta->badMathOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp2 is an error
	else if(exp1->asError()==nullptr && exp2->asError()!=nullptr ){
	    ta->badMathOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}

	// if both are not int throw Arithmetic operator applied to incompatible operands
	else if(!exp1->isInt() && !exp2->isInt()){
	    ta->badMathOpd(myExp1->line(), myExp1->col());
	    ta->badMathOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// first expression is not an int
	// throw Arithmetic operator applied to invalid operand
	else if(!exp1->isInt() && exp2->isInt() ){
	    ta->badMathOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// second expression is not an int
	// throw Arithmetic operator applied to invalid operand
	else if(exp1->isInt() && !exp2->isInt()){
	    ta->badMathOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
}

void NegNode::typeAnalysis(TypeAnalysis * ta){
}

void PostDecStmtNode::typeAnalysis(TypeAnalysis * ta){
}

void PostIncStmtNode::typeAnalysis(TypeAnalysis * ta){
}

}
